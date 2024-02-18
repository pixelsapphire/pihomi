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

    class output_console {

        std::string level_tag;
        bool enabled, error_color;

        [[nodiscard]] std::string timestamp() const;

    public:

        explicit output_console(const std::string& level, bool error = false);

        void println(const std::string& message = "") const;

        void flush() const;

        void operator()(int status, int errnum, const char* format, ...) const __attribute__((format(printf, 4, 5)));

        void set_enabled(bool enabled);
    };

    extern phm::input_console in;
    extern phm::output_console debug, info, warn, error;

    void register_command(const std::string& name, std::function<void(const std::vector<std::string>&)> callback);

    void execute_command(const std::string& cmd);

    [[nodiscard]] std::string to_string(size_t n, char** words);
}

#endif // PIHOMI_COMMANDS_HPP