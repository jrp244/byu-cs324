# HTTP Proxy Lab - Threadpool

The purpose of this assignment is to help you become more familiar with the
concepts associated with client and server sockets, HTTP, and concurrent
programming by building a working HTTP proxy server with a threadpool.


# Table of Contents


# Overview

A Web proxy is a program that acts as a intermediary between an HTTP client
(i.e., a Web browser) and an HTTP server.  Instead of requesting a resource
directing from the HTTP server, the HTTP client contacts the proxy server,
which forwards the request on to the HTTP server. When the HTTP server replies
to the proxy, the proxy sends the reply on to the browser.  In this way, the
client acts as both a *server* (to the Web browser) and a *client* (to the HTTP
server).

Proxies are useful for many purposes.  Sometimes proxies are used in firewalls,
so that browsers behind a firewall can only contact a server beyond the
firewall via the proxy.  Proxies can also act as anonymizers: by stripping
requests of all identifying information, a proxy can make the browser anonymous
to Web servers.  Proxies can even be used to cache web objects by storing local
copies of objects from servers then responding to future requests by reading
them out of its cache rather than by communicating again with remote servers.

In this lab, you will write a simple HTTP proxy objects.  For the first part of
the lab, you will set up the proxy to accept incoming connections, read and
parse requests, forward requests to web servers, read the servers' responses,
and forward those responses to the corresponding clients.  This first part will
involve learning about basic HTTP operation and how to use sockets to write
programs that communicate over network connections.  In the second part, you
will upgrade your proxy to deal with multiple concurrent connections using a
simple, thread-based model.  This will introduce you to dealing with
concurrency, a crucial systems concept.  In the the third part, you will modify
your concurrency approach to use a threadpool.


# Instructions

## Part 1 - HTTP Request Parsing

The first step in building an HTTP proxy server is parsing an incoming HTTP
request.  Some skeleton functions have been created for you in `proxy.c`,
namely `all_headers_received()` and `parse_request()`.  These are provided in
the case they are helpful for you, but you not are required to use them; if it
is more intuitive for you to complete this in another way, you are welcome to
do that.

These functions will be used by your proxy server to know when a client is done
sending its request and to parse the request.  You will find the string
functions (man `string`) very useful for this, including `strcpy()`,
`strstr()`, `strchr()`, and more!


### `all_headers_received()`

`all_headers_received()` takes the following as an argument:
 - `char *request`: a (null-terminated) string containing an HTTP request.

This function tests whether or not the HTTP request associated with `request`
contains all the headers that were intended to be sent.  This is not actually a
matter of checking the headers themselves but of checking for the
end-of-headers sequence, `\r\n\r\n`.  It returns 1 if all headers have been
received (i.e., end-of-headers sequence is found) and 0 otherwise.

In this lab, all requests will consist of only first line and one or more HTTP
headers; there will be no request body.  


### `parse_request()`

`parse_request()` takes the following as an arguments:
 - `char *request`: a (null-terminated) string containing an HTTP request.
 - `char *method`: a string to which the method, extracted from the URL, is
   copied.
 - `char *hostname`: a string to which the hostname, extracted from the URL, is
   copied.
 - `char *port`: a string to which the port, extracted from the URL, is
   copied.  If no port is specified in the URL, then it should be populated
   with "80", the default port for HTTP.
 - `char *path`: a string to which the path, extracted from the URL, is
   copied.  The path should include not only the file path, but also the query
   string, if any.
 - `char *headers`: a string to which the headers (i.e., all lines after the
   first line) are copied.


### Testing

The `test_parser()` function was built for you to test the HTTP parsing code.
It provides three scenarios: complete HTTP request with default port; complete
HTTP request with explicit port and query string; and incomplete HTTP request.

Compile your proxy code by running the following:

```bash
$ make
```

Then run the following to see that it behaves as you would expect it to:

```bash
$ ./proxy
```

At this point, remove or comment out the call to `test_parser()` in `main()`;
it was just used for testing.


## Part 2 - Sequential Web Proxy

As you implement this section, you might find it helpful to refer to the TCP
code from the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets).


### Receiving the HTTP Request

Write functions for each of the following:

 - `open_sfd()` - Create a socket with address family `AF_INET` and type
   `SOCK_STREAM`, `bind()` it to a port passed as the first argument from the
   command line, and configure it for accepting new clients with `listen()`.
   Return the file descriptor associated with the server socket.
 - `handle_client()` - Given a newly created file descriptor, returned from
   `accept()`, handle a client HTTP request.  For now, just have this method do
   the following:
   - Read from the socket into a buffer until the entire HTTP request has been
     received (again, there are no request headers in this lab, so this is
     basically just end of headers).  Use the `parse_request()` function for this.
   - Print out the values from the HTTP request, once you have received it in
     its entirety (e.g., like `test_parser()` does).
   - Close the socket.
   Later will you replace printing the values with more meaningful
   functionality. This first part is just to get you going in the right
   direction.

Now flesh out `main()` such that it calls `open_sfd()` then uses a `while(1)`
loop to do the following:
 - `accept()` a client connection
 - call `handle_client()` to handle the connection

