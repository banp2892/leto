#include "DataSend.h"
#include <iostream>
#include <stdexcept>

DataSend::DataSend(const std::string& ip, int port)
    : serverIp(ip), serverPort(port), connected(false), sock(INVALID_SOCKET) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Ошибка инициализации Winsock");
    }
}

DataSend::~DataSend() {
    disconnect();
    WSACleanup();
}

bool DataSend::connectToServer() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета\n";
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения к серверу\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    connected = true;
    return true;
}

bool DataSend::sendData(const std::string& data) {
    if (!connected) {
        std::cerr << "Не подключено к серверу\n";
        return false;
    }
    int result = send(sock, data.c_str(), static_cast<int>(data.size()), 0);
    return result != SOCKET_ERROR;
}

void DataSend::disconnect() {
    if (connected) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        connected = false;
    }
}


