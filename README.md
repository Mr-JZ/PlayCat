# PlayCat

This is a project to learn C++ and gain skills in openCV. In general we want a laser tower that plays with a cat.

## Debian dependencies

- build-essential cmake  
  for building
- libopencv-dev >= 4  
  currently has to installed from source because the version provided by debian buster seems to not include dnn  
  see [here](https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html) for a guide on how to do that
- libgtk2.0-dev pkg-config  
  to open and show a window
- git
- wget  
  for downloading model files
- ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev  
  if you want to stream the input video from somewhere
- libboost-all-dev
  for the web server

list for copying:  
git wget build-essential cmake libgtk2.0-dev pkg-config ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libboost-all-dev  
(remember to install opencv from source!)

## Setup

Clone this repository:

```bash
git clone "https://github.com/Mr-JZ/PlayCat.git"
```

Before you can detect any cats (or other objects),
you need to download a model for the opencv dnn module
to use for object detection.
This can be done by running

```bash
./getModel.sh
```

from inside the models folder, which will fetch all needed files.

If you want to use another model, make sure to edit
models/classes.txt accordingly.  
See the [tensorflow model zoo](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1_detection_zoo.md) for other models you might be able to use.  
(Note: Currently dnn seems to not understand Tensorflow v2 models.)

## Building

Create the build directory and cd into it:

```bash
mkdir -p build && cd build
```

From inside the build directory run:

```bash
cmake ..
make -j4
```

to configure and build the project.  
You may change "-j4" to "-j{number of threads to use for compiling}"
to speed things up a little.

Then run it:

```bash
./PlayCat
```

You may need to resize the window.

If you are using VSCode and the cmake extension, you need to initially open the cmake tab and click on "Configure all Projects" at the top.  
Then select "Unsepcified" from the drop down.  
After that you can press f7 to build and shitf + f5 to build and run.

## Code style

Currently using clang-format.  
The configuration might be debatable, but looks acceptable to me.  
This is included in the VSCode cmake extension; All you need to do to format the currently open file is press ctrl + shift + i.

## Development

Remember to add new source files to CMakeLists.txt.
