#include <wiringSerial.h>
#include "serial.hpp"

phm::serial_port::serial_port(const std::string& device, int baudrate) {
    fd = serialOpen(device.c_str(), baudrate);
}

phm::serial_port::~serial_port() {
    serialClose(fd);
}

bool phm::serial_port::is_open() const {
    return fd >= 0;
}

void phm::serial_port::close() const {
    serialClose(fd);
}

void phm::serial_port::write(const std::string& data) const {
    serialPuts(fd, data.c_str());
    serialFlush(fd);
}

std::string phm::serial_port::read() const {
    std::string buffer;
    while (buffer.size() == 0 or buffer[buffer.size() - 1] != '\n')
        if (serialDataAvail(fd)) buffer += char(serialGetchar(fd));
    return buffer;
}
