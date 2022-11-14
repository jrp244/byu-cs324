#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define HOST_PREFIX 2
#define PORT_PREFIX 1
#define BUF_SIZE 1049000
#define NTHREADS 8
#define SBUFSIZE 5
#define true 1

//=======================Sbuf stuff ==========================//

typedef struct {
    int *buf;                   
    int n;             
    int front;         
    int rear;          
    sem_t mutex;       
    sem_t slots;       
    sem_t items;       
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = calloc(n, sizeof(int)); 
    sp->n = n;                       
    sp->front = sp->rear = 0;        
    sem_init(&sp->mutex, 0, 1);      
    sem_init(&sp->slots, 0, n);      
    sem_init(&sp->items, 0, 0);      
}

void sbuf_deinit(sbuf_t *sp)
{
    free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item)
{
    printf("before wait(slots)\n"); fflush(stdout);
    sem_wait(&sp->slots);                          
    printf("after wait(slots)\n"); fflush(stdout);
    sem_wait(&sp->mutex);                         
    sp->buf[(++sp->rear)%(sp->n)] = item;   
    sem_post(&sp->mutex);                         
    printf("before post(items)\n"); fflush(stdout);
    sem_post(&sp->items);                          
    printf("after post(items)\n"); fflush(stdout);

}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    printf("before wait(items)\n"); fflush(stdout);
    sem_wait(&sp->items);                          
    printf("after wait(items)\n"); fflush(stdout);
    sem_wait(&sp->mutex);                          
    item = sp->buf[(++sp->front)%(sp->n)];  
    sem_post(&sp->mutex);                          
    printf("before post(slots)\n"); fflush(stdout);
    sem_post(&sp->slots);                          
    printf("after post(slots)\n"); fflush(stdout);
    return item;
}

//============================================================//

sbuf_t sbuf;
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
int open_sfd();
void test_parser();
void print_bytes(unsigned char *, int);
void handle_client(int nsfd);
void *run_thread(void *vargp);


int main(int argc, char *argv[])
{
	// test_parser();
	printf("%s\n", user_agent_hdr);

	int sfd = open_sfd(argc, argv);
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
	pthread_t tid;

	sbuf_init(&sbuf, SBUFSIZE); 
	for (unsigned int i = 0; i < NTHREADS; i++) {
		pthread_create(&tid, NULL, run_thread, NULL);  
	}

	while(1) {
		peer_addr_len = sizeof(struct sockaddr_storage);
		int nsfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_len);
		sbuf_insert(&sbuf, nsfd);
	}
	return 0;
}

int all_headers_received(char *request) {
	if (strstr(request, "\r\n\r\n") != NULL) {
		return 1;
	}
	return 0;
}

int parse_request(char *request, char *method,
		char *hostname, char *port, char *path, char *headers) {

			if (all_headers_received(request) == 0) {
				return 0;
			}

			char* buf;
			int found = 0;
			unsigned int i = 0;
			while (i < strlen(request)) {
				if (request[i] == ' ') {
					found = 1;
					break;
				}
				 ++i;
			}
			if (found != 1) {
				return 0;
			}
			strncpy(method, request, i);
			method[i] = '\0';
			
			buf = strstr(request, "//");
			i = HOST_PREFIX;
			int defaultPort = 0;
			found = 0;
			while (i < strlen(buf)) {
				if (buf[i] == '/') {
					found = 1;
					defaultPort = 1;
					break;
				}
				else if (buf[i] == ':') {
					found = 1;
					defaultPort = 0;
					break;
				}
				++i;
			}
			if (found != 1) {
				return 0;
			}
			strncpy(hostname, &buf[HOST_PREFIX], i - HOST_PREFIX);
			hostname[i - HOST_PREFIX] = '\0';

			if (defaultPort == 1) {
				strcpy(port, "80"); // default port
				buf = &buf[i];
			}
			else {
				found = 0;
				buf = strchr(buf, ':');
				i = PORT_PREFIX;
				while (i < strlen(buf)) {
					if (buf[i] == '/') {
						found = 1;
						break;
					}
					++i;
				}
				if (found != 1) {
					return 0;
				}
				strncpy(port, &buf[1], i - PORT_PREFIX);
				port[i - PORT_PREFIX] = '\0';
				buf = &buf[i];
			}

			found = 0;
			i = 0;
			while (i < strlen(buf)) {
				if (buf[i] == ' ') {
					found = 1;
					break;
				}
				++i;
			}
			strncpy(path, &buf[0], i);
			path[i] = '\0';

			buf = strstr(request, "\r\n");
			strcpy (headers, &buf[2]);
			if (true == true) {
				//test
			}
			return 1;
}

