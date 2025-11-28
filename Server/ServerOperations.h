//
// Created by pedro-souza on 25/11/2025.
//

#pragma once
#include "../Network/Platforms.h"
#include "Server.h"
#include <cstdlib>

namespace  ServerOperations {
    void sendSingleResponseToClient(
        const Server *server,
        uint32_t nonce,
        uint16_t sequence,
        uint8_t flag,
        sockaddr_in *clientAddr
    );
};


