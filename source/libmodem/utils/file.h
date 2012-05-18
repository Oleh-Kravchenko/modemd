#ifndef __FILE_H
#define __FILE_H

/**
 * @brief open serial device
 * @param portname port name
 * @param flags flags (O_RDONLY, O_WRONLY, or O_RDWR), see man open
 * @return -1 if an error occurred
 */
int serial_open(const char* tty, int flags);

/*------------------------------------------------------------------------*/
/**
 * @brief read file contents into the string buffer
 * @param filename name of file
 * @param s output buffer for string
 * @param size size of output buffer
 * @return Pointer to s, if successful
 */
char* file_get_contents(const char *filename, char* s, const int size);

/*------------------------------------------------------------------------*/

/**
 * @brief read file contents as a hex string
 * @param filename name of file
 * @return unsigned int
 */
unsigned int file_get_contents_hex(const char* filename);

#endif /* __FILE_H */
