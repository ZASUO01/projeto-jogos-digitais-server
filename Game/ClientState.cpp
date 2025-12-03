//
// Created by pedro-souza on 28/11/2025.
//

#include "ClientState.h"

#include <iostream>
#include <ostream>
#include <utility>

#include "GameState.h"
#include "../Utils/Math.h"

ClientState::ClientState(const int id, const int nameIdx, std::string  name)
:mClientID(id)
,mNameIdx(nameIdx)
,mName(std::move(name))
,mPosition(Vector2::Zero)
,mVelocity(Vector2::Zero)
,mForwardSpeed(500.0f)
,mRotation(0.0f)
,mActive(true)
,mCurrentLife(3)
,mScore(0)
{}

void ClientState::ProcessInput(const InputData *command) {
    const bool up = command->IsKeyActive(KeyValue::MOVE_FORWARD);
    const bool down = command->IsKeyActive(KeyValue::MOVE_BACKWARD);
    const bool left = command->IsKeyActive(KeyValue::MOVE_LEFT);
    const bool right = command->IsKeyActive(KeyValue::MOVE_RIGHT);

    Vector2 velocity = Vector2::Zero;

    if (up && !down) {
        velocity.y = -mForwardSpeed;
    } else if (down && !up) {
        velocity.y = mForwardSpeed;
    }

    if (left && !right) {
        velocity.x = -mForwardSpeed;
    } else if (right && !left) {
        velocity.x = mForwardSpeed;
    }

    if (velocity.x != 0.0f || velocity.y != 0.0f) {
        mRotation = (Math::Atan2(velocity.y, velocity.x));
        if (velocity.x != 0.0f && velocity.y != 0.0f) {
            const float length = Math::Sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

            velocity.x = (velocity.x / length) * mForwardSpeed;
            velocity.y = (velocity.y / length) * mForwardSpeed;
        }
    }
    mVelocity = velocity;
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