#include <stdexcept>
#include <string>
#include "gpio.hpp"

namespace phm::gpio {
    int descriptor = -1;
}

phm::gpio::pin::pin(uint8_t number, phm::pin_mode mode) : number(number), mode(mode) {}

#ifdef RASPBERRY

#include <pigpiod_if2.h>

void phm::gpio::begin() {
    phm::gpio::descriptor = pigpio_start(nullptr, nullptr);
    if (phm::gpio::descriptor < 0) std::system("sudo pigpiod; sleep 1;");
    phm::gpio::descriptor = pigpio_start(nullptr, nullptr);
    if (phm::gpio::descriptor < 0)
        throw std::runtime_error("Failed to connect to PiGPIO daemon: " + std::string(pigpio_error(phm::gpio::descriptor)));
}

void phm::gpio::pin::set(phm::logic_state state) {
    if (mode == phm::pin_mode::input) throw std::runtime_error("Cannot set state of input pin");
    if (gpio_write(phm::gpio::descriptor, number, state) < 0)
        throw std::runtime_error("Failed to change pin state: " + std::string(pigpio_error(phm::gpio::descriptor)));
}

void phm::gpio::pin::toggle() {
    if (mode == phm::pin_mode::input) throw std::runtime_error("Cannot toggle state of input pin");
    if (gpio_write(phm::gpio::descriptor, number, !gpio_read(phm::gpio::descriptor, number)) < 0)
        throw std::runtime_error("Failed to toggle pin state: " + std::string(pigpio_error(phm::gpio::descriptor)));
}

phm::logic_state phm::gpio::pin::get() const {
    if (mode == phm::pin_mode::output) throw std::runtime_error("Cannot get state of output pin");
    const int state = gpio_read(phm::gpio::descriptor, number);
    if (state < 0) throw std::runtime_error("Failed to read pin state: " + std::string(pigpio_error(phm::gpio::descriptor)));
    return phm::logic_state(state);
}

#else

void phm::gpio::begin() { phm::gpio::descriptor = 0; }

void phm::gpio::pin::set(phm::logic_state state) {}

phm::logic_state phm::gpio::pin::get() const {}

#endif
