#ifndef CGE_STR_H
#define CGE_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct CgeStr {
    const char *data;
    size_t size;
} CgeStr;

#define CGE_STR_LIT(s) \
    {(s), sizeof(s) - 1}

typedef void (*CgeStrIterCb)(uint32_t rune, void *user);
typedef void (*CgeStrWriteCb)(const char *data, size_t size, void *user);

enum CgeCat {
    CGE_CAT_LU, CGE_CAT_LL, CGE_CAT_LT, CGE_CAT_LM, CGE_CAT_LO, CGE_CAT_MN,
    CGE_CAT_MC, CGE_CAT_ME, CGE_CAT_ND, CGE_CAT_NL, CGE_CAT_NO, CGE_CAT_PC,
    CGE_CAT_PD, CGE_CAT_PS, CGE_CAT_PE, CGE_CAT_PI, CGE_CAT_PF, CGE_CAT_PO,
    CGE_CAT_SM, CGE_CAT_SC, CGE_CAT_SK, CGE_CAT_SO, CGE_CAT_ZS, CGE_CAT_ZL,
    CGE_CAT_ZP, CGE_CAT_CC, CGE_CAT_CF, CGE_CAT_CS, CGE_CAT_CO, CGE_CAT_CN
};

int CgeRuneCategory(uint32_t rune);
uint32_t CgeRuneLower(uint32_t rune);
uint32_t CgeRuneUpper(uint32_t rune);
uint32_t CgeRuneTitle(uint32_t rune);
uint32_t CgeRuneFold(uint32_t rune);
size_t CgeRuneLowerFull(uint32_t rune, uint32_t *out);
size_t CgeRuneUpperFull(uint32_t rune, uint32_t *out);
size_t CgeRuneTitleFull(uint32_t rune, uint32_t *out);
size_t CgeRuneFoldFull(uint32_t rune, uint32_t *out);

int CgeRuneIsControl(uint32_t rune);
int CgeRuneIsDigit(uint32_t rune);
int CgeRuneIsGraphic(uint32_t rune);
int CgeRuneIsLetter(uint32_t rune);
int CgeRuneIsLower(uint32_t rune);
int CgeRuneIsMark(uint32_t rune);
int CgeRuneIsNumber(uint32_t rune);
int CgeRuneIsPrint(uint32_t rune);
int CgeRuneIsPunct(uint32_t rune);
int CgeRuneIsSpace(uint32_t rune);
int CgeRuneIsSymbol(uint32_t rune);
int CgeRuneIsTitle(uint32_t rune);
int CgeRuneIsUpper(uint32_t rune);

int CgeUtf8Encode(uint32_t rune, char *data);
int CgeUtf8EncodeLax(uint32_t rune, char *data);
int CgeUtf8Decode(const char *data, size_t size, uint32_t *rune);
int CgeUtf8DecodeLax(const char *data, size_t size, uint32_t *rune);

int CgeUtf16Encode(uint32_t rune, uint16_t *data);
int CgeUtf16EncodeLax(uint32_t rune, uint16_t *data);
int CgeUtf16Decode(const uint16_t *data, size_t size, uint32_t *rune);
int CgeUtf16DecodeLax(const uint16_t *data, size_t size, uint32_t *rune);

void CgeStrIter(const CgeStr *str, CgeStrIterCb cb, void *user);
void CgeStrToLower(const CgeStr *str, CgeStrWriteCb cb, void *user);
void CgeStrToUpper(const CgeStr *str, CgeStrWriteCb cb, void *user);
void CgeStrFold(const CgeStr *str, CgeStrWriteCb cb, void *user);
int CgeStrCmp(const CgeStr *lhs, const CgeStr *rhs);
int CgeStrICmp(const CgeStr *lhs, const CgeStr *rhs);
size_t CgeStrIndexRune(const CgeStr *str, uint32_t rune);
size_t CgeStrLastIndexRune(const CgeStr *str, uint32_t rune);
size_t CgeStrIndexRuneLax(const CgeStr *str, uint32_t rune);
size_t CgeStrLastIndexRuneLax(const CgeStr *str, uint32_t rune);
size_t CgeStrIndexStr(const CgeStr *str, const CgeStr *substr);
size_t CgeStrLastIndexStr(const CgeStr *str, const CgeStr *substr);
int CgeStrHasPrefix(const CgeStr *str, const CgeStr *prefix);
int CgeStrHasSuffix(const CgeStr *str, const CgeStr *suffix);
void CgeStrTrimLeft(const CgeStr *str, CgeStr *out);
void CgeStrTrimRight(const CgeStr *str, CgeStr *out);
void CgeStrTrim(const CgeStr *str, CgeStr *out);
void CgeStrSplit(CgeStr *str, CgeStr *prefix, uint32_t delim);

#ifdef __cplusplus
}
#endif

#endif /* CGE_STR_H */
