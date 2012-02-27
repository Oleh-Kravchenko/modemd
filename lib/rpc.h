#ifndef _CELLULARD_RPC_H
#define _CELLULARD_RPC_H

#include <stdint.h>

/*-------------------------------------------------------------------------*/

typedef enum {
    TYPE_QUERY = 0,
    TYPE_RESPONSE
} __attribute__((__packed__)) proto_type_t;

/*-------------------------------------------------------------------------*/

typedef struct
{
    struct
    {
        /** type of packet */
        proto_type_t type;

        /** length of function name */
        uint8_t func_len;

        /** length of data field */
        uint16_t data_len;
    } hdr;

    /** function name */
    char* func;

    /** data */
    uint8_t* data;
} __attribute__((__packed__)) packet_t;

/*-------------------------------------------------------------------------*/

int rpc_send(int sock, packet_t *p);

/**
 * @brief receive packet over socket
 * @param sock socket
 * @return packet
 *
 * Must be called rpc_free() for packet.
 */
packet_t* rpc_recv(int sock);

/**
 * @brief free memory used by packet
 * @param p packet
 */
void rpc_free(packet_t *p);

/**
 * @brief printing packet to stdout
 * @param p packet
 */
void rpc_print(packet_t *p);

#endif /* _CELLULARD_RPC_H */
