#ifndef __STR_H
#define __STR_H

char* trim_l(char* s);

char* trim_r(char* s);

char* trim_r_esc(char* s, const char* escape_symbols);

char* trim(char *s);

#endif /* __STR_H */
