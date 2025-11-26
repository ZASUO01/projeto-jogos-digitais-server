//
// Created by pedro-souza on 24/11/2025.
//
#pragma once
#include "Platforms.h"
#include <cstdint>
#include <string>

namespace Addresses {
    void initAddrAnyV4(sockaddr_in *addr4, unsigned int port);
    bool parseAddrV4(sockaddr_in *addr, const char *addrStr, uint16_t port);
    std::string getLocalIpV4();
};