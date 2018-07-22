#ifndef COMMON_H_
#define COMMON_H_

#define SERVER_PORT (9999)

#define MAX_CLIENT (1000)
#define MAX_CLIENT_SET (MAX_CLIENT / 2)
#define MAX_LISTEN (MAX_CLIENT)
#define EPOLL_SIZE (MAX_CLIENT)

#define HEADER_SIZE (2)
#define RESV_BUF_SIZE (4096)

#define PACKET_OPCODE_RESV_HELLO (0x80)
#define PACKET_OPCODE_RESV_MSG (0x81)
#define PACKET_OPCODE_SEND_MSG (0x00)

#define SUCCESS (0)
#define FAIL (-1)

#endif /* COMMON_H_ */
