#ifndef PIHOMI_COMMANDS_HPP
#define PIHOMI_COMMANDS_HPP

#include <functional>
#include <string>

namespace phm {

    class input_console {

        std::string prompt;

    public:

        explicit input_console(const std::string& prompt);

        [[nodiscard]] std::string readln() const;
    };

    class console_output {

        std::string level_tag;
        bool error_color;

        [[nodiscard]] std::string timestamp() const;

    public:

        explicit console_output(const std::string& level, bool error = false);

        void println(const std::string& message = "") const;

        void flush() const;

        void operator()(int status, int errnum, const char* format, ...) const __attribute__((format(printf, 4, 5)));
    };

    extern phm::input_console in;
    extern phm::console_output debug, info, warn, error;

    void register_command(const std::string& name, std::function<void(const std::vector<std::string>&)> callback);

    void execute_command(const std::string& cmd);

    [[nodiscard]] std::string to_string(size_t n, char** words);
}

#endif // PIHOMI_COMMANDS_HPP