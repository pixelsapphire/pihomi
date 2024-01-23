#include "controller.hpp"

bool phm::serial_port::is_open() const noexcept { return active; }

#ifdef RASPBERRY

#include <wiringSerial.h>

phm::serial_port::~serial_port() { if (active) serialClose(fd); }

phm::serial_port::serial_port(const std::string& device, int baudrate) {
    fd = serialOpen(device.c_str(), baudrate);
    if (fd >= 0) active = true;
}

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

bool phm::irrigation::get_state() const noexcept { return on; }

void phm::irrigation::set_state(bool state) noexcept { this->on = state; }

uint8_t phm::irrigation::get_water_level() const noexcept { return level; }

void phm::irrigation::set_water_level(uint8_t water_level) noexcept { this->level = water_level; }

float phm::irrigation::get_watering_delay() const noexcept { return delay; }

void phm::irrigation::set_watering_delay(float watering_delay) noexcept { this->delay = watering_delay; }

uint32_t phm::irrigation::get_watering_volume() const noexcept { return volume; }

void phm::irrigation::set_watering_volume(uint32_t watering_volume) noexcept { this->volume = watering_volume; }

void phm::irrigation::pour_water() { phm::uwu(volume); }

bool phm::outlet::get_state() const noexcept { return on; }

void phm::outlet::set_state(bool state) noexcept { this->on = state; }

phm::periodic_task::periodic_task(std::chrono::seconds interval, std::function<void()> task)
    : interval_supplier([=] { return interval; }), task(std::move(task)) {}

phm::periodic_task::periodic_task(std::function<std::chrono::seconds()> interval_supplier, std::function<void()> task)
        : interval_supplier(interval_supplier), task(std::move(task)) {}

phm::periodic_task::~periodic_task() { stop(); }

void phm::periodic_task::start() {
    if (running) return;
    running = true;
    thread = std::thread([&] {
        while (running and interval_supplier().count() > 0) {
            task();
            std::this_thread::sleep_for(interval_supplier());
        }
    });
}

void phm::periodic_task::stop() {
    if (not running) return;
    running = false;
    thread.join();
}

phm::controller::controller(const std::string& clock_device) : clock(clock_device) {
    for (uint8_t i = 0; i < 4; i++) outlets.emplace_back();
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