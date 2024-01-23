#include <stdexcept>
#include <string>
#include "gpio.hpp"

namespace phm::gpio {
    int descriptor = -1;
}

phm::gpio::pin::pin(uint8_t number) : number(number) {}

#ifdef RASPBERRY

#include <pigpiod_if2.h>

void phm::gpio::begin() {
    std::system("sudo pigpiod; sleep 1;");
    if ((phm::gpio::descriptor = pigpio_start(nullptr, nullptr)) < 0)
        throw std::runtime_error("Failed to connect to PiGPIO daemon: " +
                                  std::string(pigpio_error(phm::gpio::descriptor)));
}

void phm::gpio::pin::set(phm::logic_state state) {
    if (gpio_write(phm::gpio::descriptor, number, state) < 0)
        throw std::runtime_error("Failed to change pin state: " +
                                 std::string(pigpio_error(phm::gpio::descriptor)));
}

#else

void phm::gpio::begin() { phm::gpio::descriptor = 0; }

void phm::gpio::pin::set(phm::logic_state state) {}

#endif
