cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME "line-casters-server")
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
        Source/Main.cpp
        Network/Platforms.h
        Network/Packet.cpp
        Network/Packet.h
        Network/NetUtils.cpp
        Network/NetUtils.h
        Network/Socket.cpp
        Network/Socket.h
        Network/Logger.cpp
        Network/Logger.h
        Network/Addresses.cpp
        Network/Addresses.h
        Network/Defs.h
        Server/Server.cpp
        Server/Server.h
        Server/ServerOperations.cpp
        Server/ServerOperations.h
        Utils/Random.cpp
        Utils/Random.h
        Server/InputData.h
        Game/ClientState.cpp
        Game/ClientState.h
        Game/GameState.cpp
        Game/GameState.h
        Utils/ThreadQueue.h
        Server/DataObjects.h
        Utils/Math.cpp
        Utils/Math.h
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif ()