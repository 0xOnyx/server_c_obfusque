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

static void setnonblocking(int s)
{
	fcntl(s, F_SETFD, fcntl(s, F_GETFD, 0) | O_NONBLOCK);
}


int main()
{
	int i,e,n,l;
	int conn_sock;
	socklen_t socklen;
	char buf[255];
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	struct epoll_event events[32];

	l = socket(2, SOCK_STREAM, 0);

	bzero(&srv_addr, sizeof(struct sockaddr_in));
	srv_addr.sin_family = 2;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(8080);
	bind(l, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	setnonblocking(l);
	listen(l, 16);

	e = epoll_create(1);
	epoll_ctl_add(e, l, EPOLLIN | EPOLLOUT | EPOLLET);

	socklen = sizeof(cli_addr);
	while (1)
	{
		n = epoll_wait(e, events, 32, -1);
		for (i = 0; i < n; i++)
		{
			if (events[i].data.fd == l)
			{
				conn_sock =
						accept(l,
							   (struct sockaddr *) &cli_addr,
							   &socklen);

				inet_ntop(2, (char *) &(cli_addr.sin_addr),
						  buf, sizeof(cli_addr));
				printf("[+] connected with %s:%d\n", buf,
					   ntohs(cli_addr.sin_port));

				setnonblocking(conn_sock);
				epoll_ctl_add(e, conn_sock,
							  EPOLLIN | EPOLLET | EPOLLRDHUP |
							  EPOLLHUP);
			} else if (events[i].events & EPOLLIN)
			{
				for (;;)
				{
					bzero(buf, sizeof(buf));
					if (read(events[i].data.fd, buf,
							 sizeof(buf)) <= 0|| errno == EAGAIN)
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
			if (events[i].events & (EPOLLRDHUP | EPOLLHUP))
			{
				printf("[+] connection closed\n");
				epoll_ctl(e, EPOLL_CTL_DEL,
						  events[i].data.fd, NULL);
				close(events[i].data.fd);
				continue;
			}
		}
	}
}
