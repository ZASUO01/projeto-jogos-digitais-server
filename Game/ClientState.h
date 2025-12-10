//
// Created by pedro-souza on 28/11/2025.
//
#pragma once
#include <iostream>
#include <ostream>

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
    void UpdateWithoutInput(float deltaTime);

    void CleanHasShot(){ mHasShot = false; }
    [[nodiscard]] bool HasShot() const{ return mHasShot; }

    [[nodiscard]] int GetLife() const { return mLife; }
    void ApplyDamage() {
        if (mInvulnerabilityTimer <= 0.0f) {
            mLife--;
            mInvulnerabilityTimer = VULNERABILITY_COOLDOWN;
        }
    }

    [[nodiscard]] bool GetInvulnerability() const { return mInvulnerabilityTimer > 0.0f; }
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

    // Score
    int mScore;

    // direction
    ClientDirection mDirection;
    bool mShoot;
    float mShootCoolDown;
    float mInvulnerabilityTimer;
    int mLife;
    const float mShootCoolDownTime = 0.2f;
    const float VULNERABILITY_COOLDOWN = 2.0f;
    const float COLLIDER_RADIUS = 50.0f;
    bool mHasShot;


    static float DirectionToRadian(ClientDirection direction);
    [[nodiscard]] bool ShootIntersectOther( const Vector2& circleCenter, float radius) const;
    static bool IsDistanceLessThanThreshold(const Vector2 &c1, const Vector2 &c2,  float threshold);
friend class GameState;
};
