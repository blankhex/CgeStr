#ifndef CGE_STR_H
#define CGE_STR_H

#include <stdint.h>

typedef struct CgeStr {
    const char* data;
    size_t size;
} CgeStr;

#define CGE_STR_LIT(s) \
    {(s), sizeof(s) - 1}

typedef void (*CgeStrIterCb)(uint32_t rune, void* user);
typedef void (*CgeStrWriteCb)(const char* data, size_t size, void* user);

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
size_t CgeRuneLowerFull(uint32_t rune, uint32_t* out);
size_t CgeRuneUpperFull(uint32_t rune, uint32_t* out);
size_t CgeRuneTitleFull(uint32_t rune, uint32_t* out);
size_t CgeRuneFoldFull(uint32_t rune, uint32_t* out);

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

int CgeUtf8Encode(uint32_t rune, char* data);
int CgeUtf8EncodeLax(uint32_t rune, char* data);
int CgeUtf8Decode(const char* data, size_t size, uint32_t* rune);
int CgeUtf8DecodeLax(const char* data, size_t size, uint32_t* rune);

int CgeUtf16Encode(uint32_t rune, uint16_t* data);
int CgeUtf16EncodeLax(uint32_t rune, uint16_t* data);
int CgeUtf16Decode(const uint16_t* data, size_t size, uint32_t* rune);
int CgeUtf16DecodeLax(const uint16_t* data, size_t size, uint32_t* rune);

void CgeStrIter(CgeStr str, CgeStrIterCb cb, void* user);
void CgeStrToLower(CgeStr str, CgeStrWriteCb cb, void* user);
void CgeStrToUpper(CgeStr str, CgeStrWriteCb cb, void* user);
void CgeStrFold(CgeStr str, CgeStrWriteCb cb, void* user);
int CgeStrCmp(CgeStr lhs, CgeStr rhs);
int CgeStrICmp(CgeStr lhs, CgeStr rhs);
size_t CgeStrIndexRune(CgeStr str, uint32_t rune);
size_t CgeStrLastIndexRune(CgeStr str, uint32_t rune);
size_t CgeStrIndexStr(CgeStr str, CgeStr substr);
size_t CgeStrLastIndexStr(CgeStr str, CgeStr substr);
int CgeStrHasPrefix(CgeStr str, CgeStr prefix);
int CgeStrHasSuffix(CgeStr str, CgeStr suffix);
CgeStr CgeStrTrimLeft(CgeStr str);
CgeStr CgeStrTrimRight(CgeStr str);
CgeStr CgeStrTrim(CgeStr str);
CgeStr CgeStrSplit(CgeStr *s, uint32_t delim);

#endif /* CGE_STR_H */
