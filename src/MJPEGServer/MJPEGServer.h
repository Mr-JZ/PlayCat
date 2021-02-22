#ifndef MJPEG_SERVER_H
#define MJPEG_SERVER_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

class MJPEGServer {
  public:
    MJPEGServer(int port);
    ~MJPEGServer();
    void start();
    void stop();
    void serve(cv::Mat* frame);

  private:
    int port;

    std::atomic<bool> running{false};
    std::atomic<bool> shouldExit{false};

    std::thread serverThread;
    void server();

    void client(boost::asio::ip::tcp::socket* socket);

    std::mutex currentFrameMutex;
    cv::Mat currentFrame;
    std::condition_variable frameReady;

    std::thread imageEncoderThread;
    void imageEncoder();
    std::mutex frameEncodedMutex;
    std::condition_variable frameEncoded;
    std::vector<u_char> encodedImageBuf;

    std::mutex clientThreadsMutex;
    int clientThreadsCount;
    std::condition_variable clientThreadExited;
};

#endif // MJPEG_SERVER_H