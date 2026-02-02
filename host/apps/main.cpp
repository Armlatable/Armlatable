
/**
 * @file main.cpp
 * @brief CLI制御アプリケーション
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm/algorithm_teleop.hpp>
#include <serial/serial_port.hpp>
#include <protocol/protocol.h>

#include <termios.h>
#include <unistd.h>

// ... (includes)

using namespace locomo;

// Terminal setting helpers
struct termios orig_termios;
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(int argc, char* argv[]) {
    // ... (Serial setup)
    std::string device = "/dev/tty.usbmodem14201";
    if (argc > 1) {
        device = argv[1];
    }

    std::cout << "Locomo CLI Controller" << std::endl;
    std::cout << "Connecting to dongle at " << device << "..." << std::endl;

    host::SerialPort serial;
    if (!serial.openPort(device, 115200)) {
        std::cerr << "Failed to open serial port." << std::endl;
        return 1;
    }

    std::unique_ptr<algorithm::AlgorithmBase> algo = std::make_unique<algorithm::AlgorithmTeleop>(0.5f, 1.0f);
    char key;

    std::cout << "Controls: W/A/S/D to move, X to stop, Q to quit." << std::endl;

    // Enable raw mode
    enableRawMode();

    // Receiver thread
    std::thread receiver([&serial]() {
        uint8_t buffer[256];
        while (serial.isOpen()) {
            int len = serial.readData(buffer, sizeof(buffer));
            if (len > 0) {
                 // Move cursor to start of line, print, then restore prompt position (simplified)
                 // For now just print. In raw mode this might look messy but acceptable for debug.
                 printf("\r\nReceived %d bytes: ", len);
                 for(int i=0; i<len; i++) printf("%02X ", buffer[i]);
                 printf("\r\n");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    while (true) {
        // Read single char
        if (read(STDIN_FILENO, &key, 1) == 1) {
            if (key == 'q') break;

            // Use AlgorithmBase::update
            auto cmd = algo->update(key);

            uint8_t packet[64];
            uint8_t packet_len = protocol::Packetizer::createPacket(
                protocol::CmdID::SET_VELOCITY,
                &cmd,
                sizeof(cmd),
                packet,
                sizeof(packet)
            );

            if (packet_len > 0) {
                serial.writeData(packet, packet_len);
                // Carriage return needed in raw mode
                printf("\rSent: vx=%.2f, wz=%.2f\n", cmd.vx, cmd.wz);
            }
        }
    }

    disableRawMode(); // Restore settings
    serial.closePort();
    if (receiver.joinable()) receiver.join();

    return 0;
}
