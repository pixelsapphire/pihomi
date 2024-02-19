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

    class serial_port {

        int fd;
        bool active = false;
        const std::string device_path;

    public:

        serial_port(std::string device, int baudrate);

        ~serial_port();

        [[nodiscard]] const std::string& device() const noexcept;

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

        [[nodiscard]] std::string status_str() const noexcept;
    };

    class irrigation {

        static inline const std::vector<uint8_t> water_level_pins = {16, 20, 21};
        static inline const uint8_t water_pump_pin = 24;

        bool on;
        float delay;
        uint32_t volume;
        std::vector<phm::gpio::pin> water_level_sensor;
        phm::gpio::pin water_pump;

    public:

        irrigation();

        [[nodiscard]] bool get_state() const noexcept;

        void set_state(bool state) noexcept;

        [[nodiscard]] uint8_t get_water_level() const;

        [[nodiscard]] float get_watering_delay() const noexcept;

        void set_watering_delay(float watering_delay) noexcept;

        [[nodiscard]] uint32_t get_watering_volume() const noexcept;

        void set_watering_volume(uint32_t watering_volume) noexcept;

        void pour_water();

        [[nodiscard]] std::string status_str() const noexcept;
    };

    class outlet {

        bool on = false;
        phm::gpio::pin relay;

    public:

        explicit outlet(uint8_t relay_pin);

        [[nodiscard]] bool get_state() const noexcept;

        void set_state(bool state) noexcept;

        void toggle();
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
        bool outlets_enabled;
        phm::periodic_task clock_task, irrigation_task;

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

        [[nodiscard]] std::string outlets_status_str() const noexcept;
    };
}

#endif // PIHOMI_CONTROLLER_HPP
