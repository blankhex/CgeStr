#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Blocks.h"
#include "TextProc.h"

#define MAX_LINE 512

struct CaseInfo {
    long rune;
    struct {
        long lower;
        long upper;
        long title;
        long fold;
    } simple;
    struct {
        long lower[4];
        long upper[4];
        long title[4];
        long fold[4];
    } full;
    struct CaseInfo* prev;
    struct CaseInfo* next;
};

typedef int (*EntryCb)(long rune, int fill, char** fields, size_t size);

static void entryProcess(FILE* in, EntryCb cb, const char* globStart,
                           const char* globEnd, size_t globField,
                           size_t codeField, size_t minFields) {
    long code, startCode, prevCode = -1;
    char line[MAX_LINE];
    char* fields[MAX_FIELDS];
    int emitted = 0;
    size_t columns;

    while ((columns = processLine(in, line, sizeof(line), fields))) {
        if (columns < minFields)
            continue;

        code = strtol(fields[codeField], NULL, 16);
        while (prevCode + 1 < code)
            emitted = cb(++prevCode, 1, fields, columns);

        if (globStart && globEnd && columns >= globField &&
            glob(globStart, fields[globField])) {
            startCode = code;
            while ((columns = processLine(in, line, sizeof(line), fields))) {
                if (columns < minFields)
                    continue;
                break;
            }
            if (!glob(globEnd, fields[globField])) {
                fprintf(stderr, "Abnormal input - can find last element\n");
                abort();
            }
            code = strtol(fields[codeField], NULL, 16);
            while (startCode <= code) {
                emitted = cb(startCode, 0, fields, columns);
                startCode++;
            }
        } else {
            emitted = cb(code, 0, fields, columns);
        }
        prevCode = code;
    }

    while (prevCode + 1 < 0x110000)
        emitted = cb(++prevCode, 1, fields, columns);
    while (emitted == -1)
        emitted = cb(code++, 1, fields, columns);
}

static struct CaseInfo* caseInfoSort(struct CaseInfo* head) {
    struct CaseInfo* current;
    struct CaseInfo* next;
    int swapped;

    if (!head || !head->next)
        return head;

    do {
        swapped = 0;
        current = head;
        while (current->next) {
            next = current->next;
            if (current->rune > next->rune) {
                swapped = 1;

                if (current->prev)
                    current->prev->next = next;
                if (next->next)
                    next->next->prev = current;

                current->next = next->next;
                next->prev = current->prev;
                current->prev = next;
                next->next = current;

                if (current == head)
                    head = next;
            } else
                current = current->next;
        }
    } while (swapped);

    return head;
}

static struct CaseInfo* caseInfoGet(struct CaseInfo** head, long rune) {
    struct CaseInfo* current = *head;
    struct CaseInfo* node;

    while (current != NULL) {
        if (current->rune == rune)
            return current;
        current = current->next;
    }

    if (!(node = malloc(sizeof(*node))))
        return NULL;

    memset(node, 0, sizeof(*node));
    node->rune = rune;
    node->next = *head;
    node->prev = NULL;
    if (*head)
        (*head)->prev = node;
    *head = node;
    return node;
}

static size_t categoryClassify(const char* name) {
    static const char *categories[] = {
        "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Mc", "Me", "Nd", "Nl", "No", "Pc",
        "Pd", "Ps", "Pe", "Pi", "Pf", "Po", "Sm", "Sc", "Sk", "So", "Zs", "Zl",
        "Zp", "Cc", "Cf", "Cs", "Co", "Cn", NULL,
    };
    size_t index = 0;

    while (categories[index]) {
        if (!strncmp(categories[index], name, 2))
            return index;
        ++index;
    }

    return categoryClassify("Cn");
}

FILE* in;
FILE* out;

struct Blocks categoryBlocks;
struct CaseInfo* caseInfo = NULL;

