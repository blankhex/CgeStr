#include "CgeStr.h"

int CgeRuneIsControl(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_CC:
            return 1;
    }
    return 0;
}

int CgeRuneIsDigit(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_ND:
            return 1;
    }
    return 0;
}

int CgeRuneIsGraphic(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LL:
        case CGE_CAT_LM:
        case CGE_CAT_LO:
        case CGE_CAT_LT:
        case CGE_CAT_LU:
        case CGE_CAT_MC:
        case CGE_CAT_ME:
        case CGE_CAT_MN:
        case CGE_CAT_ND:
        case CGE_CAT_NL:
        case CGE_CAT_NO:
        case CGE_CAT_PC:
        case CGE_CAT_PD:
        case CGE_CAT_PE:
        case CGE_CAT_PF:
        case CGE_CAT_PI:
        case CGE_CAT_PO:
        case CGE_CAT_PS:
        case CGE_CAT_SC:
        case CGE_CAT_SK:
        case CGE_CAT_SM:
        case CGE_CAT_SO:
        case CGE_CAT_ZS:
            return 1;
    }
    return 0;
}

int CgeRuneIsLetter(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LL:
        case CGE_CAT_LM:
        case CGE_CAT_LO:
        case CGE_CAT_LT:
        case CGE_CAT_LU:
            return 1;
    }
    return 0;
}

int CgeRuneIsLower(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LL:
            return 1;
    }
    return 0;
}

int CgeRuneIsMark(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_MC:
        case CGE_CAT_ME:
        case CGE_CAT_MN:
            return 1;
    }
    return 0;
}

int CgeRuneIsNumber(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_ND:
        case CGE_CAT_NL:
        case CGE_CAT_NO:
            return 1;
    }
    return 0;
}

int CgeRuneIsPrint(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LL:
        case CGE_CAT_LM:
        case CGE_CAT_LO:
        case CGE_CAT_LT:
        case CGE_CAT_LU:
        case CGE_CAT_MC:
        case CGE_CAT_ME:
        case CGE_CAT_MN:
        case CGE_CAT_ND:
        case CGE_CAT_NL:
        case CGE_CAT_NO:
        case CGE_CAT_PC:
        case CGE_CAT_PD:
        case CGE_CAT_PE:
        case CGE_CAT_PF:
        case CGE_CAT_PI:
        case CGE_CAT_PO:
        case CGE_CAT_PS:
        case CGE_CAT_SC:
        case CGE_CAT_SK:
        case CGE_CAT_SM:
        case CGE_CAT_SO:
            return 1;
    }
    return rune == ' ';
}

int CgeRuneIsPunct(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_PC:
        case CGE_CAT_PD:
        case CGE_CAT_PE:
        case CGE_CAT_PF:
        case CGE_CAT_PI:
        case CGE_CAT_PO:
        case CGE_CAT_PS:
            return 1;
    }
    return 0;
}

int CgeRuneIsSpace(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_ZL:
        case CGE_CAT_ZP:
        case CGE_CAT_ZS:
            return 1;
    }

    switch (rune) {
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            return 1;
    }
    return 0;
}

int CgeRuneIsSymbol(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_SC:
        case CGE_CAT_SK:
        case CGE_CAT_SM:
        case CGE_CAT_SO:
            return 1;
    }
    return 0;
}

int CgeRuneIsTitle(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LT:
            return 1;
    }
    return 0;
}

int CgeRuneIsUpper(uint32_t rune) {
    switch (CgeRuneCategory(rune)) {
        case CGE_CAT_LU:
            return 1;
    }
    return 0;
}
