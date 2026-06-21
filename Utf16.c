#include "CgeStr.h"

#define INVALID_RUNE    0xFFFD

int CgeUtf16Encode(uint32_t rune, uint16_t *data) {
    if (rune <= 0xFFFF) {
        if (rune >= 0xD800 && rune <= 0xDFFF)
            return -1;
        data[0] = (uint16_t)rune;
        return 1;
    }
    if (rune <= 0x10FFFF) {
        uint32_t x = rune - 0x10000;
        data[0] = (uint16_t)(0xD800 + (x >> 10));
        data[1] = (uint16_t)(0xDC00 + (x & 0x3FF));
        return 2;
    }
    return -1;
}

int CgeUtf16EncodeLax(uint32_t rune, uint16_t *data) {
    int result;

    result = CgeUtf16Encode(rune, data);
    if (result == -1)
        result = CgeUtf16Encode(INVALID_RUNE, data);

    return result;
}

int CgeUtf16Decode(const uint16_t *data, size_t size, uint32_t *rune) {
    uint16_t trail, lead = data[0];

    if (size == 0)
        return -1;

    if (lead < 0xD800 || lead > 0xDFFF) {
        *rune = lead;
        return 1;
    } else if (lead >= 0xD800 && lead <= 0xDBFF) {
        if (size < 2)
            return -1;

        trail = data[1];
        if (trail >= 0xDC00 && trail <= 0xDFFF) {
            *rune = 0x10000 + ((lead & 0x3FF) << 10) + (trail & 0x3FF);
            return 2;
        }
    }

    return -1;
}

int CgeUtf16DecodeLax(const uint16_t *data, size_t size, uint32_t *rune) {
    int result;

    result = CgeUtf16Decode(data, size, rune);
    if (result == -1) {
        *rune = INVALID_RUNE;
        result = 1;
    }

    return result;
}
