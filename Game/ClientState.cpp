//
// Created by pedro-souza on 28/11/2025.
//

#include "ClientState.h"

#include <iostream>
#include <ostream>

ClientState::ClientState(const int id)
:mClientID(id)
,mPositionX(0)
,mPositionY(0)
{}

void ClientState::ProcessInput(const InputData *command) {
    if (command->IsKeyActive(KeyValue::MOVE_FORWARD)) {
        mPositionY += 10.0f;
    }

    if (command->IsKeyActive(KeyValue::MOVE_RIGHT)) {
        mPositionX += 10.0f;
    }
}
