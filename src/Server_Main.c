#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "Common.h"
#include "Server_Main.h"
#include "Server_DoAct.h"

int main(int argc, char *argv[]) {
	setbuf(stdout, 0);

	system("ifconfig | grep \"192.168\"");

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;
	int option;

	struct epoll_event *ep_events;
	struct epoll_event event;
	int epfd, event_cnt;

	int i;
	unsigned char buf[RESV_BUF_SIZE];
	int packet_length, resv_ret;

	// 0. Setup
	init_talkerbox();

	// 1. Socket creation
	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error_handling("socket error");

	option = 1;
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
		error_handling("setsockopt error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(SERVER_PORT);

	// 2. Bind & Listen
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind error");

	if (listen(serv_sock, MAX_LISTEN) == -1)
		error_handling("listen error");

	// 3. Epoll creation
	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while (1) {
		// 4. Epoll wait
		if ((event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1)) == -1)
			error_handling("epoll_wait error");

		for (i = 0; i < event_cnt; i++) {
			// 5-1. Handle Accept
			if (ep_events[i].data.fd == serv_sock) {
				adr_sz = sizeof(clnt_adr);
				clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &adr_sz);
				event.events = EPOLLIN;
				event.data.fd = clnt_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);

				printf("+ accepted(sock desc:%d)\n", clnt_sock);
				handle_accept(epfd, clnt_sock);
			}
			// 5-2. Handle Receive
			else {
				resv_ret = FAIL;

				// 5-2-1. Handle Receive Packet Head
				if (readn(ep_events[i].data.fd, buf, HEADER_SIZE) == 0) {
					packet_length = (buf[0] << 8) + buf[1];

					// 5-2-2. Handle Receive Packet Body
					if (readn(ep_events[i].data.fd, buf + 2, packet_length - HEADER_SIZE) == 0) {
						resv_ret = handle_receive(ep_events[i].data.fd, buf[2], buf, packet_length);
					}
				}

				if (resv_ret == FAIL) {
					handle_close(epfd, ep_events[i].data.fd);
				}
			}
		}
	}

	// Cannot reach here
	close(serv_sock);
	close(epfd);

	return 0;
}

int readn(int sock, unsigned char * buf, int size) {
	int ok = 0;

	while (ok < size) {
		int cnt = read(sock, buf, size);

		if (cnt <= 0) {
			return FAIL;
		}

		ok += cnt;
	}

	return SUCCESS;
}

void error_handling(const char *format, ...) {
	va_list ap;
	char buf[1024];
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	fprintf(stdout, "[ERROR] %s", buf);
	exit(0x99);
}

void warn_handling(const char *format, ...) {
	va_list ap;
	char buf[1024];
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	fprintf(stdout, "[WARNING] %s", buf);
}
