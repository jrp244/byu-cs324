# HTTP Proxy Lab - Threadpool

The purpose of this assignment is to help you become more familiar with the
concepts associated with client and server sockets, HTTP, and concurrent
programming by building a working HTTP proxy server with a threadpool.


# Table of Contents

 - [Overview](#overview)
 - [Preparation](#preparation)
   - [Reading](#reading)
 - [Instructions](#instructions)
   - [Part 1 - HTTP Request Parsing](#part-1---http-request-parsing)
   - [Part 2 - Sequential HTTP Proxy](#part-2---sequential-http-proxy)
   - [Part 3 - Threaded HTTP Proxy](#part-3---threaded-http-proxy)
   - [Part 4 - Threadpool](#part-4---threadpool)
 - [Testing](#testing-4)
   - [Manual Testing - Non-Local Server](#manual-testing---non-local-server)
   - [Manual Testing - Local Server](#manual-testing---local-server)
   - [Automated Testing](#automated-testing)
 - [Evaluation](#evaluation)
 - [Submission](#submission)


# Overview

A Web proxy is a program that acts as a intermediary between an HTTP client
(i.e., a Web browser) and an HTTP server.  Instead of requesting a resource
directly from the HTTP server, the HTTP client contacts the proxy server,
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


# Preparation

## Reading

Read the following in preparation for this assignment:
  - Sections 11.1 - 11.6, 12.1, and 12.3 - 12.5 in the book
  - The man pages for the following:
    - `tcp`
    - `socket`
    - `socket()`
    - `send()`
    - `recv()`
    - `bind()`
    - `connect()`
    - `getaddrinfo()`
    - `pthread_create()`
    - `pthread_detach()`
    - `pthread_self()`
    - `sem_init()`
    - `sem_post()`
    - `sem_wait()`
    - `sem_overview()` (unnamed semaphores)


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


## Part 2 - Sequential HTTP Proxy

As you implement this section, you might find it helpful to refer to the TCP
code from the
[sockets homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-sockets).


### Receiving the HTTP Request

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

   - `bind()` it to a port passed as the first argument from the
     command line, and configure it for accepting new clients with `listen()`.

   - Return the file descriptor associated with the server socket.
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
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics"
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```
(Replace `port` with the port on which your proxy server is listening.)

`curl` is a command-line HTTP client, described more in
[the section on manual testing](#manual-testing---non-local-server).
For the purposes of this section, `curl` creates and sends an HTTP request to
your proxy server, which is designated with `-x`.

Note that the request to `www-notls.imaal.byu.edu:5599` is included here and in
later tests only to test that your proxy server can properly parse the
non-default HTTP port.  However, that particular server and port will not be
part of tests that require your proxy server to actually connect to a Web
server because there is no Web server listening there.

Your proxy server (i.e., in `handle_client()`) should indicate that it has
received the client request by printing out the appropriate parts of the
request.  If it does not, now is the time to fix it.

Now try the following:

```bash
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```
(Replace `port` with the port on which your proxy server is listening.)

The `./slow-client.py` script is also described more in
[the section on manual testing](#manual-testing---non-local-server).
For the purposes of this section, it acts like `curl`, but it spreads its HTTP
request over several calls to `send()`.

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
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0

```
(Some things will differ, like the "User-Agent" header, which identfies your
client, and the port used.)

It is appropriate to send a *full url* to a *proxy* server, but when sending
directly to the *HTTP server*, sending just the *path* (and query string) is
appropriate.  Also, the protocol should be changed to HTTP/1.0, and the
"Connection" and "Proxy-Connection" headers added.  These further enforce
HTTP/1.0 behavior, which is discussed in the
[next section](#communicating-with-the-http-server).

Here is an example:

```
GET /cgi-bin/slowsend.cgi?obj=lyrics HTTP/1.0
Host: www-notls.imaal.byu.edu:5599
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0
Connection: close
Proxy-Connection: close

```

To simplify your request parsing and creation, you *may* simply replace *all*
headers that were sent by the client and create your own `Host`, `User-Agent`,
`Connection`, and `Proxy-Connection` headers, as shown above.  A sample value
for `User-Agent` is provided in your `proxy.c` file.  The value for the `Host`
field will be either `hostname:port` or simply `hostname`, if the port is the
default HTTP port.  For example:
```
Host: www-notls.imaal.byu.edu:5599
```
or
```
Host: www-notls.imaal.byu.edu
```
In the second example, port 80 is implied.

In summary, for the new HTTP request that was created:
- The *URL* in the first line, as received by the client, was changed to be a
  *path* (plus query string).
- The protocol is always HTTP/1.0 (this simplifies the client-server
  interaction for the purposes of this lab).
- The "Connection" and "Proxy-Connection" headers are added.
- The headers from the client may be completely replaced with `Host`,
  `User-Agent`, `Connection`, and `Proxy-Connection` headers that are generated
  by the proxy, for simplicity.

Remember that all lines in an HTTP request end with a carriage-return-newline
sequence, `\r\n`, and the HTTP request headers are ended with
the end-of-headers sequence, `\r\n\r\n` (i.e., a blank line after the last
header).

Use `printf()` and/or `print_bytes()` to print out the HTTP request you
created.  Then re-build and re-start your proxy, and make sure it works
properly when you run the following:

```bash
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics"
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu:5599/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
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

The re-build and re-start your proxy, and make sure it works properly when you
run the following:

```bash
$ curl -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
```
(Replace `port` with the port on which your proxy server is listening.  Also
note that the request to `www-notls.imaal.byu.edu:5599` is not included in
these latest tests. That URL was only used to make sure your proxy could parse
a non-standard port, which functionality will be useful later.)


### Returning the HTTP Response

To complete `handle_client()`, send the HTTP response back to the client,
exactly as it was received from the server--no further manipulation needed.
Once you have done it, call `close()` on the socket associated with the client.
Your proxy is using HTTP/1.0, so there will be no further HTTP requests over
the existing connection.


### Testing

At this point you should be able to pass:
 - [Tests performed against a non-local Web server](#manual-testing---non-local-server).
 - [Tests performed against a local Web server](#manual-testing---local-server).
 - [Automated tests](#automated-testing) with the following command:
   ```bash
   $ ./driver.py -b 50 threadpool
   ```


## Part 3 - Threaded HTTP Proxy

Once you have a working sequential HTTP proxy server, alter it to
handle multiple requests concurrently by spawning a new thread per client.
Formulate your main loop so that every time a new client connects (i.e.,
`accept()` returns) `pthread_create()` is called.  You will want to define a
thread function that is *passed to* `pthread_create()` (i.e., its third
argument) and *calls* `handle_client()`,  after which it waits for a new
client.

Note that with this particular thread paradigm, you should run your threads in
detached mode to avoid memory leaks.  When a new thread is spawned, you
can put it in detached mode by calling within the thread routine itself:
```c
pthread_detach(pthread_self());
```

Refer to the
[concurrency homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-concurrency)
for examples and code that you can integrate.


### Testing

At this point you should be able to pass:
 - [Tests performed against a non-local Web server](#manual-testing---non-local-server).
 - [Tests performed against a local Web server](#manual-testing---local-server).
 - [Automated tests](#automated-testing) with the following command:
   ```bash
   $ ./driver.py -b 50 -c 45 multithread
   ```


## Part 4 - Threadpool

Now that you have some experience with multi-threaded server, change your proxy
server to use a pool of threads to handle concurrent HTTP requests instead of
launching a new thread for each request.

When the program starts, initialize eight producer threads, a shared buffer
(queue) with five slots, and the associated semaphores and other shared data
structures to prepare the producer and consumers for handling concurrent
requests.  Formulate your producer loop, so that every time a new client
connects (i.e., `accept()` returns), the socket file descriptor returned is
handed off to one of the consumers, after which it waits for a new client by
calling `accept()` again.  You will need to modify your thread function so that
instead of handling a single client, it continually loops, waiting on new
clients from the shared buffer (queue) and handling them in turn.

Again, refer to the
[concurrency homework assignment](https://github.com/cdeccio/byu-cs324-w2022/tree/master/hw-concurrency)
for examples and code that you can integrate.


### Testing

At this point you should be able to pass:
 - [Tests performed against a non-local Web server](#manual-testing---non-local-server).
 - [Tests performed against a local Web server](#manual-testing---local-server).
 - [Automated tests](#automated-testing) with the following command:
   ```bash
   $ ./driver.py -b 50 -c 45 threadpool
   ```


# Testing

Some tools are provided for testing--both manual and automated:

 - The code for the `tiny` Web server
 - A driver for automated testing


## Manual Testing - Non-Local Server

Testing your proxy server against a production Web server will help check its
functionality.  To test basic, sequential HTTP proxy functionality, first run the
following to start your proxy server:

```bash
$ ./proxy port
```

Then in another window on the same machine, run the following:

```bash
$ curl -o tmp1 http://www-notls.imaal.byu.edu/cgi-bin/index.html
$ ./slow-client.py -o tmp2 -b 1 http://www-notls.imaal.byu.edu/index.html
$ curl -o tmp3 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -o tmp4 -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ curl -o tmp5 http://www-notls.imaal.byu.edu/images/imaal-80x80.png
```

`curl` is a command-line HTTP client.  The `-o` option tells `curl` to save the
contents of the requested URL to the specified file (e.g., `tmp1`, `tmp2`,
etc.), rather than printing the contents to standard output.

The `./slow-client.py` script does the same thing that `curl` does (including
use of the `-o` option), but it spreads out the HTTP request over several
`send()` calls to test the robustness of your proxy server in reading from a
byte stream.  The `-b` option designates the amount of time (in seconds) that
it will sleep in between lines that it sends.

Now run the following:

```bash
$ curl -o tmp1p -x http://localhost:port/ http://www-notls.imaal.byu.edu/cgi-bin/index.html
$ ./slow-client.py -o tmp2p -x http://localhost:port/ -b 1 http://www-notls.imaal.byu.edu/index.html
$ curl -o tmp3p -x http://localhost:port/ "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ ./slow-client.py -o tmp4p -x http://localhost:port/ -b 1 "http://www-notls.imaal.byu.edu/cgi-bin/slowsend.cgi?obj=lyrics"
$ curl -o tmp5p -x http://localhost:port/ http://www-notls.imaal.byu.edu/images/imaal-80x80.png
```

(Replace `port` with the port on which your proxy server is running.)

This time we used `-x` to specify a proxy server.

Finally, run the following to see if there are any differences (there should not be):

```bash
$ diff -u tmp1 tmp1p
$ diff -u tmp2 tmp2p
$ diff -u tmp3 tmp3p
$ diff -u tmp4 tmp4p
$ diff -u tmp5 tmp5p
```

Don't forget to remove them:

```bash
$ rm tmp1 tmp1p tmp2 tmp2p tmp3 tmp3p tmp4 tmp4p tmp5 tmp5p
```


## Manual Testing - Local Server

While testing on "non-local" Web servers is useful, having a copy of the code
for `tiny` is helpful for testing right on your local machine.  To use `tiny`
for testing:

 1. Enter the `tiny` sub-directory:
    ```bash
    $ cd tiny
    ```

 2. Compile `tiny`:
    ```bash
    $ make
    ```

    Note: there are a number of compilation errors in the `tiny` code.  This is
    a product of the textbook authors and needs some cleaning up, but you can
    disregard them for the purposes of this lab.

 3. Start `tiny`:
    ```bash
    $ ./tiny port2
    ```

    Replace `port2` with the port returned by `./port-for-user.pl` -- plus one.
    For example, if `./port-for-user.pl` returned 1234, then use 1235.  This
    allows you to use a *pair* of ports that are unlikely to conflict with
    those of another user--one for your proxy server and one for the `tiny` Web
    server.

 4. While `tiny` is running in one window or pane, start your proxy server:
    ```bash
    $ ./proxy port
    ```

    Replace `port` with the port returned by `./port-for-user.pl`.

With `tiny` running on one port (`port`) and your proxy server running on
another port (`port2`), both on the same system, try running the following:

```bash
$ curl -o tmp1 http://localhost:port2/home.html
$ curl -o tmp2 http://localhost:port2/csapp.c
$ curl -o tmp3 http://localhost:port2/godzilla.jpg
$ curl -o tmp4 "http://localhost:port2/cgi-bin/slow?sleep=1&size=4096"
$ ./slow-client.py -o tmp5 "http://localhost:port2/cgi-bin/slow?sleep=1&size=4096"
```

(Replace `port2` with the port on which the `tiny` Web server is running.)

Then run the following:

```bash
$ curl -o tmp1p -x http://localhost:port/ http://localhost:port2/home.html
$ curl -o tmp2p -x http://localhost:port/ http://localhost:port2/csapp.c
$ curl -o tmp3p -x http://localhost:port/ http://localhost:port2/godzilla.jpg
$ curl -o tmp4p -x http://localhost:port/ "http://localhost:port2/cgi-bin/slow?sleep=1&size=4096"
$ ./slow-client.py -o tmp5p -x http://localhost:port/ "http://localhost:port2/cgi-bin/slow?sleep=1&size=4096"
```

(Replace `port` with the port on which your proxy server is running and `port2`
with the port on which the `tiny` Web server is running.)

Now run the following to see if there are any differences (there should not be):

```bash
$ diff -u tmp1 tmp1p
$ diff -u tmp2 tmp2p
$ diff -u tmp3 tmp3p
$ diff -u tmp4 tmp4p
$ diff -u tmp5 tmp5p
```

Don't forget to remove them:

```bash
$ rm tmp1 tmp1p tmp2 tmp2p tmp3 tmp3p tmp4 tmp4p tmp5 tmp5p
```


## Automated Testing

For your convenience, a script is provided for automated testing.  You can use
it by running the following:

```bash
$ ./driver.py -b 50 -c 45 threadpool
```

The `-b` option specifies the points awarded for basic HTTP functionality, and
the `-c` option specifies the points awarded for handling concurrent client
requests.

Basic HTTP functionality involves requesting text and binary content over HTTP
via the proxy server, both from the local `tiny` Web server and non-local Web
servers, using both `curl` and `slow-client.py`  It downloads several resources
directly and via the proxy and checks them just as shown previously.

The concurrency test has two parts:
 - Issue a single request of the proxy server while it is busy with another
   request.
 - Issue five slow requests followed by five quick requests, to show that the
   fast requests are returned before the five slow requests.

Note that the driver can run with different options to help you troubleshoot.
For example:
 - *Basic Only*.  If you are just testing the basic functionality of your proxy
   (i.e., without concurrency), just use the `-b` option.
   ```bash
   $ ./driver.py -b 50 threadpool
   ```
 - *Increased Verbosity.*  If you want more output, including descriptions of
   each test that is being performed, use `-v`:
   ```bash
   $ ./driver.py -v -b 50 -c 45 threadpool
   ```
   For even more output, including the commands that are being executed, use
   `-vv`:
   ```bash
   $ ./driver.py -vv -b 50 -c 45 threadpool
   ```
 - *Proxy Output.*  If you want the output of your proxy to go to a file, which
   you can inspect either real-time or after-the-fact, use the `-p` option.
   Use `-p - ` for your proxy output to go to standard output.
   ```bash
   $ ./driver.py -p myproxyoutput.txt -b 50 -c 45 threadpool
   ```
 - *Downloaded Files.*  By default, the downloaded files are saved to a
   temporary directory, which is deleted after the tests finish--so your home
   directory does not get bloated.  If you want to keep these files to inspect
   them, use the `-k` option.
   ```bash
   $ ./driver.py -k -b 50 -c 45 threadpool
   ```
   If you use this option, be sure to delete the directors afterwards!

Any of the above options can be used together.


# Evaluation

Your score will be computed out of a maximum of 100 points based on the
following distribution:

 - 50 for basic HTTP proxy functionality
 - 45 for handling concurrent HTTP proxy requests using a threadpool
 - 5 - compiles without any warnings (this applies to your proxy code, not
   `tiny` and friends).

Run the following to check your implementation:

```b
$ ./driver.py -b 50 -c 45 threadpool
```


# Submission

Run the following command to `tar` your file(s):

```bash
$ make handin
```

This creates a `.tar` file in the parent directory.  Upload this file to
the assignment page on LearningSuite.
