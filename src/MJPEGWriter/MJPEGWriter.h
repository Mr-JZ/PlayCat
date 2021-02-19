#ifndef MJPEG_WRITER_H
#define MJPEG_WRITER_H

#ifdef __linux__
#define MJPEG_WRITER_USABLE

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT unsigned short
#define SOCKET int
#define HOSTENT struct hostent
#define SOCKADDR struct sockaddr
#define SOCKADDR_IN struct sockaddr_in
#define ADDRPOINTER unsigned int*
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define TIMEOUT_M 200000
#define NUM_CONNECTIONS 10

#include "opencv2/opencv.hpp"
#include <iostream>
#include <mutex>
#include <thread>
#include <stdio.h>
#include <string.h>

using namespace cv;
using namespace std;

struct clientFrame {
    uchar* outbuf;
    int outlen;
    int client;
};

struct clientPayload {
    void* context;
    clientFrame cf;
};

class MJPEGWriter {
    SOCKET sock;
    fd_set master;
    int timeout;
    int quality; // jpeg compression [1..100]
    std::vector<int> clients;
    std::thread thread_listen, thread_write;
    std::mutex mutex_client;
    std::mutex mutex_cout;
    std::mutex mutex_writer;
    std::mutex mutex_frame_replace;
    Mat lastFrame;
    int port;

    int _write(int sock, char* s, int len) {
        if (len < 1) {
            len = strlen(s);
        }
        {
            try {
                int retval = ::send(sock, s, len, 0x4000);
                return retval;
            } catch (int e) {
                cout << "An exception occurred. Exception Nr. " << e << '\n';
            }
        }
        return -1;
    }

    int _read(int socket, char* buffer) {
        int result;
        result = recv(socket, buffer, 4096, MSG_PEEK);
        if (result < 0) {
            cout << "An exception occurred. Exception Nr. " << result << '\n';
            return result;
        }
        string s = buffer;
        buffer = (char*)s.substr(0, (int)result).c_str();
        return result;
    }

    static void* listen_Helper(void* context) {
        ((MJPEGWriter*)context)->Listener();
        return NULL;
    }

    static void* writer_Helper(void* context) {
        ((MJPEGWriter*)context)->Writer();
        return NULL;
    }

    static void* clientWrite_Helper(void* payload) {
        void* ctx = ((clientPayload*)payload)->context;
        struct clientFrame cf = ((clientPayload*)payload)->cf;
        ((MJPEGWriter*)ctx)->ClientWrite(cf);
        return NULL;
    }

  public:
    MJPEGWriter(int port = 0)
        : sock(INVALID_SOCKET), timeout(TIMEOUT_M), quality(80), port(port) {
        signal(SIGPIPE, SIG_IGN);
        FD_ZERO(&master);
        // if (port)
        //     open(port);
    }

    ~MJPEGWriter() {
        release();
    }

    bool release() {
        if (sock != INVALID_SOCKET)
            shutdown(sock, 2);
        sock = (INVALID_SOCKET);
        return false;
    }

    bool open() {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN address;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        if (::bind(sock, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) ==
            SOCKET_ERROR) {
            cerr << "error : couldn't bind sock " << sock << " to port " << port
                 << "!" << endl;
            return release();
        }
        if (listen(sock, NUM_CONNECTIONS) == SOCKET_ERROR) {
            cerr << "error : couldn't listen on sock " << sock << " on port "
                 << port << " !" << endl;
            return release();
        }
        FD_SET(sock, &master);
        return true;
    }

    bool isOpened() {
        return sock != INVALID_SOCKET;
    }

    void start() {
        mutex_writer.lock();
        thread_listen = std::thread([this] { this->listen_Helper(this); });
        thread_write = std::thread([this] { this->writer_Helper(this); });
    }

    void stop() {
        this->release();
        thread_listen.join();
        thread_write.join();
    }

    void write(Mat frame) {
        mutex_frame_replace.lock();
        if (!frame.empty()) {
            lastFrame.release();
            lastFrame = frame.clone();
        }
        mutex_frame_replace.unlock();
    }

  private:
    void Listener();
    void Writer();
    void ClientWrite(clientFrame& cf);
};

#endif

#endif // MJPEG_WRITER_H
