//
// Created by pedro-souza on 28/11/2025.
//

#include "ClientState.h"

#include <iostream>
#include <ostream>
#include <utility>

ClientState::ClientState(const int id, const int nameIdx, std::string  name)
:mClientID(id)
,mNameIdx(nameIdx)
,mName(std::move(name))
,mPositionX(0)
,mPositionY(0)
,mActive(true)
,mCurrentLife(3)
,mScore(0)
{}

void ClientState::ProcessInput(const InputData *command) {
    if (command->IsKeyActive(KeyValue::MOVE_FORWARD)) {
        mPositionY -= 10.0f;
    }

    if (command->IsKeyActive(KeyValue::MOVE_RIGHT)) {
        mPositionX += 10.0f;
    }

    if (command->IsKeyActive(KeyValue::MOVE_LEFT)) {
        mPositionX -= 10.0f;
    }

    if (command->IsKeyActive(KeyValue::MOVE_BACKWARD)) {
        mPositionY += 10.0f;
    }
}
