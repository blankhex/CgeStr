#ifndef TEXTPROC_H
#define TEXTPROC_H

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_FIELDS 16

static int glob(const char *pattern, const char *text) {
    const char *star = NULL;
    const char *restart = text;

    while (*text) {
        if (*pattern == *text || *pattern == '?')
            pattern++, text++;
        else if (*pattern == '*')
            star = ++pattern, restart = text;
        else if (star)
            pattern = star, text = ++restart;
        else
            return 0;
    }
    while (*pattern == '*')
        pattern++;
    return (*pattern == '\0');
}

static char *trimLeft(char *line) {
    for (; *line && isspace(*line); ++line);
    return line;
}

static void trimRight(char *line) {
    char *last = line;

    for (; *line; ++line)
        if (!isspace(*line))
            last = line + 1;
    *last = '\0';
}

static void trimComment(char *line) {
    char *separator = strchr(line, '#');
    if (separator) *separator = '\0';
}

static char *trim(char *line) {
    trimRight(line);
    return trimLeft(line);
}

static size_t fieldParse(char *line, char** fields, char separator) {
    size_t index = 0;

    do {
        fields[index] = line;
        if ((line = strchr(line, separator)))
            *(line++) = '\0';
        fields[index] = trim(fields[index]);
        index++;
    } while (line && index < MAX_FIELDS);

    return index;
}

static int processLine(FILE *in, char *line, size_t size, char** fields) {
    if (!fgets(line, size, in))
        return 0;

    trimComment(line);
    return fieldParse(line, fields, ';');
}

#endif /* TEXTPROC_H */
