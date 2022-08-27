#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

int main(int argc, char *argv[]) {
	int portindex;
	unsigned short port;
	int address_family;
	int sock_type;
	struct sockaddr_in ipv4addr;
	struct sockaddr_in6 ipv6addr;

	int sfd;
	int s;
	struct sockaddr_storage remote_addr;
	struct sockaddr *local_addr;
	socklen_t local_addr_len, remote_addr_len;

	ssize_t nread;
	char buf[BUF_SIZE];

	/* Check usage */
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

	/* Use IPv4 by default (or if -4 is specified);
	 * If -6 is specified, then use IPv6 instead. */
	if (argc == 2 || strcmp(argv[portindex], "-4") == 0) {
		address_family = AF_INET;
	} else {
		address_family = AF_INET6;
	}

	port = atoi(argv[portindex]);
	sock_type = SOCK_DGRAM;


	/* SECTION A - populate address structures */

	if (address_family == AF_INET) {
		ipv4addr.sin_family = address_family;
		ipv4addr.sin_addr.s_addr = INADDR_ANY; // listen on any/all IPv4 addresses
		ipv4addr.sin_port = htons(port);       // specify port explicitly, in network byte order

		// Assign local_addr and local_addr_len to ipv4addr
		local_addr = (struct sockaddr *)&ipv4addr;
		local_addr_len = sizeof(ipv4addr);
	} else { // address_family == AF_INET6
		ipv6addr.sin6_family = address_family;
		ipv6addr.sin6_addr = in6addr_any;     // listen on any/all IPv6 addresses
		ipv6addr.sin6_port = htons(port);     // specify port explicitly, in network byte order

		// Assign local_addr and local_addr_len to ipv6addr
		local_addr = (struct sockaddr *)&ipv6addr;
		local_addr_len = sizeof(ipv6addr);
	}


	/* SECTION B - setup socket */

	if ((sfd = socket(address_family, sock_type, 0)) < -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}
	if (bind(sfd, local_addr, local_addr_len) < 0) {
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}


	/* SECTION C - interact with clients; receive and send messages */

	/* Read datagrams and echo them back to sender */

	for (;;) {
		remote_addr_len = sizeof(struct sockaddr_storage);
		nread = recvfrom(sfd, buf, BUF_SIZE, 0,
				(struct sockaddr *) &remote_addr, &remote_addr_len);
		if (nread == -1)
			continue;   /* Ignore failed request */

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *) &remote_addr,
						remote_addr_len, host, NI_MAXHOST,
						service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
	
		if (s == 0)
			printf("Received %zd bytes from %s:%s\n",
					nread, host, service);
		else
			fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

		if (sendto(sfd, buf, nread, 0,
					(struct sockaddr *) &remote_addr,
					remote_addr_len) < 0)
			fprintf(stderr, "Error sending response\n");
	}
}
