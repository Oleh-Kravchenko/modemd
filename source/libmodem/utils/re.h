#ifndef __RE_H
#define __RE_H

#include <regex.h>

size_t re_strlen(const regmatch_t* re_subs);

int re_atoi(const char* src, const regmatch_t* re_subs);

char* re_strncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs);

char* re_strdup(const char* src, const regmatch_t* re_subs);

/**
 * @brief compare string with regular expression mask
 * @param s string
 * @param mask regular expression mask
 * @return zero if string is macthed
 */
int re_strcmp(const char* s, const char* mask);

/**
 * @brief parse the string using regular expression
 * @param s string
 * @param mask regular expression
 * @param nmatch numbers of parsed items
 * @param pmatch pointer to array of indexes
 * @return zero if successful
 *
 * pmatch must be freed by function free()
 */
int re_parse(const char* s, const char* mask, size_t* nmatch, regmatch_t** pmatch);

#endif /* __RE_H */
