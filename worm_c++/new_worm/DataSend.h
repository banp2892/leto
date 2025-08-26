#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class DataSend {
private:
    SOCKET sock;
    std::string serverIp;
    int serverPort;
    bool connected;

public:
    DataSend(const std::string& ip, int port);
    ~DataSend();

    bool connectToServer();
    bool sendData(const std::string& data);
    void disconnect();
};
