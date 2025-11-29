//
// Created by pedro-souza on 24/11/2025.
//
#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include "./Connections.h"
#include "../Game/GameState.h"
#include "../Network/Socket.h"
#include "../Utils/ThreadQueue.h"

enum class ServerState {
    SERVER_DOWN,
    SERVER_RUNNING,
};

struct ClientCommand {
    InputData data;
    int clientId;
};

class Server {
public:
    Server();
    ~Server();

    void Initialize();
    void ReadInputs();
    void Shutdown();

    [[nodiscard]] int GetSocket() const { return mSocket; }
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
    void SendStateToClients();

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
    static constexpr int MAX_CONNECTION_REQUESTS = 10;
    static constexpr int MAX_CONNECTIONS = 10;
    static int currentClientId;
    std::vector<ConnectionRequest> mConnectionRequests;
    std::vector<Connection> mConnectedClients;

    // Threads
    static constexpr int CONNECTIONS_CHECK_SLEEP_SECONDS = 1;
    static constexpr int CONNECTION_TIMEOUT_SECONDS = 10;
    std::mutex mMutex;
    std::thread mReceivingThread;
    std::thread mConnectionsChecker;
    std::thread mStateProcessingThread;

    // Game state control
    static constexpr int GAME_STATE_TICK_RATE = 60;
    GameState *mGameState;
    std::mutex mCommandsMutex;
    ThreadQueue<ClientCommand> mCommandsQueue;
};