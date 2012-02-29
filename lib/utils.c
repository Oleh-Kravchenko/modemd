#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*-------------------------------------------------------------------------*/

char* file_get_contents(const char *filename, char* s, const int size)
{
    FILE *f;
    char* res;

    if(!(f = fopen(filename, "r")))
        return(0);

    res = fgets(s, size, f);

    fclose(f);

    int rn = strlen(res);

    /* removing eof */
    if(rn && (res[rn - 1] == '\n' || res[rn - 1] == '\n'))
        res[rn - 1] = 0;

    return(res);
}

/*-------------------------------------------------------------------------*/

int file_get_contents_hex(const char* filename)
{
    char hex[256];
    int res = 0;

    if(file_get_contents(filename, hex, sizeof(hex)))
        sscanf(hex, "%x", &res);

    return(res);
}

/*-------------------------------------------------------------------------*/

int its_modem(uint16_t vendor, uint16_t product)
{
    return(vendor == 0x1199 && product == 0x68a3);
}
