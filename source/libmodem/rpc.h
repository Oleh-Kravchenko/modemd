#ifndef __MODEMD_RPC_H
#define __MODEMD_RPC_H

#include <stdint.h>

/***************************************************************************

			client			 |  network  |			 server
							   |		   |
							   |		   |
		   /- pack parameters; | --------> | unpack parameters; -\
function -+					|		   |					 +- function
		   \- unpack results;  | <-------- |	 pack results;  -/
							   |		   |

***************************************************************************/

/*------------------------------------------------------------------------*/

typedef enum {
	TYPE_QUERY = 0,
	TYPE_RESPONSE
} __attribute__((__packed__)) rpc_packet_type_t;

/*------------------------------------------------------------------------*/

typedef struct
{
	struct
	{
		/** type of packet */
		rpc_packet_type_t type;

		/** length of function name */
		uint8_t func_len;

		/** length of data field */
		uint16_t data_len;
	} hdr;

	/** function name */
	char* func;

	/** data */
	uint8_t* data;
} __attribute__((__packed__)) rpc_packet_t;

/*------------------------------------------------------------------------*/

/**
 * @brief create and return pointer for packet
 * @param type type of packet
 * @param func function name
 * @param data pointer to data buffer
 * @param data_len length of data
 * @return if successeful pointer to packet
 *
 * Result must be free by function free()
 */
rpc_packet_t* rpc_create(rpc_packet_type_t type, const char* func, const uint8_t* data, uint16_t data_len);

/**
 * @brief receive packet over socket
 * @param sock socket
 * @param p socket
 * @return Sended bytes of packet
 */
int rpc_send(int sock, rpc_packet_t *p);

/**
 * @brief receive packet over socket
 * @param sock socket
 * @return packet
 *
 * Packet must be free by function rpc_free()
 */
rpc_packet_t* rpc_recv(int sock);

/**
 * @brief receive packet over socket
 * @param sock socket
 * @param func receive only this function
 * @param tries give up after tries, must be great 0
 * @return pointer to packet, or NULL if failed
 *
 * Packet must be free by function rpc_free()
 */
rpc_packet_t* rpc_recv_func(int sock, const char* func, int tries);

/**
 * @brief free memory used by packet
 * @param p packet
 */
void rpc_free(rpc_packet_t *p);

/**
 * @brief printing packet to stdout
 * @param p packet
 */
void rpc_print(rpc_packet_t *p);

#endif /* __MODEMD_RPC_H */
