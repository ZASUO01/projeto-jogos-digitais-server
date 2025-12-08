//
// Created by pedro-souza on 28/11/2025.
//

#include "GameState.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <iostream>

int GameState::uniqueID = 0;

const std::vector<std::string> GameState::CLIENT_NAMES = {
    "CodeLancer",
    "DataPulse",
    "VoltEdge",
    "NetGhost"
};

GameState::GameState():mHighScore(0) {
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
    const std::string& name = CLIENT_NAMES[idx];
    mUsedNames[idx] = true;

    int id = uniqueID;

    mClients.emplace(id, ClientState(this, id, static_cast<int>(idx), name));

    uniqueID++;
    return id;
}

void GameState::RemoveClient(const int id) {
    if (const auto it = mClients.find(id); it != mClients.end()) {
        const int nameIndex = it->second.mNameIdx;

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
        std::cout << fst << " [" << snd.mName << "]: " << snd.mScore << "\n";
    }
}

void GameState::UpdateStateWithInput(const InputData *command, const int id, const float deltaTime) {
    if (const auto it = mClients.find(id); it != mClients.end()) {
        auto& client = it->second;
        client.ProcessInput(command);
        client.Update(deltaTime);
    }
}

void GameState::UpdateState(const float deltaTime) {

}

RawState GameState::GetRawState(const int id)  {
    RawState state{};

    if (const auto it = mClients.find(id); it != mClients.end()) {
        const auto& client = it->second;

        state.posX = client.mPosition.x;
        state.posY = client.mPosition.y;
        state.rotation = client.mRotation;
    }

    return state;
}

std::vector<OtherState> GameState::GetOtherStates(const int id) {
    std::vector<OtherState> otherStates;

    for (const auto &[fst, snd]: mClients) {
        if (fst != id) {
            otherStates.emplace_back(
                snd.mClientID,
                snd.mPosition.x,
                snd.mPosition.y,
                snd.mRotation
            );
        }
    }

    return otherStates;
}