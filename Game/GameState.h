//
// Created by pedro-souza on 28/11/2025.
//
#include <memory>
#include <unordered_map>
#include "./ClientState.h"
#include "../Server/InputData.h"

struct RawState {
    float posX, posY;

    RawState() : posX(0), posY(0) {}
};

class GameState {
public:
    GameState();
    void AddClient(int id);
    void RemoveClient(int id);
    void Update(const InputData *command, int id);
    [[nodiscard]] RawState GetRawState(int id);
private:
    static constexpr int MAX_CLIENTS = 10;
    std::unordered_map<int, std::unique_ptr<ClientState>> mClients;
};
