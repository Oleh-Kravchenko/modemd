#include <string.h>

/*------------------------------------------------------------------------*/

static const char escape_symbols[] = "\x20\a\b\t\n\v\f\r";

/*------------------------------------------------------------------------*/

char* trim_l(char* s)
{
	char* res = s;

	while(*s && strchr(escape_symbols, *s))
		++ s;

	if(res != s)
		memmove(res, s, strlen(s) + 1);

	return(res);
}

/*------------------------------------------------------------------------*/

char* trim_r_esc(char* s, const char* esc)
{
	size_t len = strlen(s);
	char *res = NULL;

	while(len -- && strchr(esc, s[len]))
		res = &s[len];

	if(res)
		*res = 0;

	return(res);
}

/*------------------------------------------------------------------------*/

char* trim_r(char* s)
{
	return(trim_r_esc(s, escape_symbols));
}

/*------------------------------------------------------------------------*/

char* trim(char *s)
{
	char* l = s;
	size_t len;

	while(*l && strchr(escape_symbols, *l))
		++ l;

	len = strlen(l);

	while(len)
	{
		-- len;

		if(!strchr(escape_symbols, l[len]))
			break;
	}

	if(len ++)
	{
		memmove(s, l, len);
		s[len] = 0;
	}

	return(s + len);
}
