//
// Created by pedro-souza on 28/11/2025.
//
#pragma once
#include "../Server/InputData.h"

class ClientState {
public:
    explicit ClientState(int id);
    void ProcessInput(const InputData *command);
private:
    int mClientID;

    float mPositionX;
    float mPositionY;

friend class GameState;
};
