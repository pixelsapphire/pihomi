#include <string>

namespace phm {

    class serial_port {

        int fd;

    public:

        serial_port(const std::string& device, int baudrate);

        ~serial_port();

        [[nodiscard]] bool is_open() const;

        void close() const;

        void write(const std::string& data) const;

        [[nodiscard]] std::string read() const;
    };
}
