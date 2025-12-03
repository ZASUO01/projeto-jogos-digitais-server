//
// Created by pedro-souza on 28/11/2025.
//
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include "./ClientState.h"
#include "../Server/InputData.h"
#include "../Server/DataObjects.h"

class GameState {
public:
    GameState();
    int AddClient();
    void RemoveClient(int id);
    void Print();

    static constexpr int WINDOW_WIDTH = 1024;
    static constexpr int WINDOW_HEIGHT = 768;

    void UpdateStateWithInput(const InputData *command, int id, float deltaTime);
    void UpdateState(float deltaTime);

    [[nodiscard]] RawState GetRawState(int id);
private:
    static constexpr int MAX_CLIENTS = 4;
    static const std::vector<std::string> CLIENT_NAMES;
    std::vector<bool> mUsedNames;
    static int uniqueID;

    int mHighScore;
    std::string mHighScoreName;

    std::unordered_map<int, std::unique_ptr<ClientState>> mClients;
};
