//
// Created by pedro-souza on 24/11/2025.
//
#include "Addresses.h"
#include <cstring>

void Addresses::initAddrAnyV4(sockaddr_in *addr4, const unsigned int port) {
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY;
    addr4->sin_port = htons(port);
}

bool Addresses::parseAddrV4(sockaddr_in *addr, const char *addrStr, const uint16_t port) {
    in_addr addr_v4{};
    if (inet_pton(AF_INET, addrStr, &addr_v4) != 1) {
        return false;
    }

    addr->sin_addr = addr_v4;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    return true;
}

std::string Addresses::getLocalIpV4() {
    const std::string dnsIp = "8.8.8.8";
    std::string resultIp;
    int sock = -1;

    sock = create_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return "";
    }

    sockaddr_in servAddr{};
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(53);

    if (inet_pton(AF_INET, dnsIp.c_str(), &servAddr.sin_addr) <= 0) {
        close_socket(sock);
        return "";
    }

    if (connect(sock, reinterpret_cast<const sockaddr *>(&servAddr), sizeof(servAddr)) < 0) {
        close_socket(sock);
        return "";
    }

    sockaddr_in localAddr{};
    socklen_t addr_len = sizeof(localAddr);

    if (getsockname(sock, reinterpret_cast<sockaddr *>(&localAddr), &addr_len) < 0) {
        close_socket(sock);
        return "";
    }

    char ip_buffer[INET_ADDRSTRLEN];

    if (const char *conversionResult = inet_ntop(AF_INET, &localAddr.sin_addr, ip_buffer, INET_ADDRSTRLEN); conversionResult != nullptr) {
        resultIp = ip_buffer;
    }

    close_socket(sock);

    return resultIp;
}