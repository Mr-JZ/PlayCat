#ifndef CAT_FINDER_H
#define CAT_FINDER_H

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

#define CLASS_ID_HUMAN 1
#define CLASS_ID_CAT 17
#define CLASS_ID_DOG 18

struct CatBox {
    int x, y, width, height;
    float confidence;
};

class CatFinder {
  public:
    CatFinder();
    ~CatFinder();
    bool initialised();
    void processFrame(cv::Mat* frame, bool overlay);
    void processFrame(cv::Mat* frame);
    std::vector<CatBox>* getCats();

  private:
    bool initOk;
    cv::dnn::Net net;
    std::vector<std::string> outNames, classes;
    std::vector<CatBox> cats;
};

#endif // CAT_FINDER_H
