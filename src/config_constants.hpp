#pragma once

#include <cstdint>
#include <string_view>

namespace socket_constants {
    constexpr std::string_view      SERVER_ADDR             = "127.0.0.1";
    constexpr uint16_t              SERVER_PORT             = 22222;
    constexpr size_t                SERVER_MAX_INSTANCES    = 10;

    constexpr uint32_t  SERVER_MAX_PACKET_SIZE  = 10 * 1024 * 1024; // 10MB
}