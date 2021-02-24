#include "ServoControl.h"

ServoControl::ServoControl(int baudRate, std::string port) {
    dummyMode = false;

    using namespace boost::system;
    using namespace boost::asio;

    ioService = new io_service();
    serialPort = new serial_port(*ioService);

    serialPort->open(port);

    serialPort->set_option(serial_port::baud_rate(baudRate));
    serialPort->set_option(serial_port::parity(serial_port::parity::none));
    serialPort->set_option(
        serial_port::character_size(serial_port::character_size(8)));
    serialPort->set_option(serial_port::stop_bits(serial_port::stop_bits::one));
    serialPort->set_option(
        serial_port::flow_control(serial_port::flow_control::none));
}

ServoControl::ServoControl() {
    dummyMode = true;
    std::cout << "servo control is running in dummy mode" << std::endl;
}

ServoControl::~ServoControl() {
    if (!dummyMode) {
        serialPort->close();
        delete serialPort;
        delete ioService;
    }
}

void ServoControl::setPos(int servoA, int servoB) {
    if (!dummyMode) {
        char msg[7];
        msg[0] = 's';
        msg[1] = 'e';
        msg[2] = (char)(servoA - 128);
        msg[3] = 'r';
        msg[4] = (char)(servoB - 128);
        msg[5] = 'v';
        msg[6] = 'o';

        serialPort->write_some(boost::asio::buffer(msg, 7));
    }
}
