#ifndef PIHOMI_GPIO_HPP
#define PIHOMI_GPIO_HPP

#include <cstdlib>
#include <cstdint>

namespace phm {

    enum logic_state {
        low = 0, high = 1
    };

    enum pin_mode {
        input, output
    };

    namespace gpio {

        void begin();

        class pin {

        private:

            uint8_t number;
            phm::pin_mode mode;

        public:

            pin(uint8_t number, phm::pin_mode mode);

            pin(const pin&) = default;

            pin(pin&&) noexcept = default;

            pin& operator=(const pin&) = default;

            pin& operator=(pin&&) noexcept = default;

            void set(phm::logic_state state);

            void toggle();

            [[nodiscard]] phm::logic_state get() const;
        };
    }
}

#endif // PIHOMI_GPIO_HPP