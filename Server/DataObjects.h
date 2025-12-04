//
// Created by pedro-souza on 30/11/2025.
//

#pragma once
#include <cstdint>
#include <chrono>
#include "InputData.h"
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
    uint32_t lastInputSequence;

    Connection(
        const uint32_t nonce,
        const sockaddr_in& addr,
        const std::chrono::steady_clock::time_point lastUpdate,
        const int id,
        const bool disconnectionCall,
        const uint32_t lastInputSequence)
        :nonce(nonce)
        ,addr(addr)
        ,lastUpdate(lastUpdate)
        ,clientId(id)
        ,disconnectionCall(disconnectionCall)
        ,lastInputSequence(lastInputSequence){}
};


// Commands to be sent to the server
struct Command {
    uint32_t sequence;
    InputData inputData;

    Command(const uint32_t sequence, const InputData &inputData)
    :sequence(sequence), inputData(inputData) {}
};

struct ClientCommand {
    int clientId;
    uint32_t sequence;
    InputData inputData;

    ClientCommand()
    :clientId(-1), sequence(0) {}

    ClientCommand(const int id, const uint32_t seq, const InputData &input)
    :clientId(id), sequence(seq), inputData(input) {}
};

struct RawState {
    float posX, posY, rotation;

    RawState() : posX(0), posY(0), rotation(0) {}
};

struct OtherState {
    int id;
    float posX, posY, rotation;

    OtherState(const int id, const float x, const float y, const float rot) :id(id), posX(x), posY(y), rotation(rot) {}
    OtherState() :id(-1), posX(0), posY(0), rotation(0) {}
};

#define MAX_OTHER_STATES 3

struct FullState {
    RawState rawState;
    OtherState otherStates[MAX_OTHER_STATES];
    size_t otherStateSize;
    uint32_t lastConfirmedInputSequence;

    FullState(const RawState &raw,const uint32_t sequence)
    :rawState(raw), otherStateSize(0), lastConfirmedInputSequence(sequence) {}
};
