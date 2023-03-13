#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl()\n");
		exit(1);
	}
}

static int setnonblocking(int sockfd)
{
	if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) ==
		-1) {
		return -1;
	}
	return 0;
}


int main()
{
	int i;
	int n;
	int epfd;
	int nfds;
	int listen_sock;
	int conn_sock;
	socklen_t socklen;
	char buf[255];
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	struct epoll_event events[32];

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&srv_addr, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(8080);
	bind(listen_sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	setnonblocking(listen_sock);
	listen(listen_sock, 16);

	epfd = epoll_create(1);
	epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

	socklen = sizeof(cli_addr);
	while (1)
	{
		nfds = epoll_wait(epfd, events, 32, -1);
		for (i = 0; i < nfds; i++)
		{
			if (events[i].data.fd == listen_sock)
			{
				/* handle new connection */
				conn_sock =
						accept(listen_sock,
							   (struct sockaddr *) &cli_addr,
							   &socklen);

				inet_ntop(AF_INET, (char *) &(cli_addr.sin_addr),
						  buf, sizeof(cli_addr));
				printf("[+] connected with %s:%d\n", buf,
					   ntohs(cli_addr.sin_port));

				setnonblocking(conn_sock);
				epoll_ctl_add(epfd, conn_sock,
							  EPOLLIN | EPOLLET | EPOLLRDHUP |
							  EPOLLHUP);
			} else if (events[i].events & EPOLLIN)
			{
				for (;;)
				{
					bzero(buf, sizeof(buf));
					n = read(events[i].data.fd, buf,
							 sizeof(buf));
					if (n <= 0|| errno == EAGAIN)
					{
						break;
					} else
					{
						printf("[+] data: %s\n", buf);
						write(events[i].data.fd, buf,
							  strlen(buf));
					}
				}
			} else
			{
				printf("[+] unexpected\n");
			}
			/* check if the connection is closing */
			if (events[i].events & (EPOLLRDHUP | EPOLLHUP))
			{
				printf("[+] connection closed\n");
				epoll_ctl(epfd, EPOLL_CTL_DEL,
						  events[i].data.fd, NULL);
				close(events[i].data.fd);
				continue;
			}
		}
	}
}


