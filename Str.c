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

static int uniStreamPut(struct UniStream *stream, uint32_t rune) {
    if (stream->size >= MAX_UNI_STREAM)
        return 0;

    stream->data[stream->tail] = rune;
    stream->tail = (stream->tail + 1) & (MAX_UNI_STREAM - 1);
    stream->size++;
    return 1;
}

static int uniStreamGet(struct UniStream *stream, uint32_t *rune) {
    if (!stream->size)
        return 0;

    *rune = stream->data[stream->head];
    stream->head = (stream->head + 1) & (MAX_UNI_STREAM - 1);
    stream->size--;
    return 1;
}

void CgeStrIter(const CgeStr *str, CgeStrIterCb cb, void *user) {
    const char *current = str->data;
    const char *end = str->data + str->size;

    while (current < end) {
        uint32_t rune;

        current += CgeUtf8DecodeLax(current, end - current, &rune);
        cb(rune, user);
    }
}

void CgeStrToLower(const CgeStr *str, CgeStrWriteCb cb, void *user) {
    const char *current = str->data;
    const char *end = str->data + str->size;

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

void CgeStrToUpper(const CgeStr *str, CgeStrWriteCb cb, void *user) {
    const char *current = str->data;
    const char *end = str->data + str->size;

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

void CgeStrFold(const CgeStr *str, CgeStrWriteCb cb, void *user) {
    const char *current = str->data;
    const char *end = str->data + str->size;

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

int CgeStrCmp(const CgeStr *lhs, const CgeStr *rhs) {
    size_t leastSize;
    int result;

    leastSize = (lhs->size < rhs->size) ? lhs->size : rhs->size;
    result = memcmp(lhs->data, rhs->data, leastSize);
    if (result < 0)
        return -1;
    else if (result > 0)
        return 1;

    if (lhs->size < rhs->size)
        return -1;
    else if (lhs->size > rhs->size)
        return 1;

    return 0;
}

int CgeStrICmp(const CgeStr *lhs, const CgeStr *rhs) {
    struct UniStream buf1 = {{0}, 0, 0, 0};
    struct UniStream buf2 = {{0}, 0, 0, 0};

    const char *current1 = lhs->data;
    const char *current2 = rhs->data;
    const char *end1 = lhs->data + lhs->size;
    const char *end2 = rhs->data + rhs->size;

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

size_t CgeStrIndexRune(const CgeStr *str, uint32_t rune) {
    char scratch[4];
    CgeStr tmp;

    tmp.data = scratch;
    if ((tmp.size = CgeUtf8Encode(rune, scratch)) == (size_t)-1)
        return (size_t)-1;

    return CgeStrIndexStr(str, &tmp);
}

size_t CgeStrLastIndexRune(const CgeStr *str, uint32_t rune) {
    char scratch[4];
    CgeStr tmp;

    tmp.data = scratch;
    if ((tmp.size = CgeUtf8Encode(rune, scratch)) == (size_t)-1)
        return (size_t)-1;

    return CgeStrLastIndexStr(str, &tmp);
}

size_t CgeStrIndexRuneLax(const CgeStr *str, uint32_t rune) {
    size_t i = 0;
    while (i < str->size) {
        uint32_t r;
        int count;

        count = CgeUtf8DecodeLax(str->data + i, str->size - i, &r);
        if (r == rune)
            return i;
        i += count;
    }
    return (size_t)-1;
}

size_t CgeStrLastIndexRuneLax(const CgeStr *str, uint32_t rune) {
    size_t i = str->size;

    while (i > 0) {
        size_t current = i;
        uint32_t r;

        while (current > 0 && (str->data[current - 1] & 0xC0) == 0x80)
            current--;
        if (!current)
            current = i - 1;

        CgeUtf8DecodeLax(str->data + current, i - current, &r);
        if (r == rune)
            return current;
        i = current;
    }
    return (size_t)-1;
}

size_t CgeStrIndexStr(const CgeStr *str, const CgeStr *substr) {
    size_t i;

    if (!substr->size)
        return 0;
    if (str->size < substr->size)
        return (size_t)-1;

    for (i = 0; i <= str->size - substr->size; i++) {
        if (!memcmp(str->data + i, substr->data, substr->size))
            return i;
    }
    return (size_t)-1;
}

size_t CgeStrLastIndexStr(const CgeStr *str, const CgeStr *substr) {
    size_t i;

    if (!substr->size)
        return str->size;
    if (str->size < substr->size)
        return (size_t)-1;

    for (i = str->size - substr->size; i != (size_t)-1; i--) {
        if (!memcmp(str->data + i, substr->data, substr->size))
            return i;
    }
    return (size_t)-1;
}

int CgeStrHasPrefix(const CgeStr *str, const CgeStr *prefix) {
    if (prefix->size > str->size)
        return 0;

    return !memcmp(str->data, prefix->data, prefix->size);
}

int CgeStrHasSuffix(const CgeStr *str, const CgeStr *suffix) {
    if (suffix->size > str->size)
        return 0;

    return !memcmp(str->data + str->size - suffix->size, suffix->data, suffix->size);
}

void CgeStrTrimLeft(const CgeStr *str, CgeStr *out) {
    out->data = str->data;
    out->size = str->size;

    while (out->size) {
        uint32_t rune;
        int count;

        count = CgeUtf8DecodeLax(out->data, out->size, &rune);
        if (!CgeRuneIsSpace(rune))
            break;

        out->data += count;
        out->size -= count;
    }
}

void CgeStrTrimRight(const CgeStr *str, CgeStr *out) {
    out->data = str->data;
    out->size = str->size;

    while (out->size) {
        size_t pos = out->size;
        uint32_t rune;

        while (pos > 0 && (out->data[pos - 1] & 0xC0) == 0x80)
            pos--;
        if (pos == 0)
            pos = 1;

        CgeUtf8DecodeLax(out->data + pos - 1, out->size - (pos - 1), &rune);
        if (!CgeRuneIsSpace(rune))
            break;

        out->size = pos - 1;
    }
}

void CgeStrTrim(const CgeStr *str, CgeStr *out) {
    CgeStrTrimLeft(str, out);
    CgeStrTrimRight(out, out);
}

void CgeStrSplit(CgeStr *str, CgeStr *prefix, uint32_t delim) {
    uint32_t r;
    size_t pos;
    int count;

    pos = CgeStrIndexRune(str, delim);
    prefix->data = str->data;

    if (pos == (size_t)-1) {
        prefix->size = str->size;
        str->size = 0;
    } else {
        count = CgeUtf8DecodeLax(str->data + pos, str->size - pos, &r);
        prefix->size = pos;
        str->data += pos + count;
        str->size -= pos + count;
    }
}
