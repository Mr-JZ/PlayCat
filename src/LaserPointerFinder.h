#ifndef LASER_POINTER_H
#define LASER_POINTER_H

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

struct LaserCircle{
    float x, y, rad;
};

class LaserPointerFinder {
  public:
    LaserPointerFinder();
    ~LaserPointerFinder();
    void processFrame(cv::Mat* frame);
    std::vector<LaserCircle>* getLaserPointers();
  private:
    std::vector<LaserCircle> laserPointers;
};

#endif // LASER_POINTER_H