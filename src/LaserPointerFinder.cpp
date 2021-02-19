#include "LaserPointerFinder.h"

LaserPointerFinder::LaserPointerFinder() {
}

LaserPointerFinder::~LaserPointerFinder() {
}

void LaserPointerFinder::processFrame(cv::Mat* frame) {
    laserPointers.clear();
    using namespace cv;

    Mat frameHSV;
    cvtColor(*frame, frameHSV, COLOR_BGR2HSV);

    Mat1b maskA, maskB;
    inRange(frameHSV, Scalar(0, 120, 70), Scalar(10, 255, 255), maskA);
    inRange(frameHSV, Scalar(170, 120, 70), Scalar(180, 255, 255), maskB);

    Mat1b mask = maskA | maskB;

    //imshow("masked", mask);

    std::vector<Vec3f> circles;
    HoughCircles(mask, circles, HOUGH_GRADIENT, 1, 20, 100, 30, 1, 50);
    for (size_t i = 0; i < circles.size(); i++) {
        Vec3f c = circles.at(i);
        Point center = Point(c[0], c[1]);
        float radius = c[2];
        // circle(*frame, center, radius, Scalar(0, 255, 0), 3, LINE_AA);
        LaserCircle laserCircle;
        laserCircle.x = c[0];
        laserCircle.y = c[1];
        laserCircle.rad = c[2];
        laserPointers.push_back(laserCircle);
    }
}

std::vector<LaserCircle>* LaserPointerFinder::getLaserPointers() {
    return &laserPointers;
}
