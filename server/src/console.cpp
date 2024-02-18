#include <chrono>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include "console.hpp"

std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands;

phm::input_console phm::in{"pihomi> "};
phm::output_console phm::debug{"DEBUG"}, phm::info{"INFO"}, phm::warn{"WARN", true}, phm::error{"ERROR", true};

std::vector<std::string> phm_split(const std::string& command) {
    std::vector<std::string> args;
    std::string arg;
    bool escaped = false, quoted = false;
    for (char c : command) {
        if (escaped) {
            arg += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            quoted = !quoted;
        } else if (c == ' ' && !quoted) {
            args.push_back(arg);
            arg.clear();
        } else {
            arg += c;
        }
    }
    if (!arg.empty()) args.push_back(arg);
    return args;
}

phm::input_console::input_console(const std::string& prompt) : prompt(prompt) {}

std::string phm::input_console::readln() const {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

phm::output_console::output_console(const std::string& level, bool error) : level_tag(level), enabled(true), error_color(error) {}

std::string phm::output_console::timestamp() const {
    const time_t time = std::time(nullptr);
    char time_str[32]{0};
    std::strftime(time_str, sizeof(time_str), "%a %b %d %Y %H:%M:%S", std::localtime(&time));
    return time_str;
}

void phm::output_console::println(const std::string& message) const {
    if (enabled) (error_color ? std::cerr : std::cout) << '[' << timestamp() << "] [" << level_tag << "] " << message << std::endl;
}

void phm::output_console::flush() const { (error_color ? std::cerr : std::cout).flush(); }

void phm::output_console::operator()(int status, int errnum, const char *format, ...) const {
    va_list args;
    va_start(args, format);
    char buffer[1024]{0};
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    phm::info.flush();
    std::string output = buffer;
    if (errnum != 0) output += ": " + std::string(strerror(errnum));
    println(output);
    flush();
    if (status != 0) std::exit(status);
}

void phm::output_console::set_enabled(bool enabled) { this->enabled = enabled; }

void phm::register_command(const std::string& name, std::function<void(const std::vector<std::string>&)> callback) {
    commands[name] = callback;
}

void phm::execute_command(const std::string& cmd) {
    auto args = phm_split(cmd);
    if (args.empty()) return;
    const std::string command = args[0];
    args.erase(args.begin());
    if (commands.find(command) != commands.end()) commands[command](args);
    else phm::error.println("Unknown command: " + command);
}

std::string phm::to_string(size_t n, char** words) {
    std::string str;
    for (size_t i = 0; i < n; ++i) {
        str += words[i];
        if (i < n - 1) str += ' ';
    }
    return str;
}
