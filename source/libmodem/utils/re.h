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

#endif /* __RE_H */
