//
// Created by pedro-souza on 24/11/2025.
//

#include "Server.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "ServerOperations.h"
#include "../Network/Addresses.h"
#include "../Network/NetUtils.h"

Server::Server()
:mState(ServerState::SERVER_DOWN)
,mSocket(-1)
,mRunning(false)
,mGameState(nullptr)
{}

Server::~Server() {
    Shutdown();
}

void Server::Initialize() {
    std::lock_guard lock(mMutex);
    if (mState != ServerState::SERVER_DOWN) {
        return;
    }

    mSocket = SocketUtils::createSocketV4();
    SocketUtils::bindSocketToAnyV4(mSocket);
    mGameState = new GameState();
    mRunning = true;

    mReceivingThread = std::thread(&Server::ReceivePackets, this);
    std::cout << "init packets receiving\n";

    mConnectionsChecker = std::thread(&Server::CheckConnections, this);
    std::cout << "init connections checking\n";

    mStateProcessingThread = std::thread(&Server::ProcessState, this);
    std::cout << "init processing state\n";

    mState = ServerState::SERVER_RUNNING;
    std::cout << "server initialized\n";
}

void Server::ReadInputs() {
    {
        std::lock_guard lock(mMutex);
        if (mState != ServerState::SERVER_RUNNING) {
            return;
        }
    }

    while (true) {
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

        else if (line == "state") {
            PrintState();
        }
    }
}

void Server::Shutdown() {
    {
        std::lock_guard lock(mMutex);
        if (mState != ServerState::SERVER_RUNNING) {
            return;
        }
        mRunning = false;
    }

    if (mReceivingThread.joinable()) {
        mReceivingThread.join();
    }
    std::cout << "finished packets receiving\n";

    if (mConnectionsChecker.joinable()) {
        mConnectionsChecker.join();
    }
    std::cout << "finished checking active connections\n";

    if (mStateProcessingThread.joinable()) {
        mStateProcessingThread.join();
    }
    std::cout << "finished processing state\n";

    std::lock_guard lock(mMutex);
    delete mGameState;
    mGameState = nullptr;

    close_socket(mSocket);
    mSocket = -1;

    mState = ServerState::SERVER_DOWN;
    std::cout << "Server stopped" << std::endl;
}

Connection* Server::GetConnection(const int clientId) {
    const auto it = std::find_if(
        mConnectedClients.begin(),
        mConnectedClients.end(),
        [&clientId](const auto& conn) {
           return conn.clientId == clientId;
        }
    );

    if (it != mConnectedClients.end()) {
        return &(*it);
    }

    return nullptr;
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

void Server::CheckConnections() {
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

            for (auto it = mConnectedClients.begin(); it != mConnectedClients.end();) {
                if (const auto secondsPassed = std::chrono::duration_cast<std::chrono::seconds>(now - it->lastUpdate); secondsPassed >= timeout) {
                    const auto idToRemove = it->clientId;

                    mGameState->RemoveClient(idToRemove);

                    it = mConnectedClients.erase(it);
                } else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(sleepInterval);
    }
}

void Server::ProcessState() {
    constexpr float tickDuration = 1.0f / GAME_STATE_TICK_RATE;
    auto lastUpdateTime = std::chrono::steady_clock::now();

    while (true) {
        auto startTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> timeUpdatedByCommands(0.0f);

        {
            std::lock_guard lock(mMutex);
            if (!mRunning) {
                break;
            }

            ClientCommand command;
            while (mCommandsQueue.TryDequeue(command)) {
                if (Connection* conn = GetConnection(command.clientId)) {
                    mGameState->UpdateStateWithInput(&command.inputData, command.clientId, tickDuration);

                    conn->lastInputSequence = command.sequence;
                }
                timeUpdatedByCommands += std::chrono::duration<float>(tickDuration);
            }
        }
        auto realTimeElapsed = std::chrono::steady_clock::now() - lastUpdateTime;

        if (std::chrono::duration<float> timeDeficit = realTimeElapsed - timeUpdatedByCommands; timeDeficit > std::chrono::duration<float>(0.0f)) {
            if (int numEmptyTicks = static_cast<int>(std::floor(timeDeficit.count() / tickDuration)); numEmptyTicks > 0) {
                numEmptyTicks = std::min(numEmptyTicks, 5);

                for (int i = 0; i < numEmptyTicks; ++i) {
                    mGameState->UpdateState(tickDuration);
                }
            }
        }
        lastUpdateTime = std::chrono::steady_clock::now();

        BroadcastState();

        auto timeElapsedInThisCycle = std::chrono::steady_clock::now() - startTime;

        if (auto timeToWait = std::chrono::duration<float>(tickDuration) - timeElapsedInThisCycle; timeToWait > std::chrono::duration<float>(0)) {
            std::this_thread::sleep_for(timeToWait);
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
            mGameState->RemoveClient(conn->clientId);
            mConnectedClients.erase(conn);
        }
        return;
    }
    mConnectionRequests.erase(it);
    if (mConnectedClients.size() == MAX_CONNECTIONS) {
        return;
    }

    const int clientId = mGameState->AddClient();
    if (clientId < 0) {
        return;
    }

    Connection connection(
        clientNonce,
        *addr4,
        std::chrono::steady_clock::now(),
        clientId,
        false,
        0
    );
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

        const auto extract = static_cast<const Command*>(pk->GetData());
        const size_t extractSize = pk->GetLength() / sizeof(Command);
        const std::vector commands(extract, extract + extractSize);
        const uint32_t lastInputSequence = conn->lastInputSequence;

        for (const auto& command : commands) {
            if (command.sequence > lastInputSequence) {
                const ClientCommand clientCommand(conn->clientId, command.sequence, command.inputData);
                mCommandsQueue.Enqueue(clientCommand);
            }
        }
    }
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
        conn->disconnectionCall = true;

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

void Server::BroadcastState() {
    std::vector<Connection> clientsToSendTo;
    {
        std::lock_guard lock(mMutex);
        for (auto conn : mConnectedClients) {
            if (!conn.disconnectionCall) {
                clientsToSendTo.emplace_back(
                    conn.nonce,
                    conn.addr,
                    conn.lastUpdate,
                    conn.clientId,
                    conn.disconnectionCall,
                    conn.lastInputSequence
                );
            }
        }
    }

    for (auto& client : clientsToSendTo) {
        RawState raw = mGameState->GetRawState(client.clientId);
        FullState full(raw, client.lastInputSequence);

        ServerOperations::sendStateDataToClient(
           this,
           client.nonce,
           0,
           &client.addr,
           &full
       );
    }
}

void Server::Helper() {
    std::cout << "help: see available options" << std::endl;
    std::cout << "quit: quit the server" << std::endl;
    std::cout << "addr: show the server local ip address" << std::endl;
    std::cout << "clients: show the current connected clients" << std::endl;
    std::cout << "state: show the current game state" << std::endl;
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
        std::cout << connection.clientId << ": "<< ip << std::endl;
    }
}

void Server::PrintState() {
    std::lock_guard lock(mMutex);
    mGameState->Print();
}
