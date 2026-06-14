#include "CgeStr.h"
#include <stdint.h>
#include <string.h>

#define INVALID_RUNE    0xFFFD
#define MAX_UNI_STREAM  4

struct UniStream {
    uint32_t data[MAX_UNI_STREAM];
    size_t head;
    size_t tail;
    size_t size;
};

static int uniStreamPut(struct UniStream* stream, uint32_t rune) {
    if (stream->size >= MAX_UNI_STREAM)
        return 0;

    stream->data[stream->tail] = rune;
    stream->tail = (stream->tail + 1) & (MAX_UNI_STREAM - 1);
    stream->size++;
    return 1;
}

static int uniStreamGet(struct UniStream* stream, uint32_t* rune) {
    if (!stream->size)
        return 0;

    *rune = stream->data[stream->head];
    stream->head = (stream->head + 1) & (MAX_UNI_STREAM - 1);
    stream->size--;
    return 1;
}

void CgeStrIter(CgeStr str, CgeStrIterCb cb, void* user) {
    const char* current = str.data;
    const char* end = str.data + str.size;

    while (current < end) {
        uint32_t rune;

        current += CgeUtf8DecodeLax(current, end - current, &rune);
        cb(rune, user);
    }
}

void CgeStrToLower(CgeStr str, CgeStrWriteCb cb, void* user) {
    const char* current = str.data;
    const char* end = str.data + str.size;

    while (current < end) {
        uint32_t rune;
        uint32_t mapped[3];
        int count, i;

        current += CgeUtf8DecodeLax(current, end - current, &rune);
        count = (int)CgeRuneLowerFull(rune, mapped);
        for (i = 0; i < count; i++) {
            char scratch[4];

            cb(scratch, CgeUtf8EncodeLax(mapped[i], scratch), user);
        }
    }
}

void CgeStrToUpper(CgeStr str, CgeStrWriteCb cb, void* user) {
    const char* current = str.data;
    const char* end = str.data + str.size;

    while (current < end) {
        uint32_t rune;
        uint32_t mapped[3];
        int count, i;

        current += CgeUtf8DecodeLax(current, end - current, &rune);
        count = (int)CgeRuneUpperFull(rune, mapped);
        for (i = 0; i < count; i++) {
            char scratch[4];

            cb(scratch, CgeUtf8EncodeLax(mapped[i], scratch), user);
        }
    }
}

void CgeStrFold(CgeStr str, CgeStrWriteCb cb, void* user) {
    const char* current = str.data;
    const char* end = str.data + str.size;

    while (current < end) {
        uint32_t rune;
        uint32_t mapped[3];
        int count, i;

        current += CgeUtf8DecodeLax(current, end - current, &rune);
        count = (int)CgeRuneFoldFull(rune, mapped);
        for (i = 0; i < count; i++) {
            char scratch[4];

            cb(scratch, CgeUtf8EncodeLax(mapped[i], scratch), user);
        }
    }
}

int CgeStrCmp(CgeStr lhs, CgeStr rhs) {
    size_t leastSize;
    int result;

    leastSize = (lhs.size < rhs.size) ? lhs.size : rhs.size;
    result = memcmp(lhs.data, rhs.data, leastSize);
    if (result < 0)
        return -1;
    else if (result > 0)
        return 1;

    if (lhs.size < rhs.size)
        return -1;
    else if (lhs.size > rhs.size)
        return 1;

    return 0;
}

int CgeStrICmp(CgeStr lhs, CgeStr rhs) {
    struct UniStream buf1 = {{0}, 0, 0, 0};
    struct UniStream buf2 = {{0}, 0, 0, 0};

    const char* current1 = lhs.data;
    const char* current2 = rhs.data;
    const char* end1 = lhs.data + lhs.size;
    const char* end2 = rhs.data + rhs.size;

    while (1) {
        uint32_t rune1, rune2;

        if (!buf1.size && current1 < end1) {
            uint32_t scratch, folded[3];
            int i, count;

            current1 += CgeUtf8DecodeLax(current1, end1 - current1, &scratch);
            count = (int)CgeRuneFoldFull(scratch, folded);
            for (i = 0; i < count; i++)
                uniStreamPut(&buf1, folded[i]);
        }

        if (!buf2.size && current2 < end2) {
            uint32_t scratch, folded[3];
            int i, count;

            current2 += CgeUtf8DecodeLax(current2, end2 - current2, &scratch);
            count = (int)CgeRuneFoldFull(scratch, folded);
            for (i = 0; i < count; i++)
                uniStreamPut(&buf2, folded[i]);
        }

        if (!buf1.size && !buf2.size)
            return 0;

        if (!uniStreamGet(&buf1, &rune1))
            return -1;

        if (!uniStreamGet(&buf2, &rune2))
            return 1;

        if (rune1 < rune2)
            return -1;
        if (rune1 > rune2)
            return 1;
    }
}

