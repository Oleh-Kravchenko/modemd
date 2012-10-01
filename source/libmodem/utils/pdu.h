#ifndef __PDU_H
#define __PDU_H

char* encode_pdu(const uint8_t* data, size_t len);

size_t decode_pdu(const char* enc, uint8_t** data);

#endif /* __PDU_H */
