#include "CgeStr.h"

#define INVALID_RUNE    0xFFFD

int CgeUtf8Encode(uint32_t rune, char *data) {
    if (rune < 0x80) {
        data[0] = (char)rune;
        return 1;
    }
    if (rune < 0x800) {
        data[0] = (char)(0xC0 | (rune >> 6));
        data[1] = (char)(0x80 | (rune & 0x3F));
        return 2;
    }
    if (rune < 0x10000) {
        if (rune >= 0xD800 && rune <= 0xDFFF)
            return -1;
        data[0] = (char)(0xE0 | (rune >> 12));
        data[1] = (char)(0x80 | ((rune >> 6) & 0x3F));
        data[2] = (char)(0x80 | (rune & 0x3F));
        return 3;
    }
    if (rune <= 0x10FFFF) {
        data[0] = (char)(0xF0 | (rune >> 18));
        data[1] = (char)(0x80 | ((rune >> 12) & 0x3F));
        data[2] = (char)(0x80 | ((rune >> 6) & 0x3F));
        data[3] = (char)(0x80 | (rune & 0x3F));
        return 4;
    }
    return -1;
}

int CgeUtf8EncodeLax(uint32_t rune, char *data) {
    int result;

    result = CgeUtf8Encode(rune, data);
    if (result == -1)
        result = CgeUtf8Encode(INVALID_RUNE, data);

    return result;
}

int CgeUtf8Decode(const char *data, size_t size, uint32_t *rune) {
    unsigned char byte = (unsigned char)data[0];
    int i, n;

    if (size == 0)
        return -1;

    if (byte < 0x80) {
        *rune = byte;
        return 1;
    }

    if ((byte & 0xE0) == 0xC0) {
        n = 2;
        *rune = byte & 0x1F;
    } else if ((byte & 0xF0) == 0xE0) {
        n = 3;
        *rune = byte & 0x0F;
    } else if ((byte & 0xF8) == 0xF0) {
        n = 4;
        *rune = byte & 0x07;
    } else {
        return -1;
    }

    if (size < (size_t)n)
        return -1;

    for (i = 1; i < n; i++) {
        byte = (unsigned char)data[i];
        if ((byte & 0xC0) != 0x80)
            return -1;
        *rune = (*rune << 6) | (byte & 0x3F);
    }

    if ((n == 2 && *rune < 0x80) ||
        (n == 3 && *rune < 0x800) ||
        (n == 4 && *rune < 0x10000)) {
        return -1;
    }

    if (*rune > 0x10FFFF || (*rune >= 0xD800 && *rune <= 0xDFFF)) {
        return -1;
    }

    return n;
}

int CgeUtf8DecodeLax(const char *data, size_t size, uint32_t *rune) {
    int result;

    result = CgeUtf8Decode(data, size, rune);
    if (result == -1) {
        *rune = INVALID_RUNE;
        result = 1;
    }

    return result;
}
