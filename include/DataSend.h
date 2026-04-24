#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

/**
 * @class DataSend
 * @brief Класс для управления сетевым соединением и отправки данных.
 * * Обеспечивает инициализацию Winsock, создание сокета, установку соединения
 * с сервером по протоколу TCP и передачу строковых данных.
 */
class DataSend {
private:
    SOCKET sock;         ///< Дескриптор сокета
    std::string serverIp; ///< IP-адрес сервера
    int serverPort;      ///< Порт сервера
    bool connected;      ///< Флаг состояния подключения

public:
    /**
     * @brief Конструктор класса. Инициализирует Winsock.
     * @param ip IP-адрес сервера (например, "127.0.0.1").
     * @param port Порт для подключения.
     * @throw std::runtime_error Если не удалось инициализировать библиотеку Winsock.
     */
    DataSend(const std::string& ip, int port);

    /**
     * @brief Деструктор. Закрывает соединение и очищает ресурсы Winsock.
     */
    ~DataSend();

    /**
     * @brief Устанавливает соединение с сервером.
     * @return true если подключение успешно, false в случае ошибки.
     */
    bool connectToServer();

    /**
     * @brief Отправляет строковые данные на сервер.
     * @param data Строка для отправки.
     * @return true если данные успешно отправлены, false если соединение отсутствует или произошла ошибка.
     */
    bool sendData(const std::string& data);

    /**
     * @brief Принудительно закрывает текущее соединение.
     */
    void disconnect();
};