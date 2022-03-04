# Socket Treasure Hunt

The purpose of this assignment is to help you become more familiar with the
concepts associated with sockets, including UDP communications, local and
remote port assignment, IPv4 and IPv6, message parsing, and more.

# Table of Contents

 - [Overview](#overview)
   - [Initial Request](#initial-request)
   - [Directions Response](#directions-response)
   - [Directions Request](#directions-request)
   - [Levels](#levels)
   - [Message Formatting](#message-formatting)
   - [Socket Setup and Manipulation](#socket-setup-and-manipulation)
   - [Usage](#usage)
   - [Output](#output)
 - [Preparation](#preparation)
   - [Reading](#reading)
 - [Instructions](#instructions)
   - [Getting Started](#getting-started)
   - [Initial Send and Receive](#initial-send-and-receive)
   - [Collect and Print the Level 0 Treasure](#collect-and-print-the-level-0-treasure)
   - [Print the Socket Information (Optional)](#print-the-socket-information-extra-credit)
   - [Generalize the Inputs](#generalize-the-inputs)
   - [Remove Any Extra Print Statements](#remove-any-extra-print-statements)
   - [Checkpoint 0](#checkpoint-0)
   - [Future Levels and Checkpoints](#future-levels-and-checkpoints)
 - [Testing Servers](#testing-servers)
 - [Automated Testing](#automated-testing)
 - [Evaluation](#evaluation)
 - [Submission](#submission)


# Overview

This lab involves a game between two parties: the *client* and the *server*.
The server runs on a CS lab machine, awaiting incoming communications.
The client, also running on a CS lab machine, initiates communications with the
server, requesting the first chunk of the treasure, as well as directions to
get the next chunk.  The client and server continue this pattern of requesting
direction and following direction, until the full treasure has been received.
Your job is to write the client.


## Initial Request

The very first message that the client sends should be exactly 8 bytes long and
have the following format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th><th>04</th><th>05</th><th>06</th><th>07</th></tr>
<tr>
<td colspan="1">0</td>
<td colspan="1">Level</td>
<td colspan="4">User ID</td>
<td colspan="2">Seed</td></tr>
</table>

 - Byte 0: 0
 - Byte 1: an integer 0 through 4, corresponding to the *level* of the
   course.
 - Bytes 2 - 5: a `unsigned int` corresponding to the user ID of the user in
   network byte order (i.e., big-endian, most significant bytes first).  This
   can be retrieved with `id -u` on one of the CS lab machines:
   ```
   $ id -u
   ```
 - Bytes 6 - 7: an `unsigned short` used, along with the user ID, to seed the
   pseudo-random number generator used by the server, in network byte order.

   This is used to allow the client to experience consistent behavior every
   time they interact with the server, to help with development and
   troubleshooting.


## Directions Response

Responses from the server are of variable length (but any given message will
consistent of fewer than 64 bytes) and will follow this format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>::</th><th>n</th><th>n + 1</th><th>n + 2</th><th>n + 3</th><th>n + 4</th>
<th>n + 5</th><th>n + 6</th><th>n + 7</th></tr>
<tr>
<td colspan="1">Chunk Length</td>
<td colspan="3">Chunk</td>
<td colspan="1">Op Code</td>
<td colspan="2">Op Param</td>
<td colspan="4">Nonce</td></tr>
</table>

 - Byte 0: an `unsigned char`.
   - If 0: the hunt is over.  All chunks of the treasure have been received in
     previous messages from the server.
   - If between 1 and 127: A chunk of the message, having length corresponding
     to the value of byte 0, immediately follows, beginning with byte 1.
   - If greater than 127: The server detected an error and is alerting the
     client of the problem:
     - 129: The message was sent from an unexpected address or port (i.e., the
       source address or port of the packet received by the server).
     - 130: The message had an incorrect length.
     - 131: The value of the nonce was incorrect.
     - 133: After multiple tries the server was unable to bind properly to the
       address and port that it had attempted.
   Note that in the case where byte 0 has value 0 or a value greater than 127,
   the entire message will only be one byte long.
 - Bytes 1 - `n` (where `n` matches the value of byte 0; only applies where `n`
   is between 1 and 127): The chunk of treasure that comes immediately after
   the one received most recently.
 - Byte `n + 1`: This is the op-code, i.e., the "directions" to follow to get
   the next chunk of treasure and the next nonce.  It will be one of the
   following:
   - 0: Do not change anything; just send back the nonce value plus 1.
   - 1: Use the port specified by the next two bytes (`n + 2` and `n + 3`),
     which is an `unsigned short` in network byte order, as the new remote
     port.  The result is that future communications *to* the server will be
     directed *to* this port (i.e., the *destination*) port), and future
     communications *from* the server will come *from* this port (i.e., the
     *source* port).

     Here you may either use `connect()` or simply update the remote port you
     you are sending to with `sendto()`.  You *may* use `getaddrinfo()`, but
     you *should not* create a new socket (i.e., with `socket()`), only update
     the existing one.  See
     [Socket Setup and Manipulation](#socket-setup-and-manipulation)).
   - 2: Bind (i.e., using `bind()`) to the local port specified by the next two
     bytes (`n + 2` and `n + 3`), which is an `unsigned short` in network
     byte order.  The result is that future communications *to* the server will
     come *from* this port (i.e., the *source* port), and future
     communications *from* the server will be directed *to* that port (i.e.,
     the *destination* port).

     Here you *must* create a new socket with `socket()`.  You *may* use
     `getaddrinfo()`, but it is not required.  Make sure you close the old
     socket!  If you are using `connect()`, call `bind()` *before* calling
     `connect()`. (See also
     [Socket Setup and Manipulation](#socket-setup-and-manipulation)).
   - 3: Read `m` datagrams from the socket (i.e., using the currently
     established local and remote ports), where `m` is specified by the next
     two bytes (`n + 2` and `n + 3`), which is an `unsigned short` in
     network byte order.  While `m` takes up two bytes for consistency with the
     other op-codes, its value will be only between 1 and 7.  Each of these
     datagrams will come from a randomly-selected remote port on the server, so
     `recvfrom()` must be used by the client to read them to determine which
     port they came from.

     Each of the `m` datagrams received will have 0 length.  However, the
     contents of the datagrams are not what is important; what is important is
     the remote ports from which they originated.  The remote ports of the `m`
     datagrams should be added together, and their sum is the nonce, whose
     value (plus 1) should be returned with the next communication to the
     server.  Note that the sum of these values might well sum to something
     that exceeds the 16 bits associated with an `unsigned short` (16 bits), so
     you will want to store the sum with an `unsigned int` (32 bits).

     Note: if you have called `connect()` on your socket (as opposed to
     using `sendto()`), you *must* create a new socket with `socket()` and
     `bind()` to the local port that was previously used.  You *may* use
     `getaddrinfo()`, but it is not required.  In this case (`connect()`), your
     code will look something like this:

     - Call `close()` on the old socket (this must be done first, or you will
       not be able to successfully bind to the same local port with the new
       socket).
     - Create the new socket.
     - Bind the new socket to the local port that was used previously.
     - Call `recvfrom()` `m` times.
     - Compute the nonce by adding the remote ports from which the `m`
       datagrams were received.
     - Call `connect()` on the new socket to set the remote address and port,
       for future communications (this must be done *after* `recvfrom()`, or
       you will not be able to receive datagrams from arbitrary remote ports).
     - Send the directions request with the nonce value + 1.

     (See [Socket Setup and Manipulation](#socket-setup-and-manipulation) for
     more).

   - 4: Switch address families from using IPv4 (`AF_INET`) to IPv6
     (`AF_INET6`) or vice-versa, and use the port specified by the next two
     bytes (`n + 2` and `n + 3`), which is an `unsigned short` in network byte
     order, as the new remote port (i.e., like op-code 1).  Future
     communications to and from the server will now use the new address family
     and the new remote port.

     Here you *must* call `getaddrinfo()`, and you *must* create a new socket
     with `socket()`.  That is because a socket is only associated with a given
     address family.
     [Socket Setup and Manipulation](#socket-setup-and-manipulation)).

     Note that switching address families for a socket will slightly affect the
     code you created to handle the other op-codes.  That is because the data
     structure you are using to hold IP addresses for `connect()`, `bind()`,
     and `recvfrom()` is different for each address family (`sockaddr_in` vs
     `struct sockaddr_in6`) because the address length for each family is
     unique--4 bytes vs. 16 bytes, respectively.
 - Bytes `n + 2` - `n + 3`: These bytes, an `unsigned short` in network byte
   order is the parameter used in conjunction with the op-code.  This is
   included in all messages, for consistency, but it is only used for op-codes
   1 through 4; for op-code 0, it exists but can be ignored.
 - Bytes `n + 4` - `n + 7`: These bytes, an `unsigned int` in network byte
   order, is the nonce, whose value, plus 1, should be returned in every
   communication back to the server.  In the case of op-code 3, this field is
   ignored.


## Directions Request

After the initial request, every subsequent request will be exactly four bytes
and will have the following format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th></tr>
<tr>
<td colspan="4">Nonce + 1</td></tr>
</table>

 - Bytes 0 - 3: an `unsigned int` having a value of one more than the nonce
   most recently sent by the server, in network byte order.  For example, if
   the server previously sent, 100, then this value would be 101.


## Levels

The level sent to the server is one of the following:

 - 0: Responses from the server will only use op-code 0.  Level 0 is to get
   practice exchanging datagrams with the server, and with each exchange: 1)
   extracting the chunk of treasure that is included in the server's response;
   and 2) extracting nonce and returning the nonce + 1.
 - 1: Responses from the server will only use op-code 1.  The client should be
   expected to do everything it did at level 0, but also to extract the remote
   port from each directions response and use `connect()` or `sendto()` to use
   it with each outgoing message (i.e., op-code 1).
 - 2: Responses from the server will select from op-codes 1 and 2 at random.
   That is, the client should be expected to do everything it did at level 1,
   but also to extract the new local port from each directions response and use
   it for all for future outgoing communications.  In this case, the old socket
   should be closed, and a new socket created, which is bound (i.e., by calling
   `bind()` to the new local port.
 - 3: Responses from the server will select from op-codes 1 through 3 at
   random.  That is, the client should be expected to do everything it did
   at level 2, but also to extract the number of datagrams to immediately
   receive from the server, as well as receive and sum the payloads of those
   datagrams, in order to get the nonce.
 - 4: Responses from the server will select from op-codes 1 through 4 at
   random.  That is, the client should be expected to do everything it did at
   level 3, but also to switch to IPv4 or IPv6, from whichever it was using
   before.


## Message Formatting

When writing to or reading from a socket, a buffer must be specified.  This can
be a pointer to data at any memory location, but the most versatile data
structure is an array of `unsigned char`.

For example, in preparing the initial directions request to the server, a
buffer might be declared like this:

```c
unsigned char buf[64];
```

If that initial message has values 1, 3985678983 (0xed90a287), and 7719
(or, equivalently, 0x1e27) for level, user ID, and seed, respectively, the
values stored in that buffer might look like this, after it is populated:

```c
buf = { 0, 1, 0xed, 0x90, 0xa2, 0x87, 0x1e, 0x27 };
```

Of course, you cannot simply populate the array with the above code because the
values will not be known ahead of time.  You will need to assign them.  So how
do you fit an `unsigned short` (16 bits) into multiple slots of `unsigned
char` (8 bits)?  There are several ways.  Consider the following program:

```c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFSIZE 8

int main() {
	unsigned char buf[BUFSIZE];
	unsigned short val = 0xabcd;
	int i = 0;
	bzero(buf, BUFSIZE);

	memcpy(&buf[6], &val, 2);
	for (i = 0; i < BUFSIZE; i++) {
		printf("%x ", buf[i]);
	}
	printf("\n");
}
```

When you compile and run this program, you will see that indexes 6 and 7 of
`buf` contain the value of `val`, exactly as they are stored in memory; that is
what the call to `memcpy()` is doing.  You will *most likely* find them to be
in the *reverse* order from what you might expect.  If so, is because the
architecture that you are using is *little endian*.  This is problematic for
network communications, which expect integers to be in *network* byte order
(i.e., *big endian*).  To remedy this, there are functions provided for you by
the system, including `htons()` and `ntohs()` ("host to network short" and
"network to host short").  See their man pages for more information.  Try
modifying the code above to assign `htons(0xabcd)` to `val` instead, and see
how the output changes.

The example above is specific to storing an `unsigned short` integer value into
an arbitrary memory location (in this case an array of `unsigned char`) in
network byte order.  You will need to use this principle to figure out how to
do similar conversions for other cirumstances, including working with integers
other than `unsigned short` and extracting integers of various lengths from
arrays of `unsigned char`.  Hint: the man page for `ntohs()` for related
functions.


## Socket Setup and Manipulation


### UDP Socket Behaviors

For this lab, all communications between client and server are over UDP (type
`SOCK_DGRAM`).  As such, the following are tips for socket creation and
manipulation:

 - Sending every message requires exactly one call to `write()`, `send()`, or
   `sendto()`.  See the man page for `udp`.
 - Receiving every message requires exactly one call to `read()`, `recv()`, or
   `recvfrom()`.  In some cases (e.g., op-code 3) `recvfrom()` *must* be used.
   See the man page for `udp`.
 - When 0 is returned by a call to `read()` or `recv()` on a socket of type
   `SOCK_DGRAM`, it simply means that there was no data/payload in the datagram
   (i.e., it was an "empty envelope").  See "RETURN VALUE" in the `recv()` man
   page.

   Note that this is different than the behavior associated with a socket of
   type `SOCK_STREAM`, in which if `read()` or `recv()` returns 0, it is a
   signal that `close()` has been called on the remote socket, and the
   connection has been shut down.  With UDP (type `SOCK_DGRAM`), there is no
   connection to be shutdown.
 - Either `connect()` must be used to associate a remote address and port with
   the socket, or `sendto()` must be used when sending messages.
 - `sendto()` can be used to override the remote address and port associated
   with the socket.  See the man page for `udp`.


### Using `bind()` with Sockets

The following tips associated with `bind()` are not specific to UDP sockets
(type `SOCK_DGRAM`) but are nonetheless useful for this lab:

 - The local address and port can be associated with a socket using `bind()`.
   See the man pages for `udp` and `bind()`.
 - `bind()` can only be called *once* on a socket.  See the man page for
   `bind()`.
 - If the client is told to use a new local port, then the current socket must
   be closed, and a new one must be created, so that `bind()` can be called.

   Even if `bind()` has *not* been called on a socket, if a local address and
   port have been associated with the socket implicitly (i.e., when `write()`,
   `send()`, or `sendto()` is called on that socket), `bind()` cannot be called
   on that socket.


### Using Different Address Families with Sockets

The following are useful tips related to address families:

 - If the client is told to use a new address family, then the current socket
   must be closed, and a new one must be created with the new address family.

   A socket can be associated with only one address family.  For this lab, it
   will be either `AF_INET` (IPv4) or `AF_INET6` (IPv6).  See the man page for
   `socket()`.
 - When using `getaddrinfo()` to create your socket a socket for IPv4 or IPv6
   use, use the `ai_family` member of the `struct addrinfo` variable passed as
   the `hints` argument to `getaddrinfo()`.  For IPv4:
   ```c
   	hints.ai_family = AF_INET;
   ```
   For IPv6:
   ```c
   	hints.ai_family = AF_INET6;
   ```
   See the man page for `getaddrinfo()`.
 - The initial communication from the client *must* be over IPv4.


### Address Structures

The data structures used for holding local or remote address and port
information are defined as follows (see the man pages for `ip(7)` and
`ipv6(7)`, respectively.

For IPv4 (`AF_INET`):
```c
           struct sockaddr_in {
               sa_family_t    sin_family; /* address family: AF_INET */
               in_port_t      sin_port;   /* port in network byte order */
               struct in_addr sin_addr;   /* internet address */
           };

           /* Internet address. */
           struct in_addr {
               uint32_t       s_addr;     /* address in network byte order */
           };
```

For IPv6 (`AF_INET6`):
```c
           struct sockaddr_in6 {
               sa_family_t     sin6_family;   /* AF_INET6 */
               in_port_t       sin6_port;     /* port number */
               uint32_t        sin6_flowinfo; /* IPv6 flow information */
               struct in6_addr sin6_addr;     /* IPv6 address */
               uint32_t        sin6_scope_id; /* Scope ID (new in 2.4) */
           };

           struct in6_addr {
               unsigned char   s6_addr[16];   /* IPv6 address */
           };
```

When using `getaddrinfo()` most of this is masked for you, and you can simply
create a socket and call `bind()` or `connect()` on it with the members of the
structures (`struct addrinfo`) that comprise the links in the linked list
created by `getaddrinfo()`.  However, in some cases `getaddrinfo()` is more
than is needed.

Here are some examples of populating and using those data structures manually,
without the help of `getaddrinfo()`.

```c
	struct sockaddr_in ipv4addr;

	int sfd;
	unsigned short port;

	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < -1) {
		perror("socket()");
	}
	...

	ipv4addr.sin_family = AF_INET; // use AF_INET (IPv4)
	ipv4addr.sin_port = htons(port); // specific port
	ipv4addr.sin_addr.s_addr = 0; // any/all local addresses

	if (bind(sfd, (struct sockaddr *)&ipv4addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind()");
	}
```
Note that both `bind()` and `connect()` take type `struct sockaddr *` as the
second argument, allowing it to accept either `struct sockaddr_in` or
`struct sockaddr_in6`, with the caveats that 1) the argument is *cast* as a
`struct sockaddr *` and 2) the length is specified as the last argument, since
the lengths of the two structures are different, and `bind()` would not
otherwise know the difference.  `sendto()` behaviors similarly.

Also note that the `sin_port` of the `struct sockaddr_in` member contains the
port in *network* byte ordering
(See [Message Formatting](#message-formatting)).

The same for IPv6:
```c
	struct sockaddr_in6 ipv6addr;

	int sfd;
	unsigned short port;

	if ((sfd = socket(AF_INET6, SOCK_DGRAM, 0)) < -1) {
		perror("socket()");
	}

	...

	ipv6addr.sin6_family = AF_INET6; // IPv6 (AF_INET6)
	ipv6addr.sin6_port = htons(port); // specific port
	bzero(ipv6addr.sin6_addr.s6_addr, 16); // any/all local addresses

	if (bind(sfd, (struct sockaddr *)&ipv6addr, sizeof(struct sockaddr_in6)) < 0) {
		perror("bind()");
	}
```

Because local and remote addresses and ports are stored in a
`struct sockaddr_in` or `struct sockaddr_in6`, one way that you might keep
always track of your address family, as well as local and remote ports, is by
declaring the following:

```c
	int af;
	struct sockaddr_in ipv4addr_local;
	struct sockaddr_in ipv4addr_remote;
	struct sockaddr_in6 ipv6addr_local;
	struct sockaddr_in6 ipv6addr_remote;
```

and maintaining them along the way.  You can initialize these structures with
value populdated by `getaddrinfo()` and `getsockname()` using something like
this:

```c
	// populate ipv4addr_remote or ipv6addr_remote with address information
        // found in the struct addrinfo from getaddrinfo()
	af = rp->ai_family;
        if (af == AF_INET) {
		ipv4addr_remote = *(struct sockaddr_in *)rp->ai_addr;
	} else {
		ipv6addr_remote = *(struct sockaddr_in6 *)rp->ai_addr;
	}
```
and
```c
	// populate ipv4addr_local or ipv6addr_local with address information
        // associated with sfd using getsockname()
	af = rp->ai_family;
	socklen_t addrlen;
        if (af == AF_INET) {
		addrlen = sizeof(struct sockaddr_in);
		getsockname(sfd, (struct sockaddr *)&ipv4addr_local, &addrlen);
	} else {
		addrlen = sizeof(struct sockaddr_in6);
		getsockname(sfd, (struct sockaddr *)&ipv6addr_local, &addrlen);
	}
```

So you can later run something like this:

```c
	if (af == AF_INET) {
		if (connect(sfd, (struct sockaddr *)&ipv4addr_remote, sizeof(struct sockaddr_in)) < 0) {
			perror("connect()");
		}
	} else {
		if (connect(sfd, (struct sockaddr *)&ipv6addr_remote, sizeof(struct sockaddr_in6)) < 0) {
			perror("connect()");
		}
	}
```
or
```c
	if (af == AF_INET) {
		ipv4addr_local.sin_family = AF_INET; // use AF_INET (IPv4)
		ipv4addr_local.sin_port = htons(port); // specific port
		ipv4addr_local.sin_addr.s_addr = 0; // any/all local addresses
		if (bind(sfd, (struct sockaddr *)&ipv4addr_local, sizeof(struct sockaddr_in)) < 0) {
			perror("bind()");
		}
	} else {
		ipv6addr_local.sin6_family = AF_INET6; // IPv6 (AF_INET6)
		ipv6addr_local.sin6_port = htons(port); // specific port
		bzero(ipv6addr_local.sin6_addr.s6_addr, 16); // any/all local addresses
		if (bind(sfd, (struct sockaddr *)&ipv6addr_local, sizeof(struct sockaddr_in6)) < 0) {
			perror("bind()");
		}
	}

```


## Usage

Your program should have the following usage:

```
$ ./treasure_hunter server port level seed
```

 - `server`: the domain name of the server.
 - `port`: the port on which the server is expecting initial communications.
 - `level`: the level to follow, a value between 0 and 4.
 - `seed`: a seed used to initialize the pseudo-random number generator on the
   server.


## Output

### Treasure - standard output

Once the client has collected all of the treasure chunks, it should print the
entire treasure to standard output, followed by a newline.  For example, if the
treasure hunt yielded the following chunks:

 - `abc`
 - `de`
 - `fghij`

Then the client would print:

```
abcdefghij
```

No treasure will be longer than 1,024 characters, so you may use that as your
buffer size.  Remember to ensure that the characters comprising your treasure
end with a null byte, so they can be used with `printf()`.


### Socket Information - standard error (Extra Credit)

This output is _optional_ but might be helpful and/or interesting to you.  You
will also get extra credit.

Note: you will find working code examples for this section and others in the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets).

Every time the client sends a message using `write()` or `send()`, it should
print the source address, source port, destination address, and destination
port to standard error, such as in the following example:

```
192.0.2.1:1234 -> 192.0.2.2:4567
```

or, for IPv6:

```
2001:db8::1:1234 -> 2001:db8::2:4567
```

Thus, if your client sent 10 messages to the server over the course of the
game, there would be 10 lines printed to standard error, each resembling the
lines above, but each slightly different from the other (because source and
destination ports will be changing).

You will find the `getsockname()`, `getpeername()`, and `getnameinfo()`
functions useful for this.  While `getnameinfo()` *can* convert an IP address
to a domain name (using the DNS), in this case, you will be using it to simply
format the IP address and port properly as strings, so you can print them out.
Thus, the `NI_NUMERICSERV` and `NI_NUMERICHOST` options will be useful.

Note: using `getpeername()` only makes sense if the remote address and port
have been explicitly set with `connect()`.  In the case that your code does
*not* use `connect()`, you can pass the same `struct sockaddr_storage`
structure to `getnameinfo()` as you did to `sendto()`.


# Preparation

## Reading

Read the following in preparation for this assignment:
  - The man pages for the following:
    - `udp`
    - `ip`
    - `ipv6`
    - `socket`
    - `socket()`
    - `send()`
    - `recv()`
    - `bind()`
    - `connect()`
    - `getaddrinfo()`
    - `htons()`
    - `ntohs()`
    - `getpeername()`
    - `getsockname()`
    - `getnameinfo()`


# Instructions

## Getting Started

First, open `treasure_hunter.c` and look around.  You will note that there are
two functions and one `#define` statement.

Now, replace `PUT_USERID_HERE` with your numerical user ID, which you can find
by running `id -u` on a CS lab machine.  From now on you can use `USERID` as an
integer wherever you need to use your user ID in the code.

Now take a look the `print_bytes()` function.  This function was created to
help you see what is in a given message that is about to be sent or has just
been received.  It is called by providing a pointer to a memory location, e.g.,
an array of `unsigned char`, and a length.  It then prints the hexadecimal
value for each byte, as well as the ASCII character equivalent for values less
than 128 (see `man ascii`).

Use the specification for the [initial request](#initial-request) and
[message formatting helps](#message-formatting) to create an initial request in
an array of `unsigned char`.  At this point, hard-code 0 for the level,
`USERID` for the user ID, and 7719 (or, equivalently, 0x1e27) for the seed.
Note: you can find the hexadecimal representation of your user ID (or any
integer) by running the following:

```
$ printf "%08x\n" 1234
```

substituting `1234` with the actual integer you wish you represent as
hexadecimal.

Now call `print_bytes()`, specifying the message buffer that you have populated
as the first argument and 8 as your second argument (i.e., because it should be
an 8-byte message).

Build your file with the following:

```bash
$ make
```

Then run it:

```bash
$ ./treasure_hunter
```

Check your message that it looks correct.  Bytes 0 and 1 should both be zero.
Bytes 2 through 5 should have the value of your user ID in network byte order.
Bytes 6 and 7 should have the value of your seed in network byte order.  If
everything looks good, move on!

Of course, you have not sent or received any messages at this point, but you
now know how to *format* a message appropriately.


## Initial Send and Receive

Note: you will find working code examples for this section and others in the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets).

With your first message created, set up a UDP client socket, with
`getaddrinfo()`, `socket()`, and (optionally) `connect()`, specifying
`AF_INET` and `SOCK_DGRAM`
(see [Socket Setup and Manipulation](#socket-setup-and-manipulation)), and
using one of the [testing servers](#testing-servers) as the remote server.

When everything is set up, send your message with `send()` (or `sendto()`, and
read the response with `recv()` (remember, it is just one call to each!).  Use
`print_bytes()` to print out the message contents.  Make sure you understand
what you are seeing, specifically that you can understand the
[response](#directions-response).


## Collect and Print the Level 0 Treasure

Save the chunk of treasure, and extract the nonce from the
[response](#directions-response) using the
[message formatting helps](#message-formatting), and build the next
[directions request](#directions-request) using the nonce.  Then use
`print_bytes()` to show that looks as you would expect.  When it looks right,
add another `send()` and `recv()` to your code to request the next chunk.

Now generalize this pattern to pull down the entire treasure, one chunk at a
time, appending to the buffer with the chunk sent in each datagram.
You will want to create a loop with the appropriate termination test indicating
that the entire treasure has been received (see the
[specification](#directions-response)).

At this point, print out the message associated with the treasure,
[as specified](#treasure---standard-output).


## Print the Socket Information (Extra Credit)

This section is _optional_.  It might be helpful or interesting for you to see
where packets are being sent.  You will also get extra credit.

Now follow the [specification](#socket-information---standard-error-extra-credit) to produce
the output showing the socket information before every *outgoing* communication
(i.e., calls to `write()`, `send()`, and/or `sendto()`).  The output will be
rather boring for level 0, but getting the output working for level 0 will
really help for subsequent levels.

At this point, the output should consist of bunch of lines that look exactly
the same.  For example:

```
192.0.2.1:1234 -> 192.0.2.2:32400
192.0.2.1:1234 -> 192.0.2.2:32400
192.0.2.1:1234 -> 192.0.2.2:32400
```

(where 192.0.2.1 is the local IP address and 192.0.2.2 is the remote IP
address.)

For level 0, the remote port will always be 32400, the remote address will be
that of the server, the local address will be that of the client, and the
remote port will be randomly assigned by the kernel--unless you have decided
to set it explicitly with `bind()` (which is not necessary at this point).

To learn the actual IP addresses of the local and remote systems, to check your
work, run the following commands:

```bash
$ dig +short -t A server client
```

(In each case, replace `client` with the domain name of the client and `server`
with the domain name of the server.)

To the find the IPv6 addresses (e.g., for level 4), use:

```bash
$ dig +short -t AAAA server client
```


## Generalize the Inputs

At this point, the initial directions request has mostly been formed from
hard-coded values, for a proof of concept.  Now we want to make them more
general.  If you have hard-coded the level, server, port, and/or seed (as
guided earlier), modify your code to accept them from the command line.  When
you have finished, make sure your code works just as well as it did before you
began generalizing the code when you run the following:

```bash
$ ./treasure_hunter server 32400 0 7719
```

(replace `server` with the domain name of the server you are using.)

Try running it with each of the following seeds:

 - 7719 (same as before)
 - 33833
 - 20468
 - 19789
 - 59455


## Remove Any Extra Print Statements

While print statements are useful for debugging, at this point remove them,
comment them out, or otherwise take them out of the code flow (e.g., with `if
(verbose)`), so your output consists of only the
[socket information](#socket-information---standard-error)
and [the treasure](#treasure---standard-output).


## Checkpoint 0

At this point, you can also test your work with
[automated testing](#automated-testing).  Level 0 should work at this point.

Now would be a good time to save your work, if you haven't already.


## Future Levels and Checkpoints

At this point, work through each of
[levels 1 through 4](#levels) by implementing the
directions given by [op-codes 1 through 4](#directions-response).  After
implementing the op-code for each level, you should be able to pass the
corresponding level with both manual and [automated testing](#automated-testing).

Use the information from the
[Socket Setup and Manipulation](#socket-setup-and-manipulation) section,
as well as code from the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets)
to complete each level.

Consider the completion of each level a checkpoint.

Note that when you implement level 4, you will need to make sure that your code
for the previous levels supports both IPv4 (`AF_INET` and IPv6 `AF_INET6`).
See the
[Socket Setup and Manipulation](#socket-setup-and-manipulation) section.


# Testing Servers

The following domain names and ports correspond to the servers where the games
might be initiated:

 - rome.cs.byu.edu:32400
 - qatar.cs.byu.edu:32400
 - utah.cs.byu.edu:32400
 - redwood.cs.byu.edu:32400

Note that communicating with any server should result the same behavior.  But
for the purposes of load balancing, please run the following commands from one
of the CS lab machines to select the *primary* machine that you should use:

```bash
$ server=(rome qatar utah redwood)
$ val=$(( 0x`echo $USER | sha1sum | cut -c4` % 4))
$ echo ${server[$val]}
```

This effectively selects a server based on your username.


# Automated Testing

For your convenience, a script is also provided for automated testing.  This is
not a replacement for manual testing but can be used as a sanity check.  You
can use it by simply running the following:

```
./driver.py server port [level]
```

Replace `server` and `port` with a server and port from the set of
[servers designated for testing](#testing-servers) (i.e., preferably the one
corresponding to your username).

Specifying `level` is optional.  If specified, then it will test
[all seeds](#evaluation) against a given level.  If not specified, it will test
_all_ levels.


# Evaluation

Your score will be computed out of a maximum of 100 points based on the
following distribution:

 - 20 points for each of 5 levels (0 through 4)
 - For each level, 4 points for each seed:
   - 7719
   - 33833
   - 20468
   - 19789
   - 59455


# Submission

Upload `treasure_hunter.c` to the assignment page on LearningSuite.
