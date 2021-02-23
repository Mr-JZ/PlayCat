#!/bin/bash
# fetches and extracts model files for ssd_mobilenet_v2

mkdir -p ssd_mobilenet
cd ssd_mobilenet
wget -c "http://download.tensorflow.org/models/object_detection/ssd_mobilenet_v2_coco_2018_03_29.tar.gz"
wget "https://raw.githubusercontent.com/opencv/opencv_extra/master/testdata/dnn/ssd_mobilenet_v2_coco_2018_03_29.pbtxt" -O "ssd_mobilenet_v2_coco_2018_03_29.pbtxt"
tar -xzf ssd_mobilenet_v2_coco_2018_03_29.tar.gz
rm ssd_mobilenet_v2_coco_2018_03_29.tar.gz
mv ssd_mobilenet_v2_coco_2018_03_29/frozen_inference_graph.pb ./
rm -rf ssd_mobilenet_v2_coco_2018_03_29
