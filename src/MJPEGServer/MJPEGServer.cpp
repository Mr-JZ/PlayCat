#include "MJPEGServer.h"

MJPEGServer::MJPEGServer(int port) {
    this->port = port;
    running = false;
}

MJPEGServer::~MJPEGServer() {
    stop();
}

void MJPEGServer::start() {
    if (!running) {
        running = true;
        shouldExit = false;
        imageEncoderThread = std::thread([this] { this->imageEncoder(); });

        serverThread = std::thread([this] { this->server(); });
    }
}

void MJPEGServer::stop() {
    if (running) {
        clientThreadsMutex.lock();
        shouldExit = true;
        clientThreadsMutex.unlock();
        {
            std::lock_guard<std::mutex> lock(currentFrameMutex);
            frameReady.notify_one();
        }
        {
            std::lock_guard<std::mutex> lock(frameEncodedMutex);
            frameEncoded.notify_all();
        }

        imageEncoderThread.join();

        // would need to be interrupted in accept()
        // serverThread.join();
    }
}

void MJPEGServer::serve(cv::Mat* frame) {
    currentFrameMutex.lock();
    currentFrame = frame->clone();
    currentFrameMutex.unlock();
    frameReady.notify_one();
}

void MJPEGServer::imageEncoder() {
    while (!shouldExit) {
        std::vector<u_char> buf;
        {
            std::unique_lock<std::mutex> lock(currentFrameMutex);
            frameReady.wait(lock);
            if (shouldExit) {
                break;
            }

            std::vector<int> params;
            params.push_back(cv::IMWRITE_JPEG_QUALITY);
            params.push_back(80);
            cv::imencode(".jpg", currentFrame, buf, params);
        }
        // signal that we encoded a frame
        {
            std::lock_guard<std::mutex> lock(frameEncodedMutex);
            encodedImageBuf = buf;
            frameEncoded.notify_all();
        }
    }
}

void MJPEGServer::server() {
    namespace beast = boost::beast;
    namespace net = boost::asio;
    using tcp = boost::asio::ip::tcp;

    boost::asio::io_context ioService{1};
    // acceptor receives incoming connections
    tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), port));

    while (!shouldExit) {
        try {
            // this will receive new connections
            // tcp::socket socket{ioService};
            tcp::socket* socket = new tcp::socket(ioService);

            // block until we get a connection
            acceptor.accept(*socket);

            clientThreadsMutex.lock();
            if (shouldExit) {
                socket->shutdown(tcp::socket::shutdown_send);
                socket->close();
                delete socket;
                break;
            }
            // start a thread to handle the connection
            std::thread thread([this, socket] { this->client(socket); });
            // detach it
            thread.detach();
            clientThreadsMutex.unlock();

        } catch (const std::exception& e) {
            std::cout << "exception in server thread: " << e.what()
                      << std::endl;
        }
    }
}

void MJPEGServer::client(boost::asio::ip::tcp::socket* socket) {
    namespace http = boost::beast::http;
    using tcp = boost::asio::ip::tcp;

    boost::system::error_code ec;

    boost::beast::flat_buffer buffer;

    try {
        // read the request
        http::request<http::string_body> req;
        http::read(*socket, buffer, req, ec);

        if (ec != http::error::end_of_stream && !ec) {

            // make sure we can handle the request
            if (req.method() != http::verb::get) {
                socket->write_some(
                    boost::asio::buffer("HTTP/1.1 405 Method Not Allowed\n"
                                        "Server: PlayCat\n\n"
                                        "unknown method"),
                    ec);

            } else {
                std::string boundary = "\n--123456789000000000000987654321\n";
                std::string contentType =
                    "Content-Type: image/jpeg\nContent-Length: ";

                socket->write_some(
                    boost::asio::buffer(
                        "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\n"
                        "Content-Type: multipart/x-mixed-replace; "
                        "boundary=123456789000000000000987654321\n"
                        "Server: PlayCat\n\n"),
                    ec);

                socket->write_some(boost::asio::buffer(boundary), ec);

                while (!shouldExit && socket->is_open()) {
                    std::string data;
                    int length;
                    {
                        std::unique_lock<std::mutex> lock(frameEncodedMutex);
                        frameEncoded.wait(lock);
                        if (shouldExit) {
                            break;
                        }
                        data = std::string(encodedImageBuf.begin(),
                                           encodedImageBuf.end());
                        length = encodedImageBuf.size();
                    }
                    socket->write_some(
                        boost::asio::buffer(contentType +
                                            std::to_string(length) + "\n\n"),
                        ec);

                    // write image data
                    socket->write_some(boost::asio::buffer(data), ec);

                    socket->write_some(boost::asio::buffer(boundary), ec);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << "exception in client handler: " << e.what() << std::endl;
    }

    socket->shutdown(tcp::socket::shutdown_send);
    socket->close();
    delete socket;
}
