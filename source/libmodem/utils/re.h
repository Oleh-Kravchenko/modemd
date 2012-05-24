#ifndef __RE_H
#define __RE_H

#include <regex.h>

int regmatch_atoi(const char* src, const regmatch_t* re_subs);

char* regmatch_ncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs);

char* regmatch_strdup(const char* src, const regmatch_t* re_subs);

/**
 * @brief compare string with regular expression mask
 * @param s string
 * @param mask regular expression mask
 * @return zero if string is macthed
 */
int regmatch_cmp(const char* s, const char* mask);

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
int regmatch_parse(const char* s, const char* mask, size_t* nmatch, regmatch_t** pmatch);

#endif /* __RE_H */
