#pragma once
using namespace std;
#include <vector>
#include <string>

// обработка локальных сетей
std::vector<std::wstring> get_local_ip_and_subnet(); // находим маску и внутренний айпишник хоста 
std::vector<std::wstring> generate_ip_range(const std::wstring& ip, const std::wstring& mask); // по маске и айпи создаем вектор всех возможных хостов в нашей сети
bool is_host_alive(const std::wstring& ip); // проверка айпишника на работоспособность
bool is_port_open(const std::wstring& ip, int port, int timeout_ms = 200); // проверка айпишников на открытые порты
bool is_private_ip(unsigned long ip); // проверка является ли айпишник локальным?