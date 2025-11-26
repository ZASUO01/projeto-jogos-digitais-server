//
// Created by pedro-souza on 25/11/2025.
//

#include "ServerOperations.h"
#include "../Network/Packet.h"
#include "../Network/Socket.h"

void ServerOperations::sendSynAckToClient(
    const Server *server,
    const uint32_t nonce,
    const uint16_t sequence ,
    sockaddr_in * clientAddr
) {
    Packet packet(sequence, Packet::SYN_ACK_FLAG, nonce);
    packet.BuildPacket();

    constexpr size_t packetSize = Packet::PACKET_HEADER_BYTES;

    SocketUtils::sendPacketToV4(
        server->GetSocket(),
        &packet,
        packetSize,
        clientAddr);
}
