#pragma comment(lib, "ws2_32.lib")

#include<iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include "NetUtils.h"
#include <IcmpAPI.h>


// Проверка, является ли IPv4 адрес частным
bool is_private_ip(unsigned long ip) { 
    ip = ntohl(ip); // корректный порядок байт
    unsigned char b1 = (ip >> 24) & 0xFF;
    unsigned char b2 = (ip >> 16) & 0xFF;

    if (b1 == 10) return true;
    if (b1 == 192 && b2 == 168) return true;
    if (b1 == 172 && (b2 >= 16 && b2 <= 31)) return true;

    return false;
}


std::vector<std::wstring> get_local_ip_and_subnet() {
    std::vector<std::wstring> results;

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_INET;
    ULONG bufLen = 15000;
    std::vector<BYTE> buffer(bufLen);

    IP_ADAPTER_ADDRESSES* pAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());
    DWORD ret = GetAdaptersAddresses(family, flags, NULL, pAddresses, &bufLen);

    if (ret == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(bufLen);
        pAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());
        ret = GetAdaptersAddresses(family, flags, NULL, pAddresses, &bufLen);
    }

    if (ret != NO_ERROR) {
        std::wcout << L"GetAdaptersAddresses failed: " << ret << std::endl;
        return results;
    }

    for (IP_ADAPTER_ADDRESSES* adapter = pAddresses; adapter; adapter = adapter->Next) {
        // Только активные адаптеры, исключаем loopback и туннели
        if (adapter->OperStatus != IfOperStatusUp) continue;
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        if (adapter->IfType == IF_TYPE_TUNNEL) continue;

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;
            unicast; unicast = unicast->Next) {

            SOCKADDR_IN* sa_in = reinterpret_cast<SOCKADDR_IN*>(unicast->Address.lpSockaddr);
            unsigned long ip = sa_in->sin_addr.S_un.S_addr;

            if (!is_private_ip(ip)) continue; // фильтруем только частные IP

            // Преобразуем IP в строку
            wchar_t ipStr[INET_ADDRSTRLEN] = {};
            InetNtopW(AF_INET, &sa_in->sin_addr, ipStr, INET_ADDRSTRLEN);

            // Маска подсети через OnLinkPrefixLength
            ULONG mask = 0xFFFFFFFF << (32 - unicast->OnLinkPrefixLength);
            IN_ADDR maskAddr;
            maskAddr.S_un.S_addr = htonl(mask);
            wchar_t maskStr[INET_ADDRSTRLEN] = {};
            InetNtopW(AF_INET, &maskAddr, maskStr, INET_ADDRSTRLEN);

            results.push_back(std::wstring(ipStr));
            results.push_back(std::wstring(maskStr));

            return results; // возвращаем первый подходящий IP
        }
    }

    return results;
}


std::vector<std::wstring> generate_ip_range(const std::wstring& ip, const std::wstring& mask)
{
    std::vector<std::wstring> ips;

    IN_ADDR ip_addr, mask_addr;

    // Преобразуем строки в бинарный вид
    if (InetPtonW(AF_INET, ip.c_str(), &ip_addr) != 1) return ips;
    if (InetPtonW(AF_INET, mask.c_str(), &mask_addr) != 1) return ips;

    ULONG ip_ul = ntohl(ip_addr.S_un.S_addr);
    ULONG mask_ul = ntohl(mask_addr.S_un.S_addr);

    // Сеть и широковещательный адрес
    ULONG network = ip_ul & mask_ul;
    ULONG broadcast = ip_ul | ~mask_ul;

    // Перебор всех адресов внутри диапазона (без network и broadcast)
    for (ULONG addr = network + 1; addr < broadcast; ++addr)
    {
        IN_ADDR a;
        a.S_un.S_addr = htonl(addr);

        wchar_t buf[INET_ADDRSTRLEN];
        if (InetNtopW(AF_INET, &a, buf, _countof(buf)))
        {
            ips.push_back(buf);
        }
    }

    return ips;
}

bool is_host_alive(const std::wstring& ip)
{
    HANDLE hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) return false;

    IN_ADDR ipAddr{};
    if (InetPtonW(AF_INET, ip.c_str(), &ipAddr) != 1) {
        IcmpCloseHandle(hIcmpFile);
        return false;
    }

    char ReplyBuffer[sizeof(ICMP_ECHO_REPLY) + 8];
    DWORD ReplySize = sizeof(ReplyBuffer);

    DWORD retVal = IcmpSendEcho(hIcmpFile, ipAddr.S_un.S_addr, nullptr, 0, nullptr, ReplyBuffer, ReplySize, 500);
    IcmpCloseHandle(hIcmpFile);

    return retVal != 0; // 0 — хост не отвечает
}

bool is_port_open(const std::wstring& ip, int port, int timeout_ms)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (InetPtonW(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        closesocket(sock);
        WSACleanup();
        return false;
    }

    // Таймаут соединения
    timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    int result = connect(sock, (sockaddr*)&addr, sizeof(addr));
    closesocket(sock);
    WSACleanup();

    return result == 0;
}
