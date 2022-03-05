/* 
 * echoserverp.c - A concurrent echo server based on processes
 */
/* $begin echoserverpmain */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 8192

void echo(int connfd);

void sigchld_handler(int sig) {
	while (waitpid(-1, 0, WNOHANG) > 0)
		;
	return;
}

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, connfd;
	int af;
	int portindex;

	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

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

	signal(SIGCHLD, sigchld_handler);

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

	while (1) {
		clientlen = sizeof(struct sockaddr_storage); 
		connfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientlen);
		if (fork() == 0) { 
			close(sfd); /* Child closes its listening socket */
			echo(connfd);    /* Child services client */ //line:conc:echoserverp:echofun
			close(connfd);   /* Child closes connection with client */ //line:conc:echoserverp:childclose
			exit(0);         /* Child exits */
		}
		close(connfd); /* Parent closes connected socket (important!) */ //line:conc:echoserverp:parentclose
	}
}
/* $end echoserverpmain */
