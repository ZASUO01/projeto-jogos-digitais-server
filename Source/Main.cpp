
//
// Created by pedro-souza on 24/11/2025.
//
#include "../Server/Server.h"
#include  "../Network/Platforms.h"
#include "../Utils/Random.h"
int main() {
    networkingInit();

    Random::Init();

    Server s;
    if (s.Initialize()) {
        s.ReadInputs();
    }
    s.Shutdown();

    networkingCleanup();
    return 0;
}