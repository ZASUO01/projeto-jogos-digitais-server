//
// Created by pedro-souza on 24/11/2025.
//
#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include <optional>
#include <chrono>

#include "../Network/Socket.h"

enum class ServerState {
    SERVER_DOWN,
    SERVER_RUNNING
};

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
    int stateId;

    Connection()
        :nonce(0),
        addr{},
        stateId(-1){}
};

class Server {
public:
    Server();
    ~Server() { Shutdown(); }

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
    void HandleEndPacket(const Packet *pk);
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
    static int currentStateId;
    std::vector<ConnectionRequest> mConnectionRequests;
    std::vector<Connection> mConnectedClients;

    // packets control

    // Threads
    static constexpr int CONNECTIONS_CHECK_SLEEP_SECONDS = 1;
    static constexpr int CONNECTION_TIMEOUT_SECONDS = 10;
    std::mutex mMutex;
    std::thread mReceivingThread;
    std::thread mConnectionsCheckThread;
};