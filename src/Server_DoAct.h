#ifndef SERVER_DOACT_H_
#define SERVER_DOACT_H_

void init_talkerbox();
void handle_accept(int epollfd, int sock);
void handle_close(int epollfd, int sock);
int handle_receive(int sock, int opcode, unsigned char *packet, int packet_length);
int forward_msg(int sock, unsigned char *packet, int packet_length);

#endif /* SERVER_DOACT_H_ */
