#!/bin/bash
# configer the raspberry pi to install opencv in an optimal way

mkdir -p opencv && cd opencv

# clean the raspberry pi
sudo apt purge -y wolfram-engine
sudo apt purge -y libreoffice*
sudo apt clean
sudo apt autoremove

# install all significant packages
sudo apt update && sudo apt-get upgrade
sudo apt install -y build-essential cmake pkg-config
sudo apt install -y libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install -y libxvidcore-dev libx264-dev
sudo apt install -y libgtk2.0-dev libgtk-3-dev
sudo apt install -y libcanberra-gtk*
sudo apt install -y libatlas-base-dev gfortran
sudo apt install -y python2.7-dev python3-dev
pip3 install regex

# Download and unpack sources
wget -O opencv.zip https://github.com/opencv/opencv/archive/master.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/master.zip
unzip opencv.zip
unzip opencv_contrib.zip

# delete minor files
rm opencv.zip
rm opencv_contrib.zip

# Create build directory and switch into it
mkdir -p build && cd build
if [ uname -m -eq "armv71"]
then
    # Configure - generate build scripts for arm
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-master/modules \
        -D ENABLE_NEON=ON \
        -D ENABLE_VFPV3=ON \
        -D BUILD_TESTS=OFF \
        -D INSTALL_PYTHON_EXAMPLES=OFF \
        -D OPENCV_ENABLE_NONFREE=ON \
        -D CMAKE_SHARED_LINKER_FLAGS='-latomic' \
        -D BUILD_EXAMPLES=OFF ../opencv-master
else
    # Configure - generate build scripts for everything else
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-master/modules \
        -D ENABLE_NEON=OFF \
        -D ENABLE_VFPV3=OFF \
        -D BUILD_TESTS=OFF \
        -D INSTALL_PYTHON_EXAMPLES=OFF \
        -D OPENCV_ENABLE_NONFREE=ON \
        -D CMAKE_SHARED_LINKER_FLAGS='-latomic' \
        -D BUILD_EXAMPLES=OFF ../opencv-master
fi


# check the memorie
phymem=$(free|awk '/^Mem:/{print $2}')

if [ 1000000 -gt phymem ]
then
    # change the swap to be able to process the compiler
    sudo sed -i -r 's/^CONF_SWAPSIZE=[0-9]+/CONF_SWAPSIZE=1024/' /etc/dphys-swapfile
    sudo /etc/init.d/dphys-swapfile stop
    sudo /etc/init.d/dphys-swapfile start
fi

# make a executable
make -j4

# install all created files
sudo make install
sudo ldconfig

phymem=$(free|awk '/^Mem:/{print $2}')

if [ 1000000 -gt phymem ]
then
    # change the swap to be able to process the compiler
    sudo sed -i -r 's/^CONF_SWAPSIZE=[0-9]+/CONF_SWAPSIZE=1024/' /etc/dphys-swapfile
    sudo /etc/init.d/dphys-swapfile stop
    sudo /etc/init.d/dphys-swapfile start
fi
