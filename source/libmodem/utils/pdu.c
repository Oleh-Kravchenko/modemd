#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------*/

static const char hex_alphabet[] = "0123456789ABCDEF";

/*------------------------------------------------------------------------*/

char* encode_pdu(const uint8_t* data, size_t len)
{
	uint8_t packed, round;
	char *res, *res_i;
	size_t i;
	div_t q;

	/* invalid length */
	if(!len)
		return(NULL);

	q = div(len * 7, 8);

	/* calculation memory size for result */
	if(!(res = malloc(((q.quot + !!q.rem) * 2) + 1)))
		return(NULL);

	res_i = res;

	/* pack bytes without 7bit */
	for(i = 0; i < len; ++ i)
	{
		round = i % 8;

		if(round == 7)
			continue;

		/* byte of packed data */
		packed = (data[i] & 0x7f) >> round;
		packed |= (data[i + 1] & ((1 << round + 1) - 1)) << 7 - round;

		/* store result as HEX value string */
		*res_i ++ = hex_alphabet[(packed & 0xf0) >> 4];
		*res_i ++ = hex_alphabet[packed & 0x0f];
	}

	/* string end \0 */
	*res_i = '\0';

	return(res);
}

/*------------------------------------------------------------------------*/

size_t decode_pdu(const char* enc, uint8_t** data)
{
	uint8_t l, h;
	uint8_t b, prev = 0, step = 0, i = 0;
	uint8_t* res_s;

	if(!(*data = malloc(strlen(enc) * 4 / 7)))
		return(0);

	res_s = *data;

	while(*enc)
	{
		/* parsing byte */
		/* high */
		if((h = *enc ++ - 0x30) > 9)
			h -= 7;

		/* low */
		if((l = *enc ++ - 0x30) > 9)
			l -= 7;

		/* error, invalid characters in the string */
		if(l > 0xf || h > 0xf)
			break;

		step = (i ++) % 7;

		b = (h << 4) | l;

		/* converting 7 bit to 8 bit */
		*res_s ++ = ((b << step) | (prev >> (8 - step))) & 0x7f;

		/* last byte in the round (7 byte -> 8 byte) */
		if(step == 6)
			*res_s ++ = b >> 1;

		prev = b;
	}

	return(res_s - *data);
}
