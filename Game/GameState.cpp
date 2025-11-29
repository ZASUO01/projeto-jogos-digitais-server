//
// Created by pedro-souza on 28/11/2025.
//

#include "GameState.h"
#include <algorithm>

GameState::GameState() {}

void GameState::AddClient(const int id) {
    if (mClients.size() == MAX_CLIENTS) {
        return;
    }

    mClients.emplace(id, std::make_unique<ClientState>(id));
}

void GameState::RemoveClient(const int id) {
    mClients.erase(id);
}

void GameState::Update(const InputData *command, const int id) {
    mClients[id]->ProcessInput(command);
}

RawState GameState::GetRawState(const int id)  {
    RawState state{};
    state.posX = mClients[id]->mPositionX;
    state.posY = mClients[id]->mPositionY;

    return state;
}