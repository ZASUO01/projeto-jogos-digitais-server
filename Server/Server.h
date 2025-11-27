//
// Created by pedro-souza on 24/11/2025.
//
#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include <utility>
#include <chrono>
#include "../Network/Socket.h"

enum class ServerState {
    SERVER_DOWN,
    SERVER_RUNNING
};

struct client  {
    uint32_t nonce{};
    sockaddr_in addr{};
    std::chrono::steady_clock::time_point lastUpdate;
};

class Server {
public:
    Server();
    ~Server() = default;

    bool Initialize();
    void ReadInputs();
    void Shutdown();

    [[nodiscard]] int GetSocket() const { return mSocket; }
private:
    void InitServerOperations();
    void StopServerOperations();
    void ReceivePackets();
    void HandleSynPacket(const Packet *pk, sockaddr_in* addr4);
    void HandleAckPacket(const Packet *pk, const sockaddr_in* addr4);
    void HandleDataPacket(const Packet *pk);
    void HandleActiveConnections();

    void Quit();
    static void PrintLocalIpAddr();
    static void Helper();
    void PrintClients();

    ServerState mState;
    SocketType mSocket;
    bool mRunning;

    // Connections control
    static constexpr int MAX_CONNECTION_REQUESTS = 10;
    static constexpr int MAX_CONNECTIONS = 10;
    std::vector<std::pair<uint32_t, uint32_t>> mConnectionRequests;
    std::vector<client> mConnectedClients;

    // packets control

    // Threads
    static constexpr int CONNECTIONS_CHECK_SLEEP_SECONDS = 1;
    static constexpr int CONNECTION_TIMEOUT_SECONDS = 10;
    std::mutex mMutex;
    std::thread mReceivingThread;
    std::thread mConnectionsCheckThread;
};