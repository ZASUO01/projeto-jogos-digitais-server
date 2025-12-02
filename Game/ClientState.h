//
// Created by pedro-souza on 28/11/2025.
//
#pragma once
#include "../Server/InputData.h"
#include <string>

class ClientState {
public:
    explicit ClientState(int id, int nameIdx, std::string  name);
    void ProcessInput(const InputData *command);
private:
    // General
    int mClientID;
    int mNameIdx;
    std::string mName;

    // World pos
    float mPositionX;
    float mPositionY;

    // Lifecycle
    bool mActive;
    int mCurrentLife;

    // Score
    int mScore;
friend class GameState;
};
