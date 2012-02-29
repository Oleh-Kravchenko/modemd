#ifndef __THREAD_H
#define __THREAD_H

/**
 * @brief read file contents into the string buffer
 * @param filename name of file
 * @param s output buffer for string
 * @param size size of output buffer
 * @return Pointer to s, if successful
 */
char* file_get_contents(const char *filename, char* s, const int size);

/**
 * @brief read file contents as a hex string
 * @param filename name of file
 * @return int
 */
int file_get_contents_hex(const char* filename);

/**
 * @brief check vendor and product id on modem db
 * @param vendor vendor id
 * @param product product id
 * @return 0 if this device is not a modem
 */
int its_modem(uint16_t vendor, uint16_t product);

#endif /* __THREAD_H */
