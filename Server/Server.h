//
// Created by pedro-souza on 24/11/2025.
//
#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include "../Game/GameState.h"
#include "../Network/Socket.h"
#include "../Utils/ThreadQueue.h"
#include "DataObjects.h"

enum class ServerState {
    SERVER_DOWN,
    SERVER_RUNNING,
};

class Server {
public:
    Server();
    ~Server();

    void Initialize();
    void ReadInputs();
    void Shutdown();

    [[nodiscard]] int GetSocket() const { return mSocket; }

    // Connections control
    Connection* GetConnection(int clientId);
private:
    // operations Control
    void InitServerOperations();
    void StopServerOperations();

    // Thread workers
    void ReceivePackets();
    void CheckConnections();
    void ProcessState();

    // Operations used by the thread workers
    void HandleSynPacket(const Packet *pk, sockaddr_in* addr4);
    void HandleAckPacket(const Packet *pk, const sockaddr_in* addr4);
    void HandleDataPacket(const Packet *pk);
    void HandleEndPacket(const Packet *pk);
    void HandlePingPacket(const Packet *pk);
    void BroadcastState();

    // Helpers called by the server inputs
    void Quit();
    static void PrintLocalIpAddr();
    static void Helper();
    void PrintClients();
    void PrintState();

    // General
    ServerState mState;
    SocketType mSocket;
    bool mRunning;

    // Connections control
    static constexpr int MAX_CONNECTION_REQUESTS = 4;
    static constexpr int MAX_CONNECTIONS = 4;
    std::vector<ConnectionRequest> mConnectionRequests;
    std::vector<Connection> mConnectedClients;

    // Threads
    static constexpr int CONNECTIONS_CHECK_SLEEP_SECONDS = 1;
    static constexpr int CONNECTION_TIMEOUT_SECONDS = 5;
    std::mutex mMutex;
    std::mutex mStateMutex;
    std::thread mReceivingThread;
    std::thread mConnectionsChecker;
    std::thread mStateProcessingThread;

    // Game state control
    static constexpr int GAME_STATE_TICK_RATE = 60;
    GameState *mGameState;

    // Client commands control
    ThreadQueue<ClientCommand> mCommandsQueue;
};