#ifndef SERVER_MAIN_H_
#define SERVER_MAIN_H_

int readn(int sock, unsigned char * buf, int size);
void error_handling(const char *format, ...);
void warn_handling(const char *format, ...);

#endif /* SERVER_MAIN_H_ */
