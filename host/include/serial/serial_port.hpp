
/**
 * @file serial_port.hpp
 * @brief シリアルポート通信クラス (POSIX/Termios)
 */

#ifndef SERIAL_PORT_HPP
#define SERIAL_PORT_HPP

#include <string>
#include <vector>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <system_error>
#include <cstring>
#include <sys/ioctl.h> // for ioctl, TIOCM*

namespace locomo {
namespace host {

class SerialPort {
public:
    SerialPort() : fd_(-1) {}
    ~SerialPort() { closePort(); }

    bool openPort(const std::string& device, int baud_rate = 115200) {
        // O_NDELAY to avoid waiting for DCD
        fd_ = open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);
        if (fd_ < 0) {
            std::cerr << "Error opening " << device << ": " << strerror(errno) << std::endl;
            return false;
        }

        // Clear DTR and RTS to avoid resetting ESP32
        int status;
        ioctl(fd_, TIOCMGET, &status);
        status &= ~(TIOCM_DTR | TIOCM_RTS);
        ioctl(fd_, TIOCMSET, &status);

        // Remove O_NDELAY now
        fcntl(fd_, F_SETFL, 0);

        struct termios tty;
        if (tcgetattr(fd_, &tty) != 0) {
            std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
            return false;
        }

        speed_t speed;
        if (baud_rate == 115200) speed = B115200;
        else if (baud_rate == 9600) speed = B9600;
        else speed = B115200; // Default

        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
        tty.c_iflag &= ~IGNBRK;                     // disable break processing
        tty.c_lflag = 0;                            // no signaling chars, no echo,
                                                    // no canonical processing
        tty.c_oflag = 0;                            // no remapping, no delays
        tty.c_cc[VMIN]  = 0;                        // read doesn't block
        tty.c_cc[VTIME] = 5;                        // 0.5 seconds read timeout

        tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
                                         // enable reading
        tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
        tty.c_cflag |= 0;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
            std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
            return false;
        }

        // Ensure DTR/RTS are cleared again after tcsetattr potentially reset them
        ioctl(fd_, TIOCMGET, &status);
        status &= ~(TIOCM_DTR | TIOCM_RTS);
        ioctl(fd_, TIOCMSET, &status);

        tcflush(fd_, TCIOFLUSH);

        // Wait a bit for ESP32 to stabilize if it was reset
        usleep(100000);

        return true;
    }

    void closePort() {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }

    int writeData(const uint8_t* data, size_t size) {
        if (fd_ < 0) return -1;
        return write(fd_, data, size);
    }

    int readData(uint8_t* buffer, size_t buffer_size) {
        if (fd_ < 0) return -1;
        return read(fd_, buffer, buffer_size);
    }

    bool isOpen() const { return fd_ >= 0; }

private:
    int fd_;
};

} // namespace host
} // namespace locomo

#endif // SERIAL_PORT_HPP
