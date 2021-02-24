#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <string>
#include <iostream>

#include <boost/asio.hpp>

class ServoControl {
  public:
    ServoControl(int baudRate, std::string port);
    // dummy mode
    ServoControl();
    ~ServoControl();

    void setPos(int servoA, int servoB);

  private:
    boost::asio::io_service* ioService;
    boost::asio::serial_port* serialPort;

    bool dummyMode;
};

#endif // SERVO_CONTROLLER_H