# HTTP Proxy Lab - I/O Multiplexing

The purpose of this assignment is to help you become more familiar with the
concepts associated with client and server sockets that are non-blocking, as
part of the I/O multiplexing concurrency paradigm.  You will learn these
concepts by building a working HTTP proxy server that uses epoll.


# Table of Contents


 - [Overview](#overview)
   - [Non-Blocking I/O](#non-blocking-io)
   - [Client Request States](#client-request-states)
   - [Client Request Data](#client-request-data)
 - [Preparation](#preparation)
   - [Reading](#reading)
   - [epoll Echo Server Example](#epoll-echo-server-example)
 - [Instructions](#instructions)
   - [Part 1 - HTTP Request Parsing](#part-1---http-request-parsing)
   - [Part 2 - I/O Multiplexing HTTP Proxy](#part-2---io-multiplexing-http-proxy)
     - [Handling a New HTTP Client](#handling-a-new-http-client)
     - [Receiving the HTTP Request](#receiving-the-http-request)
     - [Creating an HTTP Request](#creating-an-http-request)
     - [Communicating with the HTTP Server](#communicating-with-the-http-server)
     - [Returning the HTTP Response](#returning-the-http-response)
     - [Testing](#testing)
 - [Testing](#testing-1)
   - [Manual Testing - Non-Local Server](#manual-testing---non-local-server)
   - [Manual Testing - Local Server](#manual-testing---local-server)
   - [Automated Testing](#automated-testing)
 - [Evaluation](#evaluation)
 - [Submission](#submission)


# Overview

In this lab, you will be implementing an HTTP proxy server that handles
concurrent requests.  However, unlike the proxy server implemented in the
[HTTP Proxy with Threadpool Lab](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool),
the proxy server you produce will achieve concurrency using I/O multiplexing.
Your server will not spawn any additional threads or processes (i.e., it will
be single-threaded), and all sockets will be set to non-blocking.  While your
server will not take advantage of multiprocessing, it will be more efficient by
holding the processor longer because it is not blocking (and thus sleeping) on
I/O.  This model is also referred to as an example of *event-based* programming,
wherein execution of code is dependent on "events"--in this case the
availability of I/O.


## Non-Blocking I/O

All sockets that your proxy will use should be set up for non-blocking I/O.
This includes the listen socket, the sockets associated with communications
between client and proxy, and the sockets associated with communications
between proxy and server.

Additionally, all sockets must be registered with the epoll instance, for
reading or writing, using edge-triggered monitoring.

That being said, for simplicity, you may wait to set the proxy-to-server socket
as non-blocking *after* you call `connect()`, rather than before.  While that
will mean that your server not fully non-blocking, it will allow you to focus
on the more important parts of I/O multiplexing.  This is permissible.

If you instead choose to set the socket as non-blocking before calling
`connect()` (this is not required), you can execute `connect()` immediately,
but you cannot initiate the `write()` call until `epoll_wait()` indicates that
this socket is ready for writing. Because the socket is non-blocking,
`connect()` will return before the connection is actually established.  In this
case, the return value is -1 and `errno` is set to `EINPROGRESS` (see the
`connect()` man page).  This also means that when iterating through the results
of `getaddrinfo()` when a socket is non-blocking, the return value of
`connect()` is not a useful check for determining whether a given address is
reachable.


## Client Request States

A server that uses I/O multiplexing will handle multiple clients concurrently
using only a single thread.  That means that it only acts on a given client
when there is I/O associated with that client.  But because handling a proxy
client involves various tasks (e.g., receive request from client, send request
to server, etc.), it is helpful to think about the problem in terms of
"states" and events that trigger transitions between these states.  The
following is an example of a set of client request states, each associated with
different I/O operations related to proxy server operation:


### `READ_REQUEST`

This is the start state for every new client request.  You should initialize
every new client request to be in this state.

In this state, read from the client socket in a loop until one of the following
happens:

 - you have read the entire HTTP request from the client.  If this is the case:
   - parse the client request and create the request that you will send to the
     server.
   - create a new socket and connect to the HTTP server.
   - configure the new socket as non-blocking.
   - register the socket with the epoll instance for writing.
   - change state to `SEND_REQUEST`.
 - `read()` (or `recv()`) returns a value less than 0.
   - If `errno` is `EAGAIN` or `EWOULDBLOCK`, it just means that there is no
     more data ready to be read; you will continue reading from the socket when
     you are notified by epoll that there is more data to be read.
   - If `errno` is anything else, this is an error.  You can print out the
     error, cancel your client request, and deregister your socket at this
     point.


### `SEND_REQUEST`

You reach this state only after the entire request has been received from the
client and the connection to the server has been initiated (i.e., in the
`READ_REQUEST` state).

In this state, loop to write the request to the server socket until one of the
following happens:

 - you have written the entire HTTP request to the server socket.  If this is
   the case:
   - register the socket with the epoll instance for reading.
   - change state to `READ_RESPONSE`.
 - `write()` (or `send()`) returns a value less than 0.
   - If and `errno` is `EAGAIN` or `EWOULDBLOCK`, it just means that there is
     no buffer space available for writing to the socket; you will continue
     writing to the socket when you are notified by epoll that there is more
     buffer space available for writing.
   - If `errno` is anything else, this is an error.  You can print out the
     error, cancel your client request, and deregister your socket at this
     point.


### `READ_RESPONSE`

You reach this state only after you have sent the entire HTTP request (i.e., in
the `SEND_REQUEST` state) to the Web server.

In this state, loop to read from the server socket until one of the following
happens:

 - you have read the entire HTTP response from the server.  Since this is
   HTTP/1.0, this is when the call to `read()` (or `recv()`) returns 0,
   indicating that the server has closed the connection.  If this is the case:
   - register the client socket with the epoll instance for writing.
   - change state to `SEND_RESPONSE`.
 - `read()` (or `recv()`) returns a value less than 0.
   - If `errno` is `EAGAIN` or `EWOULDBLOCK`, it just means that there is no
     more data ready to be read; you will continue reading from the socket when
     you are notified by epoll that there is more data to be read.
   - If `errno` is anything else, this is an error.  You can print out the
     error, cancel your client request, and deregister your socket at this
     point.


### `SEND_RESPONSE`

You reach this state only after you have received the entire response from the
Web server (i.e., in the `READ_RESPONSE` state).

In this state, loop to write to the client socket until one of the following
happens:

 - you have written the entire HTTP response to the client socket.  If this is
   the case:
   - close your client socket.  You are done!
 - `write()` (or `send()`) returns a value less than 0.
   - If and `errno` is `EAGAIN` or `EWOULDBLOCK`, it just means that there is
     no buffer space available for writing to the socket; you will continue
     writing to the socket when you are notified by epoll that there is more
     buffer space available for writing.
   - If `errno` is anything else, this is an error.  You can print out the
     error, cancel your client request, and deregister your socket at this
     point.


## Client Request Data

You will need to keep track of the data associated with each request.  The
reason is that, just like when using blocking sockets, you won't always be able
to receive or send all your data with a single call to `read()` or `write()`.
With blocking sockets in a multi-threaded server, the solution was to use a
loop that received or sent until you had everything, before you moved on to
anything else.  Because the sockets were configured as blocking, the kernel
would context switch out the thread and put it into a sleep state until there
was I/O.

However, with I/O multiplexing and non-blocking I/O, you can't loop until you
receive (or send) everything; you have to stop when you get an value less than
0 and move on to handling the other ready events, after which you will return
to the `epoll_wait()` loop to see if it is ready for more I/O.  When a return
value to `read()` or `write()` is less than 0 and `errno` is `EAGAIN` or
`EWOULDBLOCK`, it is a an indicator that you are done for the moment--but you
need to know where you should start next time it's your turn (see man pages for
`accept()` and `read()`, and search for "blocking").  For example, you should
associate the following with each client request.

 - the socket corresponding to the requesting client
 - the socket corresponding to the connection to the Web server
 - the current state of the request (see [Client Request States](#client-request-states)).
 - the buffer(s) to read into and write from
 - the total number of bytes read from the client
 - the total number of bytes to write to the server
 - the total number of bytes written to the server
 - the total number of bytes read from the server
 - the total number of bytes written to the client

You might like to define a `struct request_info` (for example) that contains
each of these members.


# Preparation

## Reading

Read the following in preparation for this assignment:
  - Sections 11.1 - 11.6, 12.1 - 12.2 in the book
  - The man pages for the following:
    - `epoll` - general overview of epoll, including detailed examples
    - `epoll_create1` - shows the usage of the simple function to create an
      epoll instance
    - `epoll_ctl` - shows the definition of the `epoll_data_t` and
      `struct epoll_event` structures, which are used by both `epoll_ctl()` and
      `epoll_wait()`.  Also describes the event types with which events are
      registered to an epoll instance, e.g., for reading or writing, and which
      type of triggering is used (for this lab you will use edge-triggered
      monitoring).
    - `epoll_wait` - shows the usage of the simple `epoll_wait()` function,
      including how events are returned and how errors are indicated,
    - `fnctl()`
    - `socket`
    - `socket()`
    - `send()`
    - `recv()`
    - `bind()`
    - `connect()`
    - `getaddrinfo()`


## epoll Echo Server Example

The `echoserver-epoll` directory contains a working version of a echo server
using epoll, complete with non-blocking sockets and edge-triggered monitoring.
To compile it, run the following from the `echoserver-epoll` directory:

```bash
$ gcc -o echoservere echoservere.c
```

You can use the code as a guide for building your HTTP proxy server with epoll.
It runs the same way as the echo server implementations from the
[concurrency homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-concurrency).


# Instructions

## Part 1 - HTTP Request Parsing

Follow the instructions for implementing HTTP request parsing from
[HTTP Proxy Lab - Threadpool Lab](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool#part-1---http-request-parsing),
if you haven't already.


## Part 2 - I/O Multiplexing HTTP Proxy

As you implement this section, you might find it helpful to refer to the TCP
code from the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets).


### Handling a New HTTP Client

Write functions for each of the following:

 - `open_sfd()` - Create and configure a TCP socket for listening and accepting
   new client connections.
   - Create a socket with address family `AF_INET` and type
     `SOCK_STREAM`.
   - Use the following command to set an option on the socket to
     allow it bind to an address and port already in use:

     ```c
     int optval = 1;
     setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
     ```

     While this might seem like a bad idea, during development of your proxy
     server, it will allow you to immediately restart your proxy server after
     failure, rather than having to wait for it to time out.
   - Configure the socket to use *non-blocking I/O* (see the man page for
     `fcntl()` for how to do this).
   - `bind()` it to a port passed as the first argument from the
     command line, and configure it for accepting new clients with `listen()`.
   - Return the file descriptor associated with the server socket.
 
 - `handle_new_clients()` - Accept and prepare for communication with incoming
   clients.
   - Loop to `accept()` any and all client connections.  For each new file
     descriptor (i.e., corresponding to a new client) returned, configure it to
     use non-blocking I/O (see the man page for `fcntl()` for how to do this),
     and register each returned client socket with the epoll instance that you
     created for reading, using edge-triggered monitoring (i.e.,
     `EPOLLIN | EPOLLET`).  You should only break out of your loop and stop
     calling `accept()` when it returns a value less than 0, in which case:
     - If `errno` is set to `EAGAIN` or `EWOULDBLOCK`, then that is an
       indicator that there are no more clients currently pending;
     - If `errno` is anything else, this is an error.  It actually be best to
       have your proxy exit at this point.

     Have your proxy print the newly created file descriptor associated with
     any new clients.  You can remove this later, but it will be good for you
     to see now that they are being created.

     You will need to pass your epoll file descriptor as an argument, so you
     can register the new file descriptor with the epoll instance.

Now add the following to `main()`:

 - Create an epoll instance with `epoll_create1()`.
 - Call `open_sfd()` to get your listening socket.
 - Register your listen socket with the epoll instance that you created, for
   *reading* and for edge-triggered monitoring (i.e., `EPOLLIN | EPOLLET`).
 - Create a `while(1)` loop that does the following:
   - Calls `epoll_wait()` loop with a timeout of 1 second.
   - If the result was a timeout (i.e., return value from `epoll_wait()` is 0),
     check if a global flag has been set by a signal handler and, if so, break
     out of the loop; otherwise, continue.
   - If the result was an error (i.e., return value from `epoll_wait()` is less
     than 0), then handle the error appropriately (see the man page for
     `epoll_wait()` for more).
   - If there was no error, you should loop through all the events and handle
     each appropriately.  For now, just start with handling new clients.  We
     will implement the handling of existing clients later.  If the event
     corresponds to the listening file descriptor, then call
     `handle_new_clients()`.
 - After the `epoll_wait()` `while(1)` loop, you should clean up any resources
   (e.g., freeing `malloc()`'d memory), and exit.


At this point, your server is merely set up to listen for incoming client
connections and `accept()` them.  It is not yet doing anything else useful, but
you should be able to test your work so far!

Run the following to build your proxy:

```bash
$ make
```

Now use the following command to find a port that is unlikely to conflict with
that of another user:

```bash
$ ./port-for-user.pl
```

Then run the following to start your proxy server:

```bash
$ ./proxy port
```

Replace `port` with the port returned by `./port-for-user.pl`.

Now, from another terminal on the same machine, run the following:

```bash
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```
(Replace `port` with the port on which your proxy server is listening.)

`curl` is a command-line HTTP client, described more in
[the section on manual testing](#manual-testing---non-local-server).
For the purposes of this section, `curl` creates and sends an HTTP request to
your proxy server, which is designated with `-x`.

Your proxy server should be printing the file descriptors associated with each
of the two connections at this point.  However, you shouldn't expect it to be
doing much else--not just yet anyway.


### Receiving the HTTP Request

Write a function, `handle_client()`, that takes a pointer to a
[client request](#client-request-data), determines what state it is in, and
performs the actions associated with that state (i.e., picks up where it left
off.  See [Client Request States](#client-request-states) for more information.
For now, just implement the `READ_REQUEST` state.

At this point, it might be good to add a debug print statement to show when
your client enters `handle_client()`, as well as the file descriptor associated
with the client and the current state of the client.

Now add some code to your `epoll_wait()` loop that calls `handle_client()` when
an event corresponds to an existing client.

Re-build and re-start your proxy, and make sure it works properly when you run
the following:

```bash
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```
(Replace `port` with the port on which your proxy server is listening.)

The `./slow-client.py` script is also described more in
[the section on manual testing](#manual-testing---non-local-server).
For the purposes of this section, it acts like `curl`, but it spreads its HTTP
request over several calls to `send()`.

In both cases, your proxy server (i.e., in `handle_client()`) should indicate
that it has received the client request by making its way through the
`READ_REQUEST` state to the `SEND_REQUEST`  state.  Note that the `curl`
command will likely result in a client request being in the `READ_REQUEST`
state only once in the `handle_request()` function, but the `slow-client.py`
command should require at least two times through `READ_REQUEST` in the
`handle_request()` function.

At this point, you might also want to print out the HTTP request you have
received, potentially over multiple calls to `handle_client()`, to see that it
was what you expected.


### Creating an HTTP Request

Follow the instructions for creating an HTTP request from
[HTTP Proxy Lab - Threadpool Lab](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool#creating-an-http-request),
if you haven't already.  Your proxy should create this request *after* it has
received the entire HTTP request.


### Communicating with the HTTP Server

Now that your proxy can read an HTTP request from a client and create an HTTP
request to be sent to the server, it is time to send the request, i.e., the
`SEND_REQUEST` state.  In the `handle_client()` function, add the functionality
for the `SEND_REQUEST` and `READ_RESPONSE` states, as specified in the
[Client Request States](#client-request-states).

Now would be a good time to test with the following commands:

```bash
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```

Just as before, you should not only observe that the proxy server successfully
issues the request and receives the response from the HTTP server, but also
that it took several calls to `handle_client()` to receive the entire response.


### Returning the HTTP Response

With the response from the server, all that is left is for your proxy to send
it back to the client.  In the `handle_client()` function, add the
functionality for the `SEND_RESPONSE` state, as specified in the
[Client Request States](#client-request-states).


### Testing

At this point you should be able to pass:
 - [Tests performed against a non-local Web server](#manual-testing---non-local-server).
 - [Tests performed against a local Web server](#manual-testing---local-server).
 - [Automated tests](#automated-testing) with the following command:
   ```bash
   $ ./driver.py -b 20 -c 75 epoll
   ```

# Testing

Some tools are provided for testing--both manual and automated:

 - The code for the `tiny` Web server
 - A driver for automated testing


## Manual Testing - Non-Local Server

See
[Manual Testing - Non-Local Server](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool#manual-testing---non-local-server).


## Manual Testing - Local Server

See
[Manual Testing - Local Server](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool#manual-testing---local-server).


## Automated Testing

See
[Automated Testing](https://github.com/cdeccio/byu-cs324-w2022/tree/master/lab-proxy-threadpool#automated-testing),
but use "epoll" in place of "threadpool" whenever the driver is used.


# Evaluation

Your score will be computed out of a maximum of 100 points based on the
following distribution:

 - 20 for basic HTTP proxy functionality
 - 75 for handling concurrent HTTP proxy requests using epoll
 - 5 - compiles without any warnings (this applies to your proxy code, not
   `tiny` and friends).

Run the following to check your implementation:

```b
$ ./driver.py -b 20 -c 75 epoll
```


# Submission

Run the following command to `tar` your file(s):

```bash
$ make handin
```

This creates a `.tar` file in the parent directory.  Upload this file to
the assignment page on LearningSuite.
