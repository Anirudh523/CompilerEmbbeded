#pragma once
#include <cstdint>
#include <cstring>

inline uint16_t crc16(const uint8_t* data, size_t length) {
 uint16_t crc = 0xFFFF;
 for (size_t i = 0; i < length; i++) {
     crc ^= static_cast<uint16_t>(data[i]) << 8;
     for(int bit = 0; bit < 8; bit++) {
         if(crc & 0x8000) {
             crc = (crc << 1) ^ 0x1021;
         } else {
             crc <<= 1;
         }
     }
 }
 return crc;
}

struct extra_endian {
    uint16_t something_inside;
};
static_assert(sizeof(extra_endian) == 2);

inline void pack(const extra_endian& value, uint8_t* buffer) {
    std::memcpy(buffer + 0, &value.something_inside, 2);
}

inline bool unpack(const uint8_t* buffer, extra_endian& value) {
    std::memcpy(&value.something_inside, buffer + 0, 2);
    return true;
}

