#include "CatFinder.h"

CatFinder::CatFinder() {
    initOk = false;

    // load a model
    net = cv::dnn::readNetFromTensorflow(
        "../models/ssd_mobilenet/frozen_inference_graph.pb",
        "../models/ssd_mobilenet/ssd_mobilenet_v2_coco_2018_03_29.pbtxt");
    if (net.empty()) {
        std::cout << "failed to load model" << std::endl;
        return;
    }
    outNames = net.getUnconnectedOutLayersNames();

    // load class names
    {
        std::string file = "../models/classes.txt";
        std::ifstream ifs(file.c_str());
        std::string line;
        while (std::getline(ifs, line)) {
            classes.push_back(line);
        }
        ifs.close();
    }

    initOk = true;
}

CatFinder::~CatFinder() {
}

bool CatFinder::initialised() {
    return initOk;
}

void CatFinder::processFrame(cv::Mat* frame) {
    processFrame(frame, false);
}

void CatFinder::processFrame(cv::Mat* frame, bool overlay) {
    if (!initOk) {
        return;
    }

    using namespace cv;

    if (!frame->empty()) {
        cats.clear();

        // rotate the frame
        // rotate(frame, frame, ROTATE_90_COUNTERCLOCKWISE);

        // preprocess the frame
        static Mat blob;
        {
            bool swapRB = true;
            // preserve aspect ratio
            int targetX = 300;
            int targetY =
                (int)((float)targetX / (float)frame->cols * (float)frame->rows);
            cv::dnn::blobFromImage(*frame, blob, 1.0, Size(targetX, targetY),
                                   Scalar(), swapRB, false, CV_8U);
        }

        // run the model
        Scalar mean = Scalar();
        net.setInput(blob, "", 1.0, mean);

        std::vector<Mat> outs;
        net.forward(outs, outNames);

        static std::vector<int> outLayers = net.getUnconnectedOutLayers();
        static std::string outLayerType = net.getLayer(outLayers[0])->type;

        for (size_t i = 0; i < outs.size(); i++) {
            float* data = (float*)outs[i].data;
            for (size_t k = 0; k < outs[i].total(); k += 7) {
                float confidence = data[k + 2];
                // check if we are confident enough
                if (confidence >= 0.5) {
                    int classId = (int)data[k + 1];
                    // ignore background class id
                    if (classId != 0) {
                        int left = (int)(data[k + 3] * frame->cols);
                        int top = (int)(data[k + 4] * frame->rows);
                        int right = (int)(data[k + 5] * frame->cols);
                        int bottom = (int)(data[k + 6] * frame->rows);

                        if (overlay) {
                            // draw a box around it
                            rectangle(*frame, Point(left, top),
                                      Point(right, bottom), Scalar(0, 255, 0));
                        }

                        // check if we found a cat (or a dog or human for now)
                        if (classId == CLASS_ID_CAT ||
                            classId == CLASS_ID_DOG ||
                            classId == CLASS_ID_HUMAN) {
                            CatBox catBox;
                            catBox.confidence = confidence;
                            catBox.x = left;
                            catBox.y = top;
                            catBox.width = right - left;
                            catBox.height = bottom - top;
                            cats.push_back(catBox);
                        }

                        if (overlay) {
                            // find out class name and print it above the box
                            std::string className;
                            if (classId < classes.size()) {
                                className = classes[classId];
                            } else {
                                className =
                                    "unknown: " + std::to_string(classId);
                            }
                            if (className == "") {
                                className =
                                    "unknown: " + std::to_string(classId);
                            }
                            std::string label =
                                className + format(" %.2f", confidence);
                            int labelTop = top - 5;
                            if (labelTop < 12) {
                                labelTop = 12;
                            }
                            putText(*frame, label, Point(left, labelTop),
                                    FONT_HERSHEY_SIMPLEX, 0.5,
                                    Scalar(0, 255, 0));
                        }
                    }
                }
            }
        }

        if (overlay) {
            // show performance info
            std::vector<double> layerTimes;
            double freq = cv::getTickFrequency() / 1000.0;
            double time = net.getPerfProfile(layerTimes) / freq;
            std::string perfLabel = format("inference time: %.2f ms", time);
            putText(*frame, perfLabel, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5,
                    Scalar(0, 0, 0));
        }
    }
}

std::vector<CatBox>* CatFinder::getCats() {
    return &cats;
}