At this point, there are no threads, no concurrency, and no HTTP response.
But you should be able to get a sense for how your proxy server is progressing.

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
$ curl -x http://localhost:port/ http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics
$ curl -x http://localhost:port/ http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics
```
(Replace `port` with the port on which your proxy server is listening.)

`curl` is a command-line HTTP client.  The `-x` option is used to specify a
proxy server.  Note that if you leave out the `-x` option, it will bypass the
proxy and go directly to the server.

Your proxy server (i.e., in `handle_client()`) should indicate that it has
received the client request by printing out the appropriate parts of the
request.  If it does not, now is the time to fix it.

Now try the following:

```bash
$ ./slow-client.py -x http://localhost:port/ -b 1 http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics
$ ./slow-client.py -x http://localhost:port/ -b 1 http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics
```
(Replace `port` with the port on which your proxy server is listening.)

The `./slow-client.py` script does the same thing that `curl` does (including
use of the `-x` option), but it spreads out the HTTP request over several
`send()` calls to test the robustness of your proxy server in reading from a
byte stream.  The `-b` option designates the amount of time (in seconds) that
it will sleep in between lines that it sends.

Again, your proxy server (i.e., in `handle_client()`) should indicate that it
has received the client request by printing out the appropriate parts of the
request.  If it does not, now is the time to fix it.


### Creating an HTTP Request

Now that you proxy server has received the entire HTTP request, you can modify
your `handle_client()` code to create the HTTP request to send to the server.
The HTTP request your proxy received from the client looked something like
this:
```
GET http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics HTTP/1.1
Host: www-notls.imaal.byu.edu:5599
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";
```
It is appropriate to send a *full url* to a *proxy* server, but when sending
directly to the *HTTP server*, sending just the *path* (and query string) is
appropriate.  Also, the protocol should be changed to HTTP/1.0, and the
"Connection" and "Proxy-Connection" headers added.  Here is an example:
```
GET /cgi-bin/slowsend.cgi?obj=lyrics HTTP/1.0
Host: www-notls.imaal.byu.edu:5599
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";
Connection: close
Proxy-Connection: close
```
In summary, only the following have changed between the request that was
received and the new one that was created:
- The URL in the first line was changed to a path (plus query string).
- The protocol is always changed to HTTP/1.0 (this simplifies the client-server
  interaction for the purposes of this lab).
- The "Connection" and "Proxy-Connection" headers are added.
   
Remember that all lines in an HTTP request end with a carriage-return-newline
sequence, `\r\n`, and the HTTP request headers are ended with
the end-of-headers sequence, `\r\n`.

Use `printf()` and/or `print_bytes()` to print out the HTTP request you
created.  The re-build and re-start your proxy, and make sure it works properly
when you run the following:

```bash
$ curl -x http://localhost:port/ http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics
$ curl -x http://localhost:port/ http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics
$ ./slow-client.py -x http://localhost:port/ -b 1 http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics
$ ./slow-client.py -x http://localhost:port/ -b 1 http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics
```
(Replace `port` with the port on which your proxy server is listening.)


### Communicating with the HTTP Server

With the modified HTTP request prepared, you can now communicate with the HTTP
server.  Modify your `handle_client()` function again:

 - Use `getaddrinfo()` and `connect()` to create a *new* socket and establish a
   connection with the HTTP server (i.e., the host and port specified by the
   client in its request).  This does not replace the socket with the client;
   the proxy server is now communicating with both client *and* server
   concurrently, using a dedicated socket for each connection.
 - Send the updated HTTP request over the socket connected to the server.
 - Receive the HTTP response from the server.  Just like when the proxy server
   received the request from the client, the proxy will need to loop and
   continue reading from the server socket until the entire response has been
   received.

   With HTTP/1.0 (what is being used in *this* lab), only *one* request is made
   over a given TCP connection.  Thus, when the server has sent all it has to
   send (the entire HTTP response), it closes the connection--as opposed to
   waiting for another HTTP request.  An HTTP/1.0 client (in this case, the
   proxy server) therefore knows when it has received the entire HTTP response
   when `read()` or `recv()` returns 0 (i.e., the indicator that the other side
   has called `close()` on the socket).

   A client-server pair using a more modern HTTP version (i.e., HTTP/1.1 or
   higher) might exchange several request-response transactions over the same
   TCP connection, which case, the "Content-Length" header (and/or other modern
   conventions) would need to be consulted to determine when the server was
   finished sending a given response.  But again, for *thiis* lab, that is not
   necessary.
 - Close the socket associated with the HTTP server.

Now would be a good time to test with the same commands you did in the last
section.


### Returning the HTTP Response

To complete `handle_client()`, send the HTTP response back to the client,
exactly as it was received from the server--no further manipulation needed.
And close the socket associated with the client; your proxy uses HTTP/1.0, and
you are done!


### Testing 

TBD


## Part 3 - Threaded Web Proxy

Once you have a working sequential HTTP proxy server, alter it to
handle multiple requests concurrently.  Initially, spawn a new thread to handle
each new connection request.

Note that with this particular thread paradigm, you should run your threads in
detached mode to avoid memory leaks.  When a new thread is spawned, you
can put it in detached mode by calling within the thread routine itself:
```c
pthread_detach(pthread_self());
```


## Part 4 - Threadpool

For the final part of the lab, you will change your proxy server to use a pool
of threads to handle concurrent HTTP requests instead of launching a new thread
for each request.


# Evaluation

Your score will be computed out of a maximum of 100 points based on the
following distribution:

 - 50 points for basic HTTP proxy functionality
 - 45 points for handling concurrent HTTP proxy requests using a threadpool
 - 5 compiles without any warnings


# Automated Testing

TBD


# Submission

TBD