static int entryUnicodeData(long rune, int fill, char** fields, size_t size) {
    long lowercase, uppercase, titlecase;
    struct CaseInfo* node;

    if (fill) {
        return blockInsert(&categoryBlocks, categoryClassify("Cn"), 0);
    } else {
        lowercase = strtol(fields[13], NULL, 16);
        uppercase = strtol(fields[12], NULL, 16);
        titlecase = strtol(fields[14], NULL, 16);

        if (lowercase || uppercase || titlecase) {
            node = caseInfoGet(&caseInfo, rune);
            node->simple.lower = lowercase;
            node->simple.upper = uppercase;
            node->simple.title = titlecase;
        }

        return blockInsert(&categoryBlocks, categoryClassify(fields[2]), 0);
    }
}

static void arrayParseFromStr(const char* field, long* array) {
    char* endptr = (char*)field;
    size_t written = 0;

    while (1) {
        array[written] = strtol(endptr, &endptr, 16);
        if (!array[written])
            break;
        written++;
    }
}

static int entryCaseFolding(long rune, int fill, char** fields, size_t size) {
    struct CaseInfo* node;

    if (fill || !strcmp("T", fields[1]))
        return 1;

    node = caseInfoGet(&caseInfo, rune);
    if (strcmp("F", fields[1])) {
        node->simple.fold = strtol(fields[2], NULL, 16);
    } else {
        arrayParseFromStr(fields[2], node->full.fold);
    }
    return 1;
}

static int entrySpecialCasing(long rune, int fill, char** fields, size_t size) {
    struct CaseInfo* node;

    if (fill || strcmp("", fields[4]))
        return 1;

    node = caseInfoGet(&caseInfo, rune);
    arrayParseFromStr(fields[1], node->full.lower);
    arrayParseFromStr(fields[3], node->full.upper);
    arrayParseFromStr(fields[2], node->full.title);

    return 1;
}

static void mappingRemoveSingle(long* array) {
    if (array[0] && !array[1])
        array[0] = 0;
}

static void caseInfoReduce(void) {
    struct CaseInfo* current = caseInfo;
    while (current) {
        if (!current->simple.title && current->simple.upper)
            current->simple.title = current->simple.upper;
        if (!current->full.title[0] && current->full.upper[0])
            memcpy(current->full.title, current->full.upper, 4 * sizeof(long));

        mappingRemoveSingle(current->full.lower);
        mappingRemoveSingle(current->full.upper);
        mappingRemoveSingle(current->full.title);
        mappingRemoveSingle(current->full.fold);
        current = current->next;
    }
}

struct Blocks lowerBlocks, upperBlocks, titleBlocks, foldBlocks;
struct Blocks lowerFullBlocks, upperFullBlocks, titleFullBlocks, foldFullBlocks;

long longIndexData[1024][4];
size_t longIndexSize = 0;

static long longIndexGet(long* array) {
    size_t i;

    for (i = 0; i < longIndexSize; i++) {
        if (!memcmp(array, longIndexData[i], 4 * sizeof(long)))
            return i;
    }

    memcpy(longIndexData[longIndexSize], array, 4 * sizeof(long));
    return longIndexSize++;
}

