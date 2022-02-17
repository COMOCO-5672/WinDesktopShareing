#include "BufferReader.h"
#include "Socket.h"

using namespace xop;
uint32_t xop::ReadUint32BE(char *data)
{
    uint8_t *p = (uint8_t *)data;
    uint32_t value = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    return value;
}

uint32_t xop::ReadUint32LE(char *data)
{
    uint8_t *p = (uint8_t *)data;
    uint32_t value = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
    return value;
}

uint32_t xop::ReadUint24BE(char *data)
{
    uint8_t *p = (uint8_t *)data;
    uint32_t value = (p[0] << 16) | (p[1] << 8) | p[2];
    return value;
}

uint32_t xop::ReadUint24LE(char* data)
{
    uint8_t* p = (uint8_t*)data;
    uint32_t value = (p[2] << 16) | (p[1] << 8) | p[0];
    return value;
}

uint16_t xop::ReadUint16BE(char* data)
{
    uint8_t* p = (uint8_t*)data;
    uint16_t value = (p[0] << 8) | p[1];
    return value;
}

uint16_t xop::ReadUint16LE(char* data)
{
    uint8_t* p = (uint8_t*)data;
    uint16_t value = (p[1] << 8) | p[0];
    return value;
}
