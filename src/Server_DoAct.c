#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "Common.h"
#include "Server_DoAct.h"
#include "Server_Main.h"

struct TalkBox {
	int sock_A;
	int sock_B;
};

static struct TalkBox talkerbox[MAX_CLIENT_SET];

void init_talkerbox() {
	memset(talkerbox, 0, sizeof(talkerbox));
}

void handle_accept(int epollfd, int sock) {
	static unsigned char hello_packet[3] = { 0, 3, PACKET_OPCODE_RESV_HELLO };
	static int accept_start_index = 0;

	for (int i = accept_start_index; i < MAX_CLIENT_SET; i++) {
		if (talkerbox[i].sock_A == 0 && talkerbox[i].sock_B == 0) {
			talkerbox[i].sock_A = sock;
			break;
		} else if (talkerbox[i].sock_B == 0) {
			talkerbox[i].sock_B = sock;
			accept_start_index = i + 1;

			if (accept_start_index >= MAX_CLIENT_SET) {
				accept_start_index = 0;
			}

			if (write(talkerbox[i].sock_A, hello_packet, 3) == -1) {
				warn_handling("handle_accept error(sock desc:%d)", talkerbox[i].sock_A);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_A, NULL);
				close(talkerbox[i].sock_A);
				talkerbox[i].sock_A = 0;
			}

			if (write(talkerbox[i].sock_B, hello_packet, 3) == -1) {
				warn_handling("handle_accept error(sock desc:%d)", talkerbox[i].sock_B);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_B, NULL);
				close(talkerbox[i].sock_B);
				talkerbox[i].sock_B = 0;
			}

			break;
		}
	}
}

void handle_close(int epollfd, int sock) {
	for (int i = 0; i < MAX_CLIENT_SET; i++) {
		if (talkerbox[i].sock_A == sock) {
			printf("- closed(sock desc:%d)\n", talkerbox[i].sock_A);
			epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_A, NULL);
			close(talkerbox[i].sock_A);
			talkerbox[i].sock_A = 0;

			if (talkerbox[i].sock_B) {
				printf("- closed(sock desc:%d)\n", talkerbox[i].sock_B);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_B, NULL);
				close(talkerbox[i].sock_B);
				talkerbox[i].sock_B = 0;
			}
			break;
		} else if (talkerbox[i].sock_B == sock) {
			printf("- closed(sock desc:%d)\n", talkerbox[i].sock_B);
			epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_B, NULL);
			close(talkerbox[i].sock_B);
			talkerbox[i].sock_B = 0;

			if (talkerbox[i].sock_A) {
				printf("- closed(sock desc:%d)\n", talkerbox[i].sock_A);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, talkerbox[i].sock_A, NULL);
				close(talkerbox[i].sock_A);
				talkerbox[i].sock_A = 0;
			}
			break;
		}
	}
}

int handle_receive(int sock, int opcode, unsigned char *packet, int packet_length) {
	switch (opcode) {
	case PACKET_OPCODE_SEND_MSG:
		return forward_msg(sock, packet, packet_length);
	default:
		return FAIL;
	}
}

int forward_msg(int sock, unsigned char *packet, int packet_length) {
	packet[2] = PACKET_OPCODE_RESV_MSG;

	for (int i = 0; i < MAX_CLIENT_SET; i++) {
		if (talkerbox[i].sock_A == sock) {
			if (write(talkerbox[i].sock_B, packet, packet_length) == -1) {
				warn_handling("write fail");
				return FAIL;
			}
			return SUCCESS;
		} else if (talkerbox[i].sock_B == sock) {
			if (write(talkerbox[i].sock_A, packet, packet_length) == -1) {
				warn_handling("write fail");
				return FAIL;
			}
			return SUCCESS;
		}
	}

	return FAIL;
}
