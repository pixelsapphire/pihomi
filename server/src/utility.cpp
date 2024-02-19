#include <cstdio>
#include <stdexcept>
#include <sstream>
#include "utility.hpp"

static std::string execute_command(const std::string& command) {

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == 0) throw std::runtime_error("Could not execute command: " + command);

    char* buffer = nullptr;
    size_t buffer_size = 0;
    ssize_t line_length;
    std::ostringstream output;

    do {
        line_length = getline(&buffer, &buffer_size, pipe);
        if (line_length > 0) output << buffer;
    } while (line_length > 0);

    pclose(pipe);
    return output.str();
}

std::string phm::first_acm_device() {
    const std::string devices = execute_command("ls /dev");
    const size_t pos = devices.find("ttyACM");
    if (pos == std::string::npos) throw std::runtime_error("No ACM device found");
    return "/dev/" + devices.substr(pos, devices.find('\n', pos) - pos);
}

std::string phm::ip_address() {
    const std::string ip = execute_command("hostname -I");
    const size_t pos = ip.find(' ');
    if (pos == std::string::npos) throw std::runtime_error("No IP address found");
    return ip.substr(0, pos);
}
