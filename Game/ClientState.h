//
// Created by pedro-souza on 28/11/2025.
//
#pragma once
#include "../Server/InputData.h"
#include <string>
#include "../Utils/Math.h"
#include "GameState.h"

enum class ClientDirection {
    up,
    upleft,
    upright,
    down,
    downleft,
    downright,
    right,
    left,
};

class GameState;

class ClientState {
public:
    explicit ClientState(GameState *gm, int id, int nameIdx, std::string  name);
    void ProcessInput(const InputData *command);
    void Update(float deltaTime);
private:
    static void ScreenWrap(Vector2 &position);

    // game state
    GameState *mGameState;

    // General
    int mClientID;
    int mNameIdx;
    std::string mName;

    // World pos
    Vector2 mPosition;
    Vector2 mVelocity;
    float mColliderRadius;
    float mForwardSpeed;
    float mRotation;
    static constexpr int MAX_VELOCITY = 700;

    // Lifecycle
    bool mActive;
    int mCurrentLife;

    // Score
    int mScore;

    // direction
    ClientDirection mDirection;
    bool mShoot;

    static float DirectionToRadian(ClientDirection direction);
    bool ShootIntersectOther( const Vector2& circleCenter, float radius) const;
friend class GameState;
};
