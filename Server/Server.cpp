//
// Created by pedro-souza on 24/11/2025.
//

#include "Server.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "InputData.h"
#include "ServerOperations.h"
#include "../Network/Addresses.h"
#include "../Network/NetUtils.h"

int Server::currentStateId = 0;

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

    std::cout << "server initialized\n";

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
    std::cout << "init packets receiving\n";

    mConnectionsCheckThread = std::thread(&Server::HandleActiveConnections, this);
    std::cout << "init connections checking\n";
}

void Server::StopServerOperations() {
    if (mReceivingThread.joinable()) {
        mReceivingThread.join();
    }
    std::cout << "finished packets receiving\n";

    if (mConnectionsCheckThread.joinable()) {
        mConnectionsCheckThread.join();
    }
    std::cout << "finished checking active connections\n";
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
            case Packet::DATA_FLAG:{
                HandleDataPacket(&packet);
                break;
            }
            case Packet::END_FLAG: {
                HandleEndPacket(&packet);
                break;
            }
            default:
                break;
        }
    }
}

void Server::HandleSynPacket(const Packet *pk, sockaddr_in* addr4) {
    std::lock_guard lock(mMutex);
    if (mConnectedClients.size() == MAX_CONNECTIONS ||
        mConnectionRequests.size() == MAX_CONNECTION_REQUESTS) {
        return;
    }

    const uint32_t clientNonce = pk->GetNonce();
    const uint16_t clientSequence = pk->GetSequence();

    const auto it = std::find_if(
        mConnectionRequests.begin(),
        mConnectionRequests.end(),
        [&clientNonce](const ConnectionRequest& request) {
            return request.requestNonce == clientNonce;
        }
    );

    const uint32_t serverNonce = NetUtils::getRandomNonce(clientNonce);

    if (it == mConnectionRequests.end()) {
        ConnectionRequest request;
        request.requestNonce = clientNonce;
        request.connectionNonce = serverNonce;
        request.lastRequestTime = std::chrono::steady_clock::now();

        mConnectionRequests.emplace_back(request);
    }else {
        it->connectionNonce = serverNonce;
        it->lastRequestTime = std::chrono::steady_clock::now();
    }

    const uint16_t serverSequence = clientSequence + 1;
    ServerOperations::sendSingleResponseToClient(
        this,
        serverNonce,
        serverSequence,
        Packet::SYN_ACK_FLAG,
        addr4
    );
}

void Server::HandleAckPacket(const Packet *pk, const sockaddr_in *addr4) {
    const uint32_t clientNonce = pk->GetNonce();

    std::lock_guard lock(mMutex);
    const auto it = std::find_if(
       mConnectionRequests.begin(),
       mConnectionRequests.end(),
       [&clientNonce](const ConnectionRequest& request) {
           return request.connectionNonce == clientNonce;
       }
   );

    if (it == mConnectionRequests.end()) {
        // Check end ACK
        const auto conn = std::find_if(
            mConnectedClients.begin(),
            mConnectedClients.end(),
            [&clientNonce](const Connection& connection) {
                return connection.nonce == clientNonce;
            }
        );
        if (conn != mConnectedClients.end()) {
            mConnectedClients.erase(conn);
        }
        return;
    }
    mConnectionRequests.erase(it);
    if (mConnectedClients.size() == MAX_CONNECTIONS) {
        return;
    }

    Connection connection;
    connection.nonce = clientNonce;
    connection.addr = *addr4;
    connection.lastUpdate = std::chrono::steady_clock::now();
    connection.stateId = currentStateId;
    currentStateId++;

    mConnectedClients.emplace_back(connection);
}

void Server::HandleDataPacket(const Packet *pk) {
    const auto clientNonce = pk->GetNonce();

    std::lock_guard lock(mMutex);
    const auto conn = std::find_if(
        mConnectedClients.begin(),
        mConnectedClients.end(),
        [&clientNonce](const Connection& connection) {
           return connection.nonce == clientNonce;
    });

    if (conn != mConnectedClients.end()) {
        conn->lastUpdate = std::chrono::steady_clock::now();
    }

    /*
    const auto data = static_cast<const InputData *>(pk->GetData());
    std::cout << "received from: " << clientNonce << "\n";
    */
}

void Server::HandleEndPacket(const Packet *pk) {
    const auto clientNonce = pk->GetNonce();
    const auto clientSequence = pk->GetSequence();

    std::lock_guard lock(mMutex);
    const auto conn = std::find_if(
        mConnectedClients.begin(),
        mConnectedClients.end(),
        [&clientNonce](const Connection& connection) {
           return connection.nonce == clientNonce;
    });

    if (conn != mConnectedClients.end()) {
        conn->lastUpdate = std::chrono::steady_clock::now();

        const uint16_t serverSequence = clientSequence + 1;
        ServerOperations::sendSingleResponseToClient(
            this,
            conn->nonce,
            serverSequence,
            Packet::END_ACK_FLAG,
            &conn->addr
        );
    }
}

void Server::HandleActiveConnections() {
    const auto sleepInterval = std::chrono::seconds(CONNECTIONS_CHECK_SLEEP_SECONDS);
    const auto timeout = std::chrono::seconds(CONNECTION_TIMEOUT_SECONDS);
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

        {
            std::lock_guard lock(mMutex);
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

            auto requestsEnd = std::remove_if(
                mConnectionRequests.begin(),
                mConnectionRequests.end(),
                [&now, &timeout](const ConnectionRequest& request) {
                    const auto secondsPassed= std::chrono::duration_cast<std::chrono::seconds>(now - request.lastRequestTime);

                    return secondsPassed >= timeout;
                }
            );
            mConnectionRequests.erase(requestsEnd, mConnectionRequests.end());

            auto connectionsEnd = std::remove_if(
                mConnectedClients.begin(),
                mConnectedClients.end(),
                [&now, &timeout](const Connection& connection) {
                    const auto secondsPassed= std::chrono::duration_cast<std::chrono::seconds>(now - connection.lastUpdate);

                    return secondsPassed >= timeout;
                }
            );
            mConnectedClients.erase(connectionsEnd, mConnectedClients.end());
        }
        std::this_thread::sleep_for(sleepInterval);
    }
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
    for (auto connection : mConnectedClients) {
        inet_ntop(AF_INET, &connection.addr, ip, INET_ADDRSTRLEN);
        std::cout << connection.stateId << ": "<< ip << std::endl;
    }
}


