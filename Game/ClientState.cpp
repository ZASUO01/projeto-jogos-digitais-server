//
// Created by pedro-souza on 28/11/2025.
//

#include "ClientState.h"
#include <iostream>
#include <ostream>
#include <utility>
#include "../Utils/Math.h"

ClientState::ClientState(class GameState *gm, const int id, const int nameIdx, std::string  name)
    :mGameState(gm)
        , mClientID(id)
      , mNameIdx(nameIdx)
      , mName(std::move(name))
      , mPosition(Vector2::Zero)
      , mVelocity(Vector2::Zero)
      , mColliderRadius(50.0f)
      , mForwardSpeed(500.0f)
      , mRotation(0.0f)
      , mActive(true)
      , mScore(0)
      , mDirection(ClientDirection::right)
      , mShoot(false)
      ,mShootCoolDown(0.0f)
      ,mInvulnerabilityTimer(0.0f)
      ,mMaxLife(3)
      ,mHasShot(false)
{
}

void ClientState::ProcessInput(const InputData *command) {
    const bool up = command->IsKeyActive(KeyValue::MOVE_FORWARD);
    const bool down = command->IsKeyActive(KeyValue::MOVE_BACKWARD);
    const bool left = command->IsKeyActive(KeyValue::MOVE_LEFT);
    const bool right = command->IsKeyActive(KeyValue::MOVE_RIGHT);
    const bool space = command->IsKeyActive(KeyValue::SHOOT);

    Vector2 velocity_direction = Vector2::Zero;

    if (up && !down) {
        velocity_direction.y = -1.0f;
    } else if (down && !up) {
        velocity_direction.y = 1.0f;
    }

    if (left && !right) {
        velocity_direction.x = -1.0f;
    } else if (right && !left) {
        velocity_direction.x = 1.0f;
    }

    if (velocity_direction.x != 0.0f || velocity_direction.y != 0.0f) {
        const float length = Math::Sqrt(
            velocity_direction.x * velocity_direction.x +
            velocity_direction.y * velocity_direction.y
        );

        if (length != 0.0f) {
            const float invLength = 1.0f / length;
            velocity_direction.x *= invLength;
            velocity_direction.y *= invLength;
        }

        mVelocity.x = velocity_direction.x * mForwardSpeed;
        mVelocity.y = velocity_direction.y * mForwardSpeed;

        mRotation = (Math::Atan2(mVelocity.y, mVelocity.x));

        if (up && !down) {
            if (left && !right) {
                mDirection = ClientDirection::upleft;
            } else if (right && !left) {
                mDirection = ClientDirection::upright;
            } else {
                mDirection = ClientDirection::up;
            }
        } else if (down && !up) {
            if (left && !right) {
                mDirection = ClientDirection::downleft;
            } else if (right && !left) {
                mDirection = ClientDirection::downright;
            } else {
                mDirection = ClientDirection::down;
            }
        } else if (left && !right) {
            mDirection = ClientDirection::left;
        } else if (right && !left) {
            mDirection = ClientDirection::right;
        }
    } else {
        mVelocity = Vector2::Zero;
    }

    mShoot = space;
}

void ClientState::Update(const float deltaTime) {
    if (mVelocity.x > MAX_VELOCITY) {
        mVelocity.x = MAX_VELOCITY;
    }else if (mVelocity.x < -MAX_VELOCITY) {
        mVelocity.x = -MAX_VELOCITY;
    }else {
        if (Math::Abs(mVelocity.x) < 0.1f) {
            mVelocity.x = 0.0f;
        }
    }
    mPosition.x += mVelocity.x * deltaTime;

    if (mVelocity.y > MAX_VELOCITY) {
        mVelocity.y = MAX_VELOCITY;
    }else if (mVelocity.y < -MAX_VELOCITY) {
        mVelocity.y = -MAX_VELOCITY;
    }else {
        if (Math::Abs(mVelocity.y) < 0.1f) {
            mVelocity.y = 0.0f;
        }
    }
    mPosition.y += mVelocity.y * deltaTime;
    ScreenWrap(mPosition);
}

void ClientState::UpdateWithoutInput(const float deltaTime) {
    if (mInvulnerabilityTimer > 0.0f) {
        mInvulnerabilityTimer -= deltaTime;
        if (mInvulnerabilityTimer < 0.0f) {
            mInvulnerabilityTimer = 0.0f;
        }
    }

    if (mShootCoolDown > 0.0f) {
        mShootCoolDown -= deltaTime;
    }

    if (mShoot && mShootCoolDown <= 0.0f) {
        mHasShot = true;
        mShootCoolDown = mShootCoolDownTime;
        mShoot = false;

        auto clients = mGameState->GetClientStates();
        for (auto&[id, snd] : clients) {
            auto& client = snd;
            if (const auto center = Vector2(client.mPosition.x, client.mPosition.y); id != mClientID && ShootIntersectOther(center, COLLIDER_RADIUS)) {
                mGameState->RemoveClient(id);
            }
        }
    }
}


void ClientState::ScreenWrap(Vector2 &position){
    if (position.x > GameState::WINDOW_WIDTH) {
        position.x = 0;
    }else if (position.x < 0) {
        position.x = GameState::WINDOW_WIDTH;
    }

    if (position.y > GameState::WINDOW_HEIGHT) {
        position.y = 0;
    }else if (position.y < 0) {
        position.y = GameState::WINDOW_HEIGHT;
    }
}

float ClientState::DirectionToRadian(const ClientDirection direction) {
    constexpr float PiOver4 = Math::PiOver2 / 2.0f;

    switch (direction) {
        case ClientDirection::right:
            return 0.0f;
        case ClientDirection::upright:
            return -PiOver4;
        case ClientDirection::up:
            return -(Math::PiOver2);
        case ClientDirection::upleft:
            return -(3.0f * PiOver4);
        case ClientDirection::left:
            return Math::Pi;
        case ClientDirection::downleft:
            return 3.0f * PiOver4;
        case ClientDirection::down:
            return Math::Pi / 2.0f;
        case ClientDirection::downright:
            return PiOver4;
        default:
            return 0.0f;
    }
}

bool ClientState::ShootIntersectOther(const Vector2 &circleCenter, float radius) const {
    const float characterRotation = DirectionToRadian(mDirection);

    Vector2 direction;
    direction.x = Math::Cos(characterRotation);
    direction.y = Math::Sin(characterRotation);

    Vector2 PC;
    PC.x = circleCenter.x - mPosition.x;
    PC.y = circleCenter.y - mPosition.y;

    float t = PC.x * direction.x + PC.y * direction.y;

    Vector2 perpendicularVector;
    perpendicularVector.x = PC.x - t * direction.x;
    perpendicularVector.y = PC.y - t * direction.y;


    const float distanceSq = perpendicularVector.x * perpendicularVector.x +
                       perpendicularVector.y * perpendicularVector.y;

    if (distanceSq <= radius * radius) {
        return true;
    }

    return false;
}

void ClientState::ResetClient() {
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mRotation = 0.0f;
    mInvulnerabilityTimer = 0.0f;
    mShootCoolDown = 0.0f;
    mHasShot = false;
}