static void blocksBuild(void) {
    struct CaseInfo* current = caseInfo;
    int emitted;
    long last = -1;

    blockInit(&lowerBlocks, 4, 1, 64, 16, 1);
    blockInit(&upperBlocks, 4, 1, 64, 16, 1);
    blockInit(&titleBlocks, 4, 1, 64, 16, 1);
    blockInit(&foldBlocks, 4, 1, 64, 16, 1);
    blockInit(&lowerFullBlocks, 4, 1, 64, 32, 1);
    blockInit(&upperFullBlocks, 4, 1, 64, 32, 1);
    blockInit(&titleFullBlocks, 4, 1, 64, 32, 1);
    blockInit(&foldFullBlocks, 4, 1, 64, 32, 1);

    while (current) {
        while (last + 1 < current->rune) {
            blockInsert(&lowerBlocks, 0, 0);
            blockInsert(&upperBlocks, 0, 0);
            blockInsert(&titleBlocks, 0, 0);
            blockInsert(&foldBlocks, 0, 0);
            blockInsert(&lowerFullBlocks, -1, 0);
            blockInsert(&upperFullBlocks, -1, 0);
            blockInsert(&titleFullBlocks, -1, 0);
            blockInsert(&foldFullBlocks, -1, 0);
            last++;
        }

        if (current->simple.lower)
            blockInsert(&lowerBlocks, current->simple.lower - current->rune, 0);
        else
            blockInsert(&lowerBlocks, 0, 0);

        if (current->simple.upper)
            blockInsert(&upperBlocks, current->simple.upper - current->rune, 0);
        else
            blockInsert(&upperBlocks, 0, 0);

        if (current->simple.title)
            blockInsert(&titleBlocks, current->simple.title - current->rune, 0);
        else
            blockInsert(&titleBlocks, 0, 0);

        if (current->simple.fold)
            blockInsert(&foldBlocks, current->simple.fold - current->rune, 0);
        else
            blockInsert(&foldBlocks, 0, 0);

        if (current->full.lower[0])
            blockInsert(&lowerFullBlocks, longIndexGet(current->full.lower), 0);
        else
            blockInsert(&lowerFullBlocks, -1, 0);

        if (current->full.upper[0])
            blockInsert(&upperFullBlocks, longIndexGet(current->full.upper), 0);
        else
            blockInsert(&upperFullBlocks, -1, 0);

        if (current->full.title[0])
            blockInsert(&titleFullBlocks, longIndexGet(current->full.title), 0);
        else
            blockInsert(&titleFullBlocks, -1, 0);

        if (current->full.fold[0])
            emitted = blockInsert(&foldFullBlocks, longIndexGet(current->full.fold), 0);
        else
            emitted = blockInsert(&foldFullBlocks, -1, 0);

        last = current->rune;
        current = current->next;
    }

    while (last + 1 < 0x110000 || emitted == -1) {
        blockInsert(&lowerBlocks, 0, 0);
        blockInsert(&upperBlocks, 0, 0);
        blockInsert(&titleBlocks, 0, 0);
        blockInsert(&foldBlocks, 0, 0);
        blockInsert(&lowerFullBlocks, -1, 0);
        blockInsert(&upperFullBlocks, -1, 0);
        blockInsert(&titleFullBlocks, -1, 0);
        emitted = blockInsert(&foldFullBlocks, -1, 0);
        last++;
    }
}