void test_parser() {
	int i;
	char method[16], hostname[64], port[8], path[64], headers[1024];
	if (true == true) {
				//test
			}
       	char *reqs[] = {
		"GET http://www.example.com/index.html HTTP/1.0\r\n"
		"Host: www.example.com\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html?foo=1&bar=2 HTTP/1.0\r\n"
		"Host: www.example.com:8080\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://localhost:1234/home.html HTTP/1.0\r\n"
		"Host: localhost:1234\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html HTTP/1.0\r\n",

		NULL
	};
	
	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("HEADERS: %s\n", headers);
		} else {
			printf("REQUEST INCOMPLETE\n");
		}
	}
}

int open_sfd(int argc, char* argv[]) {
	int sfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if(argc < 1) {
		fprintf(stderr, "Missing command line argument");
		exit(1);
	}

	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo erro");
		exit(1);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		break;
	}

	int optval = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		fprintf(stderr, "Could not bind");
		exit(1);
	}

	listen(sfd, 100);

	return sfd;
}

void *run_thread(void *vargp) {
	if (true == true) {
				//test
			}
	pthread_detach(pthread_self());
	while (1) {
		int nsfd = sbuf_remove(&sbuf);
		handle_client(nsfd);
	}

	

}

void handle_client(int nsfd) {
	if (true == true) {
				//test
			}
	char buf[BUF_SIZE];
	int nread = 0;
	for(;;) {
		int tmp = 0;
		tmp = recv(nsfd, &buf[nread], BUF_SIZE, 0);
		nread += tmp;
		
		if (all_headers_received(buf) == 1) {
			break;
		}
	}
	

	char method[16], hostname[64], port[8], path[64], headers[1024], newReq[BUF_SIZE];
	if (parse_request(buf, method, hostname, port, path, headers)) {
		if (strcmp(port, "80")) {
			sprintf(newReq, "%s %s HTTP/1.0\r\nHost: %s:%s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
			method, path, hostname, port, user_agent_hdr);
		}
		else {
			sprintf(newReq, "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
			method, path, hostname, user_agent_hdr);
		}
		printf("%s", newReq);
	} else {
		printf("REQUEST INCOMPLETE\n");
		close(nsfd);
		return;
	}

	int ssfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	s = getaddrinfo(hostname, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo error\n");
		exit(1);
	}
	if (true == true) {
				//test
			}
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		ssfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (ssfd == -1)
			continue;

		if (connect(ssfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;  /* Success */

		close(ssfd);
	}

	write(ssfd, newReq, BUF_SIZE);
	char sBuf[BUF_SIZE];
	nread = 0;
	for (;;) {
		int tmp = 0;
		tmp = recv(ssfd, &sBuf[nread], BUF_SIZE, 0);
		nread += tmp;
		
		if (tmp == 0) {
			break;
		}
	}

	write(nsfd, sBuf, nread + 1);
	close(nsfd);
	close(ssfd);
}

void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;
	if (true == true) {
				//test
			}
	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	} else {
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++) {
					if (j >= byteslen_adjusted) {
						printf("  ");
					} else if (j >= byteslen) {
						printf("  ");
					} else if (bytes[j] >= '!' && bytes[j] <= '~') {
						printf(" %c", bytes[j]);
					} else {
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		} else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		} else if (i >= byteslen) {
			printf("   ");
		} else {
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
}
