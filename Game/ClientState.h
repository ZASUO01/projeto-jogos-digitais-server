//
// Created by pedro-souza on 28/11/2025.
//
#pragma once
#include "../Server/InputData.h"
#include <string>

#include "../Utils/Math.h"

class ClientState {
public:
    explicit ClientState(int id, int nameIdx, std::string  name);
    void ProcessInput(const InputData *command);
    void Update(float deltaTime);
private:
    static void ScreenWrap(Vector2 &position);

    // General
    int mClientID;
    int mNameIdx;
    std::string mName;

    // World pos
    Vector2 mPosition;
    Vector2 mVelocity;
    float mForwardSpeed;
    float mRotation;
    static constexpr int MAX_VELOCITY = 700;

    // Lifecycle
    bool mActive;
    int mCurrentLife;

    // Score
    int mScore;
friend class GameState;
};
