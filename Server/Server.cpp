//
// Created by pedro-souza on 24/11/2025.
//

#include "Server.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "ServerOperations.h"
#include "../Network/Addresses.h"
#include "../Network/NetUtils.h"

Server::Server()
:mState(ServerState::SERVER_DOWN)
,mSocket(-1)
,mRunning(false)
{}

bool Server::Initialize() {
    if (mState != ServerState::SERVER_DOWN) {
        return false;
    }

    mSocket = SocketUtils::createSocketV4();
    SocketUtils::bindSocketToAnyV4(mSocket);
    mRunning = true;

    InitServerOperations();

    std::cout << "server initialized" << std::endl;

    mState = ServerState::SERVER_RUNNING;
    return true;
}

void Server::ReadInputs() {
    while (mRunning) {
        std::cout << "Type help to see the available commands" << std::endl;

        std::string line;
        std::cout << ">> ";

        if (!getline(std::cin, line)) {
            std::cout << "read error" << std::endl;
            break;
        }

        if (line.empty()) {
            continue;
        }

        if (line == "help") {
            Helper();
        }

        else if (line == "quit") {
            Quit();
            break;
        }

        else if (line == "addr") {
           PrintLocalIpAddr();
        }

        else if (line == "clients") {
            PrintClients();
        }
    }
}

void Server::Shutdown() {
    if (mState != ServerState::SERVER_RUNNING) {
        return;
    }

    StopServerOperations();

    close_socket(mSocket);

    mState = ServerState::SERVER_DOWN;

    std::cout << "Server stopped" << std::endl;
}

void Server::InitServerOperations() {
    if (mState != ServerState::SERVER_DOWN) {
        return;
    }

    mReceivingThread = std::thread(&Server::ReceivePackets, this);
}

void Server::StopServerOperations() {
    if (mState != ServerState::SERVER_RUNNING) {
        return;
    }

    if (mReceivingThread.joinable()) {
        mReceivingThread.join();
    }
}

void Server::ReceivePackets() {
    bool shouldRun = true;

    while (true) {
        {
            std::lock_guard lock(mMutex);
            if (!mRunning) {
                shouldRun = false;
            }
        }

        if (!shouldRun) {
            break;
        }

        if (!SocketUtils::socketReadyToReceive(mSocket, 0)) {
            continue;
        }

        Packet packet;
        sockaddr_in clientAddr{};

        if (!SocketUtils::receivePacketFromV4(
            mSocket,
            &packet,
            &clientAddr)) {
            continue;
        }

        if (!packet.IsValid()) {
            continue;
        }

        switch (packet.GetFlag()) {
            case Packet::SYN_FLAG: {
                HandleSynPacket(&packet, &clientAddr);
                break;
            }
            case Packet::ACK_FLAG: {
                HandleAckPacket(&packet, &clientAddr);
                break;
            }
            default:
                break;
        }
    }
}

void Server::HandleSynPacket(const Packet *pk, sockaddr_in* addr4) {
    std::lock_guard lock(mMutex);
    if (mConnectionRequests.size() == MAX_CONNECTION_REQUESTS) {
        return;
    }

    const uint32_t clientNonce = pk->GetNonce();
    const uint16_t clientSequence = pk->GetSequence();

    const auto it = std::find_if(
        mConnectionRequests.begin(),
        mConnectionRequests.end(),
        [&clientNonce](const std::pair<uint32_t, uint32_t>& request) {
            return request.first == clientNonce;
        }
    );

    const uint32_t serverNonce = NetUtils::getRandomNonce(clientNonce);

    if (it == mConnectionRequests.end()) {
        mConnectionRequests.emplace_back(clientNonce, serverNonce);
    }else {
        it->second = serverNonce;
    }

    const uint16_t serverSequence = clientSequence + 1;

    ServerOperations::sendSynAckToClient(this, serverNonce, serverSequence, addr4);
}

void Server::HandleAckPacket(const Packet *pk, const sockaddr_in *addr4) {
    const uint32_t clientNonce = pk->GetNonce();

    std::lock_guard lock(mMutex);
    const auto it = std::find_if(
       mConnectionRequests.begin(),
       mConnectionRequests.end(),
       [&clientNonce](const std::pair<uint32_t, uint32_t>& request) {
           return request.second == clientNonce;
       }
   );

    if (it == mConnectionRequests.end()) {
        return;
    }
    mConnectionRequests.erase(it);

    if (mConnectedClients.size() == MAX_CONNECTIONS) {
        return;
    }

    client c{clientNonce, *addr4};
    mConnectedClients.emplace_back(c);
}

void Server::Helper() {
    std::cout << "help: see available options" << std::endl;
    std::cout << "quit: quit the server" << std::endl;
    std::cout << "addr: show the server local ip address" << std::endl;
    std::cout << "clients: shown the current connected clients" << std::endl;
}

void Server::Quit() {
    std::cout << "shutting down..." << std::endl;
    std::lock_guard lock(mMutex);
    mRunning = false;
}

void Server::PrintLocalIpAddr() {
    const std::string addr = Addresses::getLocalIpV4();
    if (addr.empty()) {
        std::cout << "local ip error" << addr << std::endl;
        return;
    }

    std::cout << "local ip: " << addr << std::endl;
}

void Server::PrintClients() {
    std::lock_guard lock(mMutex);
    std::cout << "Total requests: " << mConnectionRequests.size() << std::endl;
    std::cout << "Total clients: " << mConnectedClients.size() << std::endl;

    char ip[INET_ADDRSTRLEN];
    for (auto [nonce, addr] : mConnectedClients) {
        inet_ntop(AF_INET, &addr, ip, INET_ADDRSTRLEN);
        std::cout << ip << std::endl;
    }
}