size_t CgeStrIndexRune(CgeStr str, uint32_t rune) {
    size_t i = 0;
    while (i < str.size) {
        uint32_t r;
        int count;

        count = CgeUtf8DecodeLax(str.data + i, str.size - i, &r);
        if (r == rune)
            return i;
        i += count;
    }
    return (size_t)-1;
}

size_t CgeStrLastIndexRune(CgeStr str, uint32_t rune) {
    size_t i = str.size;

    while (i > 0) {
        size_t current = i;
        uint32_t r;

        while (current > 0 && (str.data[current - 1] & 0xC0) == 0x80)
            current--;
        if (!current)
            current = i - 1;

        CgeUtf8DecodeLax(str.data + current, i - current, &r);
        if (r == rune)
            return current;
        i = current;
    }
    return (size_t)-1;
}

size_t CgeStrIndexStr(CgeStr str, CgeStr substr) {
    size_t i;

    if (!substr.size)
        return 0;
    if (str.size < substr.size)
        return (size_t)-1;

    for (i = 0; i <= str.size - substr.size; i++) {
        if (!memcmp(str.data + i, substr.data, substr.size))
            return i;
    }
    return (size_t)-1;
}

size_t CgeStrLastIndexStr(CgeStr str, CgeStr substr) {
    size_t i;

    if (!substr.size)
        return str.size;
    if (str.size < substr.size)
        return (size_t)-1;

    for (i = str.size - substr.size; i != (size_t)-1; i--) {
        if (!memcmp(str.data + i, substr.data, substr.size))
            return i;
    }
    return (size_t)-1;
}

int CgeStrHasPrefix(CgeStr str, CgeStr prefix) {
    if (prefix.size > str.size)
        return 0;

    return !memcmp(str.data, prefix.data, prefix.size);
}

int CgeStrHasSuffix(CgeStr str, CgeStr suffix) {
    if (suffix.size > str.size)
        return 0;

    return !memcmp(str.data + str.size - suffix.size, suffix.data, suffix.size);
}

CgeStr CgeStrTrimLeft(CgeStr str) {
    while (str.size) {
        uint32_t rune;
        int count;

        count = CgeUtf8DecodeLax(str.data, str.size, &rune);
        if (!CgeRuneIsSpace(rune))
            break;

        str.data += count;
        str.size -= count;
    }
    return str;
}

CgeStr CgeStrTrimRight(CgeStr str) {
     while (str.size) {
        size_t pos = str.size;
        uint32_t rune;

        while (pos > 0 && (str.data[pos - 1] & 0xC0) == 0x80)
            pos--;
        if (pos == 0)
            pos = 1;

        CgeUtf8DecodeLax(str.data + pos - 1, str.size - (pos - 1), &rune);
        if (!CgeRuneIsSpace(rune))
            break;

        str.size = pos - 1;
    }
    return str;
}

CgeStr CgeStrTrim(CgeStr str) {
    return CgeStrTrimRight(CgeStrTrimLeft(str));
}

CgeStr CgeStrSplit(CgeStr *s, uint32_t delim) {
    size_t pos;
    int count;
    uint32_t r;
    CgeStr result;

    pos = CgeStrIndexRune(*s, delim);
    if (pos == (size_t)-1) {
        result = *s;
        s->size = 0;
        return result;
    }

    count = CgeUtf8DecodeLax(s->data + pos, s->size - pos, &r);

    result.data = s->data;
    result.size = pos;
    s->data += pos + count;
    s->size -= pos + count;
    return result;
}
