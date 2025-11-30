//
// Created by pedro-souza on 28/11/2025.
//
#include <memory>
#include <unordered_map>
#include "./ClientState.h"
#include "../Server/InputData.h"
#include "../Server/DataObjects.h"

class GameState {
public:
    GameState();
    void AddClient(int id);
    void RemoveClient(int id);

    void UpdateStateWithInput(const InputData *command, int id, float deltaTime);
    void UpdateState(float deltaTime);

    [[nodiscard]] RawState GetRawState(int id);
private:
    static constexpr int MAX_CLIENTS = 10;
    std::unordered_map<int, std::unique_ptr<ClientState>> mClients;
};
