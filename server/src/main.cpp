
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "CatFinder.h"
#include "LaserPointerFinder.h"
#include "ServoControl/EnumSerial.h"
#include "ServoControl/ServoControl.h"

#define USE_MJPEG_SERVER
#ifdef USE_MJPEG_SERVER
#include "MJPEGServer/MJPEGServer.h"
#define MJPEG_SERVER_PORT 8080
#endif

//#define NO_GUI

//#define USE_ESP32CAM
#ifdef USE_ESP32CAM
#define ESP32CAM_VIDEO_URL "http://10.42.0.69/stream.mjpeg"
#endif

using namespace cv;
using namespace dnn;

int main() {
    // needed to be able to check if data is available in cin
    // see https://stackoverflow.com/a/29262017
    std::cin.sync_with_stdio(false);

    CatFinder* catFinder = new CatFinder();

    if (!catFinder->initialised()) {
        delete catFinder;
        return -1;
    }

#ifndef NO_GUI
    // open a window
    static const std::string windowName = "window";
    namedWindow(windowName, WINDOW_NORMAL);
#endif

    // open a video device
    VideoCapture cap;
#ifdef USE_ESP32CAM
    if (!cap.open(ESP32CAM_VIDEO_URL, CAP_FFMPEG)) {
        std::cout << "failed to open camera stream" << std::endl;
        delete catFinder;
        return -1;
    }
#else
    if (!cap.open(0)) {
        std::cout << "failed to open camera" << std::endl;
        delete catFinder;
        return -1;
    }

    /*
    if (!cap.open("../../video_in.mp4")) {
        std::cout << "failed to open video" << std::endl;
        delete catFinder;
        return -1;
    }
    */
    /*
    Size inputSize = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
                          (int)cap.get(CAP_PROP_FRAME_HEIGHT));

    VideoWriter videoOut;
    if (!videoOut.open("../../output.mp4", CAP_FFMPEG,
                       VideoWriter::fourcc('F', 'M', 'P', '4'), 10.0, inputSize,
                       true)) {
    }
    */

#endif

#ifdef USE_MJPEG_SERVER
    MJPEGServer jpegServer(MJPEG_SERVER_PORT);
    jpegServer.start();
#endif

    LaserPointerFinder* laserPointerFinder = new LaserPointerFinder();

    // initialize servo control
    ServoControl* servoControl;
    {
        EnumSerial enumSerial;
        std::vector<std::string> connectedPorts = enumSerial.enumSerialPorts();
        if (connectedPorts.size() == 0) {
            std::cout << "failed to find connected serial port" << std::endl;
            servoControl = new ServoControl();
        } else {
            std::string serialPortName = connectedPorts.at(0);
            servoControl = new ServoControl(9600, serialPortName);
            std::cout << "using arduino on port " << serialPortName
                      << std::endl;
        }
    }

    servoControl->setPos(90, 90);

    // keep running until a key is pressed
    // call to waitKey() is needed for the window to show up
    // and change it's contents
    bool shouldQuit = false;
    std::cout << "press q and hit enter to quit" << std::endl;
    while (waitKey(1) < 0 && !shouldQuit) {
        // get a frame
        Mat frame;
        cap >> frame;

        if (!frame.empty()) {

            // laser pointer detection
            laserPointerFinder->processFrame(&frame);

            // object detection
            bool overlay = true;
            catFinder->processFrame(&frame, overlay);
#ifdef USE_MJPEG_SERVER
            jpegServer.serve(&frame);
#endif

            std::vector<CatBox>* cats = catFinder->getCats();
            for (size_t i = 0; i < cats->size(); i++) {
                CatBox cat = cats->at(i);
                /*
                printf("found: %.2f %i %i %i %i\n", cat.confidence, cat.x,
                cat.y, cat.width, cat.height);
                */
            }

            // replace with laser pointer target calculation stuff
            if (cats->size() > 0) {
                CatBox cat = cats->at(0);
                int servoA = (int)((float)(cat.x + (cat.width / 2)) /
                                   (float)frame.cols * 180.0);
                int servoB = (int)((float)(cat.y + (cat.height / 2)) /
                                   (float)frame.rows * 180.0);
                servoControl->setPos(servoA, servoB);
            }

            std::vector<LaserCircle>* lasers =
                laserPointerFinder->getLaserPointers();
            for (size_t i = 0; i < lasers->size(); i++) {
                LaserCircle laser = lasers->at(i);
                circle(frame, Point(laser.x, laser.y), laser.rad,
                       Scalar(0, 255, 0), 3, LINE_AA);
            }

#ifndef NO_GUI
            // show the frame
            imshow(windowName, frame);
#endif

            // Mat frameCopy = frame.clone();
            // write the frame to the video file
            // videoOut.write(frame);
        }

        // check if user pressed q to quit
        while(std::cin.rdbuf()->in_avail() > 0){
            char c;
            std::cin >> c;
            if(c == 'q'){
                shouldQuit = true;
                break;
            }
        }
    }

#ifdef USE_MJPEG_SERVER
    jpegServer.stop();
#endif

    delete catFinder;
    delete laserPointerFinder;
    delete servoControl;

    return 0;
}