static void outputCode(void) {
#define DUMP(NAME, BLOCK, TYPE1, TYPE2, TYPE3, TYPE4) \
    blockDump(&BLOCK, 0, out, NAME "1", TYPE1); \
    blockDump(&BLOCK, 1, out, NAME "2", TYPE2); \
    blockDump(&BLOCK, 2, out, NAME "3", TYPE3); \
    blockDump(&BLOCK, 3, out, NAME "4", TYPE4)

    fprintf(out, "/* Auto-generated case mapping tables */\n\n");
    fprintf(out, "#include <stdint.h>\n\n");

    DUMP("cat", categoryBlocks, "uint8_t", "uint16_t", "uint16_t", "uint8_t");
    DUMP("low", lowerBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("upp", upperBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("tit", titleBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("fod", foldBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("lfx", lowerFullBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("ufx", upperFullBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("tfx", titleFullBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");
    DUMP("ffx", foldFullBlocks, "uint8_t", "uint8_t", "uint8_t", "uint32_t");

    fprintf(out, "static const int32_t case_data[][3] = {");
    {
        size_t i;
        for (i = 0; i < longIndexSize; ++i) {
            if (i % 4 == 0)
                fprintf(out, "\n    ");
            fprintf(out, "{%ld, %ld, %ld}, ",
                    longIndexData[i][0], longIndexData[i][1],
                    longIndexData[i][2]);
        }
    }
    fprintf(out, "\n};\n\n");

#define EMIT_SIMPLE(FUNC, BLOCKS, BASE) do { \
    fprintf(out, "uint32_t CgeRune" #FUNC "(uint32_t r){\n"); \
    fprintf(out, "    long t;\n    if(r>1114111ul)return r;\n"); \
    blockAccess(&BLOCKS, 0, out, "t", "r", BASE "1"); \
    blockAccess(&BLOCKS, 1, out, "t", "r", BASE "2"); \
    blockAccess(&BLOCKS, 2, out, "t", "r", BASE "3"); \
    blockAccess(&BLOCKS, 3, out, "t", "r", BASE "4"); \
    fprintf(out, "    return t?t+r:r;\n}\n\n"); \
} while(0)

#define EMIT_FULL(FUNC, SIMPLE, FULL_BLOCKS, FULL_BASE, SIMPLE_FUNC) do { \
    fprintf(out, "size_t CgeRune" #FUNC "Full(uint32_t r, uint32_t* out){\n"); \
    fprintf(out, "    long t;\n    if(r>1114111ul){\n        *out=r;\n        return 1;\n    }\n"); \
    blockAccess(&FULL_BLOCKS, 0, out, "t", "r", FULL_BASE "1"); \
    blockAccess(&FULL_BLOCKS, 1, out, "t", "r", FULL_BASE "2"); \
    blockAccess(&FULL_BLOCKS, 2, out, "t", "r", FULL_BASE "3"); \
    blockAccess(&FULL_BLOCKS, 3, out, "t", "r", FULL_BASE "4"); \
    fprintf(out, "    if(t>=0){\n"); \
    fprintf(out, "        const int32_t* p=case_data[t];\n"); \
    fprintf(out, "        size_t i=0;\n"); \
    fprintf(out, "        while(p[i] && i<3){out[i]=p[i];i++;}\n"); \
    fprintf(out, "        return i;\n    }\n"); \
    fprintf(out, "    *out=CgeRune" #SIMPLE "(r);\n    return 1;\n}\n\n"); \
} while(0)

    fprintf(out, "int CgeRuneCategory(uint32_t r){\n");
    fprintf(out, "    long t;\n    if(r>1114111ul)return %d;\n", (int)categoryClassify("Cn"));
    blockAccess(&categoryBlocks, 0, out, "t", "r", "cat1");
    blockAccess(&categoryBlocks, 1, out, "t", "r", "cat2");
    blockAccess(&categoryBlocks, 2, out, "t", "r", "cat3");
    blockAccess(&categoryBlocks, 3, out, "t", "r", "cat4");
    fprintf(out, "    return t;\n}\n\n"); \

    EMIT_SIMPLE(Lower, lowerBlocks, "low");
    EMIT_SIMPLE(Upper, upperBlocks, "upp");
    EMIT_SIMPLE(Title, titleBlocks, "tit");
    EMIT_SIMPLE(Fold, foldBlocks, "fod");

    EMIT_FULL(Lower, Lower, lowerFullBlocks, "lfx", Lower);
    EMIT_FULL(Upper, Upper, upperFullBlocks, "ufx", Upper);
    EMIT_FULL(Title, Title, titleFullBlocks, "tfx", Title);
    EMIT_FULL(Fold, Fold, foldFullBlocks, "ffx", Fold);
}

#undef DUMP
#undef EMIT_SIMPLE
#undef EMIT_FULL

int main() {
    if (!(in = fopen("UnicodeData.txt", "r"))) {
        fprintf(stderr, "UnicodeData.txt not found. Download it from:\n");
        fprintf(stderr, "https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt\n");
        return -1;
    }
    out = stdout;

    fprintf(stderr, "Processing UnicodeData.txt\n");
    blockInit(&categoryBlocks, 4, 1, 16, 8, 8);
    entryProcess(in, entryUnicodeData, "<*, First>", "<*, Last>", 1, 0, 15);
    fclose(in);

    if (!(in = fopen("CaseFolding.txt", "r"))) {
        fprintf(stderr, "CaseFolding.txt not found. Download it from:\n");
        fprintf(stderr, "https://www.unicode.org/Public/UCD/latest/ucd/CaseFolding.txt\n");
        return -1;
    }
    fprintf(stderr, "Processing CaseFolding.txt\n");
    entryProcess(in, entryCaseFolding, NULL, NULL, 0, 0, 3);
    fclose(in);

    fprintf(stderr, "Processing SpecialCasing.txt\n");
    if (!(in = fopen("SpecialCasing.txt", "r"))) {
        fprintf(stderr, "SpecialCasing.txt not found. Download it from:\n");
        fprintf(stderr, "https://www.unicode.org/Public/UCD/latest/ucd/SpecialCasing.txt\n");
        return -1;
    }
    entryProcess(in, entrySpecialCasing, NULL, NULL, 0, 0, 4);
    fclose(in);

    caseInfo = caseInfoSort(caseInfo);
    caseInfoReduce();
    blocksBuild();
    outputCode();

    return 0;
}
