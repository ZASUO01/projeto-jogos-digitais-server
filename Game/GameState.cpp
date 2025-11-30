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

void GameState::UpdateStateWithInput(const InputData *command, const int id, const float deltaTime) {
    mClients[id]->ProcessInput(command);
}

void GameState::UpdateState(const float deltaTime) {

}

RawState GameState::GetRawState(const int id)  {
    RawState state{};
    state.posX = mClients[id]->mPositionX;
    state.posY = mClients[id]->mPositionY;

    return state;
}