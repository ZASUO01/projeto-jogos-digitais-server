//
// Created by pedro-souza on 25/11/2025.
//

#include "ServerOperations.h"
#include "../Network/Packet.h"
#include "../Network/Socket.h"

void ServerOperations::sendSingleResponseToClient(
    const Server *server,
    const uint32_t nonce,
    const uint16_t sequence,
    const uint8_t flag,
    sockaddr_in *clientAddr
) {
    if (flag != Packet::SYN_ACK_FLAG && flag != Packet::END_ACK_FLAG) {
        return;
    }

    Packet packet(sequence, flag, nonce);
    packet.BuildPacket();

    constexpr size_t packetSize = Packet::PACKET_HEADER_BYTES;

    SocketUtils::sendPacketToV4(
        server->GetSocket(),
        &packet,
        packetSize,
        clientAddr
    );
}

void ServerOperations::sendStateDataToClient(
    const Server *server,
    const uint32_t nonce,
    const uint16_t sequence,
    sockaddr_in *clientAddr,
    const RawState *state
) {
    Packet packet(sequence, Packet::DATA_FLAG, nonce);
    packet.SetData(state, sizeof(RawState));
    packet.BuildPacket();

    const size_t packetSize = Packet::PACKET_HEADER_BYTES + packet.GetLength();

    SocketUtils::sendPacketToV4(
        server->GetSocket(),
        &packet,
        packetSize,
        clientAddr
    );
}
