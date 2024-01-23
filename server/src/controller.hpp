#ifndef PIHOMI_CONTROLLER_HPP
#define PIHOMI_CONTROLLER_HPP

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include "gpio.hpp"

namespace phm {

    enum device {
        on = true, off = false
    };

    template<typename ...Args>
    void uwu(Args&& ...) {}

    class serial_port {

        int fd;
        bool active = false;

    public:

        serial_port(const std::string& device, int baudrate);

        ~serial_port();

        [[nodiscard]] bool is_open() const noexcept;

        void write(const std::string& data) const;

        [[nodiscard]] std::string read() const;
    };

    class clock {

        serial_port arduino;
        bool error = false, on = true;

        void process_response();

    public:

        clock(const std::string& device);

        [[nodiscard]] bool is_good() const noexcept;

        [[nodiscard]] bool get_state() const noexcept;

        void set_state(bool state);

        void update_time();
    };

    class irrigation {

        bool on = true;
        uint8_t level = 0;
        float delay = 0;
        uint32_t volume = 0;

    public:

        [[nodiscard]] bool get_state() const noexcept;

        void set_state(bool state) noexcept;

        [[nodiscard]] uint8_t get_water_level() const noexcept;

        void set_water_level(uint8_t water_level) noexcept;

        [[nodiscard]] float get_watering_delay() const noexcept;

        void set_watering_delay(float watering_delay) noexcept;

        [[nodiscard]] uint32_t get_watering_volume() const noexcept;

        void set_watering_volume(uint32_t watering_volume) noexcept;

        void pour_water();
    };

    class outlet {

        bool on = true;
        phm::gpio::pin relay;

    public:

        explicit outlet(uint8_t relay_pin);

        [[nodiscard]] bool get_state() const noexcept;

        void set_state(bool state) noexcept;
    };

    class periodic_task {

        std::thread thread;
        bool running = false;
        std::function<std::chrono::seconds()> interval_supplier;
        std::function<void()> task;

    public:

        periodic_task(std::chrono::seconds interval, std::function<void()> task);

        periodic_task(std::function<std::chrono::seconds()> interval_supplier, std::function<void()> task);

        ~periodic_task();

        void start();

        void stop();
    };

    class controller {

        static inline const std::vector<uint8_t> outlet_pins = {17, 27, 22, 23};

        phm::clock clock;
        phm::irrigation irrigation;
        std::vector<phm::outlet> outlets;
        bool outlets_enabled = true;
        phm::periodic_task clock_task{std::chrono::seconds(1), [&] { clock.update_time(); }};
        phm::periodic_task irrigation_task{
                [&] { return std::chrono::seconds(uint64_t(irrigation.get_watering_delay() * 4)); },
                [&] { if (irrigation.get_state() == phm::device::on) irrigation.pour_water(); }
        };

    public:

        controller(const std::string& clock_device);

        ~controller();

        void set_outlets_state(bool enable) noexcept;

        [[nodiscard]] bool outlets_state() const noexcept;

        [[nodiscard]] phm::clock& get_clock() noexcept;

        [[nodiscard]] const phm::clock& get_clock() const noexcept;

        [[nodiscard]] phm::irrigation& get_irrigation() noexcept;

        [[nodiscard]] const phm::irrigation& get_irrigation() const noexcept;

        [[nodiscard]] phm::outlet& get_outlet(uint8_t id);

        [[nodiscard]] const phm::outlet& get_outlet(uint8_t id) const;
    };
}

#endif // PIHOMI_CONTROLLER_HPP
