#include "MJPEGWriter.h"

#ifdef MJPEG_WRITER_USABLE

#include <fstream>
void MJPEGWriter::Listener() {

    // send http header
    std::string header;
    header += "HTTP/1.0 200 OK\r\n";
    header += "Cache-Control: no-cache\r\n";
    header += "Pragma: no-cache\r\n";
    header += "Connection: close\r\n";
    header += "Content-Type: multipart/x-mixed-replace; "
              "boundary=123456789000000000000987654321\r\n\r\n";
    const int header_size = header.size();
    char* header_data = (char*)header.data();
    fd_set rread;
    SOCKET maxfd;
    this->open();
    mutex_writer.unlock();
    while (true) {
        rread = master;
        struct timeval to = {0, timeout};
        maxfd = sock + 1;
        if (sock == INVALID_SOCKET) {
            return;
        }
        int sel = select(maxfd, &rread, NULL, NULL, &to);
        if (sel > 0) {
            for (int s = 0; s < maxfd; s++) {
                if (FD_ISSET(s, &rread) && s == sock) {
                    int addrlen = sizeof(SOCKADDR);
                    SOCKADDR_IN address = {0};
                    SOCKET client =
                        accept(sock, (SOCKADDR*)&address, (socklen_t*)&addrlen);
                    if (client == SOCKET_ERROR) {
                        cerr << "error : couldn't accept connection on sock "
                             << sock << " !" << endl;
                        return;
                    }
                    maxfd = (maxfd > client ? maxfd : client);
                    mutex_cout.lock();
                    cout << "new client " << client << endl;
                    char headers[4096] = "\0";
                    int readBytes = _read(client, headers);
                    // cout << headers;
                    mutex_cout.unlock();
                    mutex_client.lock();
                    _write(client, header_data, header_size);
                    clients.push_back(client);
                    mutex_client.unlock();
                }
            }
        }
        usleep(1000);
    }
}

void MJPEGWriter::Writer() {
    mutex_writer.lock();
    mutex_writer.unlock();
    const int milis2wait = 16666;
    while (this->isOpened()) {
        mutex_client.lock();
        int num_connected_clients = clients.size();
        mutex_client.unlock();
        if (!num_connected_clients) {
            usleep(milis2wait);
            continue;
        }
        std::thread threads[NUM_CONNECTIONS];
        int count = 0;

        std::vector<uchar> outbuf;
        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(quality);
        mutex_writer.lock();
        mutex_frame_replace.lock();
        imencode(".jpg", lastFrame, outbuf, params);
        mutex_frame_replace.unlock();
        mutex_writer.unlock();
        int outlen = outbuf.size();

        mutex_client.lock();
        std::vector<int>::iterator begin = clients.begin();
        std::vector<int>::iterator end = clients.end();
        mutex_client.unlock();
        std::vector<clientPayload*> payloads;
        for (std::vector<int>::iterator it = begin; it != end; ++it, ++count) {
            if (count > NUM_CONNECTIONS)
                break;
            struct clientPayload* cp = new clientPayload(
                {(MJPEGWriter*)this, {outbuf.data(), outlen, *it}});
            payloads.push_back(cp);
            threads[count] =
                std::thread([cp] { MJPEGWriter::clientWrite_Helper(cp); });
        }
        for (; count > 0; count--) {
            threads[count - 1].join();
            delete payloads.at(count - 1);
        }
        usleep(milis2wait);
    }
}

void MJPEGWriter::ClientWrite(clientFrame& cf) {
    stringstream head;
    head << "--123456789000000000000987654321\r\nContent-Type: "
            "image/jpeg\r\nContent-Length: "
         << cf.outlen << "\r\n\r\n";
    string string_head = head.str();
    mutex_client.lock();
    _write(cf.client, (char*)string_head.c_str(), string_head.size());
    int n = _write(cf.client, (char*)(cf.outbuf), cf.outlen);
    if (n < cf.outlen) {
        std::vector<int>::iterator it;
        it = find(clients.begin(), clients.end(), cf.client);
        if (it != clients.end()) {
            cerr << "kill client " << cf.client << endl;
            clients.erase(
                std::remove(clients.begin(), clients.end(), cf.client));
            ::shutdown(cf.client, 2);
        }
    }
    mutex_client.unlock();
    // pthread_exit(NULL);
}

#endif
