#ifndef PIHOMI_GPIO_HPP
#define PIHOMI_GPIO_HPP

#include <cstdlib>
#include <cstdint>

namespace phm {

    enum logic_state {
        low = 0, high = 1
    };

    namespace gpio {

        void begin();

        class pin {

            uint8_t number;

        public:

            explicit pin(uint8_t number);

            pin(const pin&) = default;

            pin(pin&&) noexcept = default;

            pin& operator=(const pin&) = default;

            pin& operator=(pin&&) noexcept = default;

            void set(phm::logic_state state);
        };
    }
}

#endif // PIHOMI_GPIO_HPP