//
// Created by pedro-souza on 28/11/2025.
//

#pragma once
#include <chrono>
#include "../Network/Platforms.h"

// It represents a client connection request,
// and it is used while the client is not connected yet
struct ConnectionRequest {
    uint32_t requestNonce;
    uint32_t connectionNonce;
    std::chrono::steady_clock::time_point lastRequestTime;

    ConnectionRequest() : requestNonce(0), connectionNonce(0) {}
};

// It represents a client connected to the server
// and contains all the connection information
struct Connection  {
    uint32_t nonce;
    sockaddr_in addr;
    std::chrono::steady_clock::time_point lastUpdate;
    int clientId;
    bool disconnectionCall;

    Connection()
        :nonce(0)
        ,addr{}
        ,clientId(-1)
        ,disconnectionCall(false){}
};