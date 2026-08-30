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

struct StatusByte {
    uint8_t error_code;
    uint8_t mode;
    uint8_t error;
};
// Note: StatusByte packs to 1 bytes on the wire; in-memory struct size may differ.

inline void pack(const StatusByte& value, uint8_t* buffer) {
    buffer[0] = 0;
    buffer[0] = (buffer[0] & ~(7 << 5)) | ((value.error_code & 7) << 5);
    buffer[0] = (buffer[0] & ~(3 << 3)) | ((value.mode & 3) << 3);
    buffer[0] = (buffer[0] & ~(1 << 2)) | ((value.error & 1) << 2);
}

inline bool unpack(const uint8_t* buffer, StatusByte& value) {
    value.error_code = (buffer[0] >> 5) & 7;
    value.mode = (buffer[0] >> 3) & 3;
    value.error = (buffer[0] >> 2) & 1;
    return true;
}

