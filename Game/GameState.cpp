//
// Created by pedro-souza on 28/11/2025.
//

#include "GameState.h"
#include <algorithm>
#include <iostream>
#include <iostream>

int GameState::uniqueID = 0;

const std::vector<std::string> GameState::CLIENT_NAMES = {
    "CodeLancer",
    "DataPulse",
    "VoltEdge",
    "NetGhost"
};

GameState::GameState() {
    mUsedNames = { false, false , false , false};
}

int GameState::AddClient() {
    if (mClients.size() == MAX_CLIENTS) {
        return -1;
    }

    const auto it = std::find(mUsedNames.begin(), mUsedNames.end(), false);

    if (it == mUsedNames.end()) {
        return -1;
    }

    const long idx = std::distance(mUsedNames.begin(), it);
    std::string name = CLIENT_NAMES[idx];
    mUsedNames[idx] = true;

    int id = uniqueID;

    mClients.emplace(id, std::make_unique<ClientState>(id, static_cast<int>(idx), name));

    uniqueID++;
    return id;
}

void GameState::RemoveClient(const int id) {
    if (const auto it = mClients.find(id); it != mClients.end()) {
        const int nameIndex = it->second->mNameIdx;

        mUsedNames[nameIndex] = false;

        mClients.erase(it);
    }
}

void GameState::Print() {
    std::cout << "Total Clients: " << mClients.size() << "\n";

    if (!mHighScoreName.empty()) {
        std::cout << "High score [" << mHighScoreName <<"]: " << mHighScore << "\n";
    }

    for (const auto &[fst, snd] : mClients) {
        std::cout << fst << " [" << snd->mName << "]: " << snd->mScore << "\n";
    }
}

void GameState::UpdateStateWithInput(const InputData *command, const int id, const float deltaTime) {
    mClients[id]->ProcessInput(command);
    mClients[id]->Update(deltaTime);
}

void GameState::UpdateState(const float deltaTime) {

}

RawState GameState::GetRawState(const int id)  {
    RawState state{};
    state.posX = mClients[id]->mPosition.x;
    state.posY = mClients[id]->mPosition.y;
    state.rotation = mClients[id]->mRotation;

    return state;
}