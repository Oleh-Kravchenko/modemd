#include <stdlib.h>
#include <string.h>

#include "re.h"

/*------------------------------------------------------------------------*/

int re_atoi(const char* src, const regmatch_t* re_subs)
{
	int res = 0;
	size_t len;
	char* buf;

	len = re_subs->rm_eo - re_subs->rm_so;

	if(!(buf = malloc(len + 1)))
		return(res);

	memcpy(buf, src + re_subs->rm_so, len);
	buf[len] = 0;

	res = atoi(buf);

	free(buf);

	return(res);
}

/*------------------------------------------------------------------------*/

char* re_strncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs)
{
	size_t len;

	len = re_subs->rm_eo - re_subs->rm_so;
	len = (len > n ? n - 1 : len);

	memcpy(dst, src + re_subs->rm_so, len);
	dst[len] = 0;

	return(dst + len);
}

/*------------------------------------------------------------------------*/

char* re_strdup(const char* src, const regmatch_t* re_subs)
{
	size_t len;
	char* res;

	len = re_subs->rm_eo - re_subs->rm_so;

	if(!(res = malloc(len + 1)))
		return(res);

	memcpy(res, src + re_subs->rm_so, len);
	res[len] = 0;

	return(res);
}

/*------------------------------------------------------------------------*/

int re_strcmp(const char* s, const char* mask)
{
	regex_t re;
	int res;

	regcomp(&re, mask, REG_EXTENDED);
	res = regexec(&re, s, 0, NULL, 0);
	regfree(&re);

	return(res);
}

/*------------------------------------------------------------------------*/

int re_parse(const char* s, const char* mask, size_t* nmatch, regmatch_t** pmatch)
{
	regex_t re;
	int res;

	*nmatch = 0;
	*pmatch = NULL;

	if((res = regcomp(&re, mask, REG_EXTENDED)))
		return(res);

	*nmatch = re.re_nsub + 1;

	if((*pmatch = malloc(sizeof(regmatch_t) * (*nmatch))) == NULL)
	{
		res = -1;
		goto err;
	}

	if((res = regexec(&re, s, *nmatch, *pmatch, 0)))
		free(*pmatch);

err:
	regfree(&re);

	return(res);
}
