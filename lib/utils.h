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

#endif /* __THREAD_H */
