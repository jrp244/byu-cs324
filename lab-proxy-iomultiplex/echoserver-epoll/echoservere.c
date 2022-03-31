#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/epoll.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>

#define MAXEVENTS 64
#define MAXLINE 2048


struct client_info {
	int fd;
	char desc[1024];
};

int main(int argc, char **argv) 
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, connfd;
	int af;
	int portindex;

	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	int efd;
	struct epoll_event event;
	struct epoll_event *events;
	int i;
	int len;

	struct client_info *new_client;
	struct client_info *listener;
	struct client_info *active_client;

	size_t n; 
	char buf[MAXLINE]; 

	if (!(argc == 2 || (argc == 3 &&
			(strcmp(argv[1], "-4") == 0 || strcmp(argv[1], "-6") == 0)))) {
		fprintf(stderr, "Usage: %s [ -4 | -6 ] port\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc == 2) {
		portindex = 1;
	} else {
		portindex = 2;
	}
	/* Use IPv4 by default (or if -4 is used).  If IPv6 is specified,
	 * then use that instead. */
	if (argc == 2 || strcmp(argv[portindex], "-4") == 0) {
		af = AF_INET;
	} else {
		af = AF_INET6;
	}

	/* pre-socket setup; getaddrinfo() */

	memset(&hints, 0, sizeof(struct addrinfo));

	/* As a server, we want to exercise control over which protocol (IPv4
	   or IPv6) is being used, so we specify AF_INET or AF_INET6 explicitly
	   in hints, depending on what is passed on on the command line. */
	hints.ai_family = af;	/* Choose IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	/* For wildcard IP address */
	hints.ai_protocol = 0;		  /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(NULL, argv[portindex], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.  However,
	   because we have only specified a single address family (AF_INET or
	   AF_INET6) and have only specified the wildcard IP address, there is
	   no need to loop; we just grab the first item in the list. */
	if ((s = getaddrinfo(NULL, argv[portindex], &hints, &result)) < 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	if ((sfd = socket(result->ai_family, result->ai_socktype, 0)) < 0) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}

	if (listen(sfd, 100) < 0) {
		perror("Could not listen");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);   /* No longer needed */

	if ((efd = epoll_create1(0)) < 0) {
		fprintf(stderr, "error creating epoll fd\n");
		exit(1);
	}


	// set listening file descriptor non-blocking
	if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "error setting socket option\n");
		exit(1);
	}

	// allocate memory for a new struct client_info, and populate it with
	// info for the listening socket
	listener = malloc(sizeof(struct client_info));
	listener->fd = sfd;
	sprintf(listener->desc, "Listen file descriptor (accepts new clients)");

	// register the listening file descriptor for incoming events using
	// edge-triggered monitoring
	event.data.ptr = listener;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		exit(1);
	}

	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof(struct epoll_event));

	while (1) {
		// wait for event to happen (-1 == no timeout)
		n = epoll_wait(efd, events, MAXEVENTS, -1);

		for (i = 0; i < n; i++) {
			// grab the data structure from the event, and cast it
			// (appropriately) to a struct client_info *.
			active_client = (struct client_info *)(events[i].data.ptr);

			printf("New event for %s\n", active_client->desc);

			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				/* An error has occured on this fd */
				fprintf(stderr, "epoll error on %s\n", active_client->desc);
				close(active_client->fd);
				free(active_client);
				continue;
			}

			if (sfd == active_client->fd) {
				clientlen = sizeof(struct sockaddr_storage); 

				// loop until all pending clients have been accepted
				while (1) {
					connfd = accept(active_client->fd, (struct sockaddr *)&clientaddr, &clientlen);

					if (connfd < 0) {
						if (errno == EWOULDBLOCK ||
								errno == EAGAIN) {
							// no more clients ready to accept
							break;
						} else {
							perror("accept");
							exit(EXIT_FAILURE);
						}
					}

					// set client file descriptor non-blocking
					if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
						fprintf(stderr, "error setting socket option\n");
						exit(1);
					}

					// allocate memory for a new struct
					// client_info, and populate it with
					// info for the new client
					new_client = (struct client_info *)malloc(sizeof(struct client_info));
					new_client->fd = connfd;
					sprintf(new_client->desc, "Client with file descriptor %d", connfd);

					// register the client file descriptor
					// for incoming events using
					// edge-triggered monitoring
					event.data.ptr = new_client;
					event.events = EPOLLIN | EPOLLET;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
						fprintf(stderr, "error adding event\n");
						exit(1);
					}
				}
			} else {
				// read from socket until (1) the remote side
				// has closed the connection or (2) there is no
				// data left to be read.
				while (1) {
					len = recv(active_client->fd, buf, MAXLINE, 0);   
					if (len == 0) { // EOF received
						// closing the fd will automatically
						// unregister the fd from the efd
						close(active_client->fd);
						free(active_client);
						break;
					} else if (len < 0) {
						if (errno == EWOULDBLOCK ||
								errno == EAGAIN) {
							// no more data to be read
						} else {
							perror("client recv");
							close(active_client->fd);
							free(active_client);
						}
						break;
					} else {
						printf("Received %d bytes\n", len);
						send(active_client->fd, buf, len, 0);
					}
				}
			}
		}
	}
	free(events);
	free(listener);
}
