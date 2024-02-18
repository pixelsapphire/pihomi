#include "console.hpp"
#include "controller.hpp"

bool phm::serial_port::is_open() const noexcept { return active; }

#ifdef RASPBERRY

#include <wiringSerial.h>

phm::serial_port::~serial_port() { if (active) serialClose(fd); }

phm::serial_port::serial_port(std::string device, int baudrate) : device_path(std::move(device)) {
    fd = serialOpen(device_path.c_str(), baudrate);
    if (fd >= 0) active = true;
}

const std::string& phm::serial_port::device() const noexcept { return device_path; }

void phm::serial_port::write(const std::string& data) const {
    serialPuts(fd, data.c_str());
    serialFlush(fd);
}

[[nodiscard]] std::string phm::serial_port::read() const {
    std::string response;
    while (true)
        if (serialDataAvail(fd)) {
            response += char(serialGetchar(fd));
            if (response.back() == '\n') break;
        } else std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return response;
};

#else

phm::serial_port::~serial_port() {}

phm::serial_port::serial_port(const std::string& device, int baudrate) {
    phm::uwu(device, baudrate);
    active = true;
}

void phm::serial_port::write(const std::string& data) const { phm::uwu(data); }

[[nodiscard]] std::string phm::serial_port::read() const { return "OK1\n"; };

#endif

phm::clock::clock(const std::string& device) : arduino(device, 9600) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void phm::clock::process_response() {
    const auto response = arduino.read();
    if (response.starts_with("OK")) {
        error = false;
        on = response.ends_with("1\n");
    } else {
        error = true;
        on = false;
    }
}

bool phm::clock::is_good() const noexcept { return arduino.is_open() and not error; }

bool phm::clock::get_state() const noexcept { return on; }

void phm::clock::set_state(bool state) {
    arduino.write('s' + std::to_string(state) + '\n');
    process_response();
}

void phm::clock::update_time() {
    const time_t time = std::time(nullptr);
    char time_str[10]{0};
    std::strftime(time_str, sizeof(time_str), "t%I:%M%p\n", std::localtime(&time));
    arduino.write(time_str);
    process_response();
}

std::string phm::clock::status_str() const noexcept {
    return std::string(get_state() ? "enabled" : "disabled") + " (" + arduino.device() + ", " + (is_good() ? "ok" : "error") + ")";
}

phm::irrigation::irrigation() : on(true), delay(0), volume(0), water_pump(water_pump_pin, phm::pin_mode::output) {
    for (uint8_t i = 0; i < 3; i++) water_level_sensor.emplace_back(phm::irrigation::water_level_pins[i], phm::pin_mode::input);
}

bool phm::irrigation::get_state() const noexcept { return on; }

void phm::irrigation::set_state(bool state) noexcept { this->on = state; }

uint8_t phm::irrigation::get_water_level() const {
    const phm::logic_state b4 = water_level_sensor[0].get(), b2 = water_level_sensor[1].get(), b1 = water_level_sensor[2].get();
    return b4 << 2 | b2 << 1 | b1;
}

float phm::irrigation::get_watering_delay() const noexcept { return delay; }

void phm::irrigation::set_watering_delay(float watering_delay) noexcept { this->delay = watering_delay; }

uint32_t phm::irrigation::get_watering_volume() const noexcept { return volume; }

void phm::irrigation::set_watering_volume(uint32_t watering_volume) noexcept { this->volume = watering_volume; }

void phm::irrigation::pour_water() {
    phm::info.println("Pouring " + std::to_string(volume) + "ml of water");
    water_pump.set(phm::logic_state::high);
    std::this_thread::sleep_for(std::chrono::milliseconds(volume * 4));
    water_pump.set(phm::logic_state::low);
}

std::string phm::irrigation::status_str() const noexcept {
    const uint8_t level = get_water_level();
    std::string level_percent = std::to_string(level / 7.0 * 100);
    level_percent = level_percent.substr(0, level_percent.find('.') + 2);
    return std::string(get_state() ? "enabled" : "disabled") + " (water level: " + level_percent +
           "% [" + std::to_string(level) + "/7], watering delay: " + std::to_string(get_watering_delay()) +
           " days, watering volume: " + std::to_string(get_watering_volume()) + "ml)";
}

phm::outlet::outlet(uint8_t relay_pin) : relay(relay_pin, phm::pin_mode::output) { relay.set(phm::logic_state::high); }

bool phm::outlet::get_state() const noexcept { return on; }

void phm::outlet::set_state(bool state) noexcept {
    on = state;
    relay.set(state ? phm::logic_state::low : phm::logic_state::high);
}

void phm::outlet::toggle() { on = not on; relay.toggle(); }

phm::periodic_task::periodic_task(std::chrono::seconds interval, std::function<void()> task)
        : interval_supplier([=] { return interval; }), task(std::move(task)) {}

phm::periodic_task::periodic_task(std::function<std::chrono::seconds()> interval_supplier, std::function<void()> task)
        : interval_supplier(interval_supplier), task(std::move(task)) {}

phm::periodic_task::~periodic_task() { stop(); }

void phm::periodic_task::start() {
    if (running) return;
    running = true;
    thread = std::thread([&] {
        while (running) {
            if (interval_supplier().count() >= 0) {
                const auto start = std::chrono::steady_clock::now();
                task();
                const auto elapsed = std::chrono::steady_clock::now() - start;
                const auto target_delay = interval_supplier();
                if (elapsed < target_delay) std::this_thread::sleep_for(target_delay - elapsed);
            }
        }
    });
}

void phm::periodic_task::stop() {
    if (not running) return;
    running = false;
    thread.join();
}

phm::controller::controller(const std::string& clock_device) 
        : clock(clock_device), outlets_enabled(true), clock_task{std::chrono::seconds(1), [&] { clock.update_time(); }},
          irrigation_task{
              [&] { return std::chrono::seconds(int64_t(irrigation.get_watering_delay() > 0 ? irrigation.get_watering_delay() * 4 : -1)); },
              [&] { if (irrigation.get_state() == phm::device::on) irrigation.pour_water(); }
          } {
    for (uint8_t i = 0; i < 4; i++) outlets.emplace_back(phm::controller::outlet_pins[i]);
    clock_task.start();
    irrigation_task.start();
}

phm::controller::~controller() {
    clock_task.stop();
    irrigation_task.stop();
}

void phm::controller::set_outlets_state(bool enable) noexcept { this->outlets_enabled = enable; }

bool phm::controller::outlets_state() const noexcept { return outlets_enabled; }

phm::clock& phm::controller::get_clock() noexcept { return clock; }

const phm::clock& phm::controller::get_clock() const noexcept { return clock; }

phm::irrigation& phm::controller::get_irrigation() noexcept { return irrigation; }

const phm::irrigation& phm::controller::get_irrigation() const noexcept { return irrigation; }

phm::outlet& phm::controller::get_outlet(uint8_t id) { return outlets[id]; }

const phm::outlet& phm::controller::get_outlet(uint8_t id) const { return outlets[id]; }

std::string phm::controller::outlets_status_str() const noexcept {
    std::string outlets_status;
    for (uint8_t i = 0; i < 4; i++) outlets_status += std::to_string(i + 1) + ":" + (outlets[i].get_state() ? "on" : "off") + ", ";
    return std::string(outlets_enabled ? "enabled" : "disabled") + " (" + outlets_status.substr(0, outlets_status.size() - 2) + ")";
}