# Sockets

The purpose of this assignment is to give you hands-on experience with sockets.
Code is provided for a working client and server that communicate over
sockets of type `SOCK_DGRAM` (UDP).  You will experiment with that code and
then modify it to work for `SOCK_STREAM` (TCP), so you become familiar with
both communication types.


# Preparation

 1. Read the following in preparation for this assignment:
    - Sections 11.1 - 11.4 in the book
    - The man pages for the following:
      - `udp`
      - `tcp`
      - `send()` / `sendto()` / `write()`
      - `recv()` / `recvfrom()` / `read()`

 2. Run `make` to build two executables: `client` and `server`.  These are
    programs that will communicate with each other as client and server,
    respectively.

 3. Read the [Strings and Bytes](#strings-and-bytes) section in this document.

 4. Start a tmux session.  Create three panes, such that the window looks like
    this:

    ```
    ---------------------------
    |  remote   |    local    |
    | (server)  |   (client)  |
    --------------------------|
    |         remote          |
    |       (analysis)        |
    ---------------------------
    ```

 5. On the two "remote" panes, ssh to a different CS lab machine machine (see a
    list of machine names
    [here](https://docs.cs.byu.edu/doku.php?id=open-lab-layout)).  You must log
    in to the _same_ lab machine from both "remote" panes.

 6. You will be doing a writeup on this assignment. Call your writeup
    `sockets.txt`. Make sure you answer any questions/provide outputs as
    stated in the *bolded* questions/statements (there should be XX in all).


## Strings and Bytes

This section provides some important background related to the nuances between
"strings" and "bytes" in C.  Both use an array of type `char` and are really
just arrays of 8-bit integers.  However, the same array of `char` is treated
very differently when used with a function related to strings (e.g.,
`strcmp()`, `strcpy()`, `fputs()`, and `fgets()`) than when used with a
function related to bytes (e.g., `memcmp()`, `memcpy()`, `write()` and
`read()`).  The string-related functions operate on all characters in the array
from the begining until a "null" (i.e., value 0) character is reached, whereas
the byte-related functions operate on all characters, independent of the
presence of a null character.  That is why the byte-related functions always
involve a explicit length; it cannot be inferred otherwise.

Consider the following definition:

```c
char buf[] = { 'a', 'b', 'c', '\0', 'd', 'e', 'f' };
```

The following two statements result in the equivalent behavior (minus the
effects of buffering with `fputs()`):

```c
fputs(buf, stdout);
write(1, buf, 3);
```

`fputs()` stops at index 2 (value 'c'`) because of the null value at index 3,
whereas `write()` could care less about the character with null value.  In
fact, `write()` will continue writing characters from the start of `buf`, for
as long you designate.  For example:

```c
write(1, buf, 7);
```

or even (gasp!):

```c
write(1, buf, 15);
```

Yes, the second one will write memory 8 bytes beyond `buf`.  This is allowed in
C, but in practice, you will want to check the bounds of your array before
reading from or writing to that array, so you don't read or write beyond.

Finally, the following declarations result in `buf1` and `buf2` having the
same exact content as `buf` above:

```c
char buf1[] = { 97, 98, 99, 0, 100, 101, 102 };
char buf2[] = { 0x61, 0x62, 0x63, 0x00, 0x64, 0x65, 0x66 };
```

If that does not make sense to you, remember that a `char` is just an 8-bit
integer.  When used with string printing operations, an array of `char` is
presented using its ASCII equivalent (e.g., 97 is a, 98 is b, etc.  See `man
ascii` for more).  However, no matter how it is _presented_ (`'a'`, `97`,
`0x67`), it is still just an integer.  

Now, we include the following table, which provides the equivalent functions
for comparing, copying, and performing I/O operations with strings and bytes:

| String | Bytes |
| -------| ----- |
| `strcmp()` | `memcmp()` |
| `strcpy()` | `memcpy()` |
| `fputs()` | `write()` |
| `fgets()` | `read()` |
| `strlen()` | no equivalent! |

Remember, that the string operations should only be used when you _know_ that
you are working with a array of bytes that has a null character.  That will
_not_ typically be the case with sockets.  The primary way, therefore, to keep
track of byte lengths with socket operations is with the return values of
`recv()`/`recvfrom()`/`read()` or `send()`/`sendto()`/`write()`.


# Part 1: UDP Sockets

Open `client.c`, and look at what it does.  Then answer the following
questions.

 1. *What two system calls are used to create and prepare a (UDP) client socket
    for reading and writing, before you ever read or write to that socket?*

 2. *Describe how your client code for reading and writing would be different
    if the second call were not used.*  See the man page for `udp`,
    specifically within the first two paragraphs of the "DESCRIPTION" section.

 3. *Where do the strings come from that are sent to the server (i.e., written
    to the socket)?*

Open `server.c`, and look at what it does.  Specific questions about the server
will come later.

In the next steps, you will be experimenting with UDP sockets on the client and
server side, using the `client` and `server` programs, respectively.

In the top-left "remote" pane, run the the following command:

```bash
$ ./server -4 port
```

(Replace `port` with a port of your choosing, an integer between 1024 and
65535.  Use of ports with values less than 1023 require root privileges).

The `-4` forces the server to prepare the socket to receive incoming messages
only on its IPv4 addresses.  While it is possible to have a program listen on
both IPv4 _and_ IPv6, that requires _two_ sockets--one for each protocol--and
that goes beyond the scope of this assignment, so using either `-4` or `-6`
designates which specific single address family (IP version) to use, IPv4 or
IPv6, respectively.

While it looks like the server is "hanging", it is actually just awaiting
incoming data from some client.  This is what is referred to as _blocking_.  To
see the system call on which it is blocking, sandwich the line containing the
`recvfrom()` statement between the following two print statements:

```c
printf("before recvfrom()\n"); fflush(stdout);
// recvfrom() goes here...
printf("after recvfrom()\n"); fflush(stdout);              
```

Then re-run `make` and restart the server using the same command-line arguments
as before.  While your server is again blocking, at least you can see _where_!

Now, let's run the client to create some interaction between client and server.
In the top-right "local" pane, run the following:

```bash
$ ./client -4 hostname port foo bar abc123
```

(Replace `hostname` and `port` with name of the "remote" host and the port on
which the server program is now listening, respectively.)

The `-4` forces the client to prepare the socket to send messages to the
server's IPv4 address only.  Remember the server is listening on its IPv4
addresses only.  It is possible to have a your client try to communicate to the
server over whichever works first--IPv4 or IPv6, but there are some challenges
with doing that over UDP, so at this point, using `-4` is the best option.

Now run the command a second time:

```bash
$ ./client -4 hostname port foo bar abc123
```

 4. The server prints out the remote (i.e., client-side) address and port
    associated with the incoming message.  *What do you notice about the port
    value used by the client for different messages sent using the _same_
    socket (i.e., from running `./client` a single time)?*
 5. *What do you notice about the port value used by the client for different
    messages sent using _different_ sockets (i.e., from running `./client`
    multiple times)?*
 6. *Looking inside `server.c`, how many sockets does the server use to
    communicate with multiple clients?*  For example, one for _each_ client,
    one for _all_ clients, etc.

Let's make some other observations.  First, note that the lengths (i.e., number
of bytes) of the messages sent were longer than the lengths of the strings
making up the messages.  This is because we _chose_ to explicitly include the
null character for this particular program:

```c
		len = strlen(argv[j]) + 1;
		/* +1 for terminating null byte */
		// ...
		if (write(sfd, argv[j], len) != len) {
```

`strlen()` is used on `argv[j]` only because we know it is a null-terminated
string.  But `write()` is only concerned with bytes, so writing with argument
`len` will result in writing one more character than the string is long--the
null character.  See [Strings and Bytes](#strings-and-bytes) for more.  When
the server echoes back our message, we can use string operations on it--but
only because we know that it contains the null character that we included when
we sent the message.

Now take note of how the number of calls to `send()` on the client relates to
the number of `recvfrom()` calls on the server.  Let's make some modifications
to both client and server code to better understand what is going on:

 - Modify `server.c`:
   - sleeps for 2 seconds immediately after calling `recvfrom()` on the socket.
   - remove the `printf()` statements that you added earlier around the
     `recvfrom()` statement.
 - Modify `client.c` such that it does not attempt to read from the
   socket--or print what it read--after writing to the socket.  Comment out
   the code associated with the `read()` and `printf()` as described.

These changes make it so that the client is no longer waiting for the server to
respond before sending its subsequent messages; it just sends them one after
the other.  The two-second `sleep()` effectively _guarantees_ that the second
and third packets will _both_ have been received by the server's kernel, ready
to be read, before `recvfrom()` is called by the server.

Re-run `make` to rebuild both binaries.  Then interrupt and restart the server
in the top-left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the top-right "local" pane, sending the same strings as
before.

 7. *How many total calls to `send()` / `write()` were made by the client?*
    Refer to `client.c`.
 8. *How many messages were received by the kernel of the server-side process
    _before_ the server called `recvfrom()` the second time?*
 9. *How many total calls to `recvfrom()` were required for the server process
    to read all the messages/bytes that were sent?*
 10. *Why didn't the server read all the messages that were ready with a single
     call to `recvfrom()`?*  Hint: see the man page for `udp`, specifically
     within the first three paragraphs of the "DESCRIPTION" section.


## Part 2: TCP Sockets

In the next steps, you will be modifying the programs, so that they communicate
over TCP instead of UDP.  You will experiment with these TCP sockets on the
client and server side, using the `client` and `server` programs, respectively.

Before you being modifications, make a copy of the UDP version of your client
and server programs:

```bash
$ cp client.c client-udp.c
$ cp server.c server-udp.c
```

Make the following modifications:

 - Modify `client.c`:
   - Make the socket use TCP instead of UDP.
   - Add a 30-second `sleep()` immediately before the `for` loop in which the
     messages are sent to the server.
   - Uncomment the read/print code that you commented out in Part 1.

 - Modify `server.c`:
   - Make the server socket use TCP instead of UDP.
   - Wrap the entire `for` loop (i.e., `for (;;)`) in _another_ `for` loop with
     the same conditions (i.e., `for (;;)`).
   - Immediately before the _outer_ `for` loop, call the `listen()` function on
     the TCP server socket (you can use a `backlog` value of 100).
   - _Inside_ the outer `for` loop and immediately _before_ the inner `for`
     loop, use the `accept()` function to block, waiting for a client to
     connect, and return a new client socket.
     - You will need to declare a new variable of type `int` to hold the socket
       returned by `accept()`.
     - You can re-use some of the arguments that are currently used with
       `recvfrom()`, further down.
     - You will need to initialize `peer_addr_len` before it is used with
       `accept()`, just as it is currently initialized before being called with
       `recvfrom()`.
   - Change the `recvfrom()` call to `recv()`  and the `sendto()` call to
     `send()`.  Note that you just need to remove some of the arguments for
     each.
   - Comment out the `sleep()` call that you added in Part 1.
   - Use the file descriptor returned by `accept()` in the `recv()` and
     `send()` calls (i.e., the client socket).
   - If `recv()` returns 0 bytes, then:
     - call `close()` on the client socket.  When 0 is returned by `recv()`,
       the client has closed its end of the connection and is effectively EOF.
     - break out of the inner `for` loop; we can now listen for another client.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the top-left "remote" pane.

 11. *How does the role of the original socket (i.e., `sfd`, returned from the
     call to `socket()`), after `listen()` is called on it, compare with  the
     role of the socket returned from the call to `accept()`?*  See the man
     pages for `listen()` and `accept()`.

 12. *With the new changes you have implemented, how have the semantics
     associated with the call to `connect()` changed?  That is, what will
     happen now when you call `connect()` that is different from when you
     called `connect()` with a UDP socket?*  See the man pages for `connect()`,
     `tcp`, and `udp`.

While the server is running on the remote host in the top-left "remote" pane,
run the following in the top-right "local" pane:

```bash
$ ./client -4 hostname port foo bar abc123
```

(Replace `hostname` and `port` with name of the "remote" host and the port
on which the server program is now listening, respectively.)

The `ss` command can be used to show active TCP connections on a given host.
With both client and server running, run `ss` in the bottom "remote" pane
during the client's 30-second sleep, i.e., _after_ the call to `connect()` but
_before_ the client has sent any messages.

Use the `-t`, `-p`, `-n` options when you run `ss` so that, respectively:

 - only TCP sockets are shown
 - the PIDs and command names are shown alongside the sockets listed
 - IP addresses are shown instead of names

Finally, after all the above command-line options, show only sockets bound to
the local port that you have selected for the server by adding the following
(_including_ quotes):

```
"sport = :port"
```

(replace the "port" on the right-hand side of the colon with the actual port
you used).

See the man page for `ss` for more on how to use these options.

 13. *Why does the `ss` output show an established connection ("ESTAB") between
     client and server before any messages are sent from client to server?*
     Hint: see the man page for `tcp`, specifically within the first two
     paragraphs of the "DESCRIPTION" section.

Make the following modification:
  - In `client.c`, remove the 30-second `sleep()`.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the top-left "remote" pane.

Now run the following command twice:

```bash
$ ./client -4 hostname port foo bar abc123
$ ./client -4 hostname port foo bar abc123
```

 14. The server prints out the remote (i.e., client-side) address and port
     associated with the incoming message.  *What do you notice about the port
     value used by the client for different messages sent using the _same_
     socket (i.e., from running `./client` a single time)?*
 15. *What do you notice about the port value used by the client for different
     messages sent using _different_ sockets (i.e., from running `./client`
     multiple times)?*
 16. *Looking inside `server.c`, how many sockets does the server use to
     communicate with multiple clients?*  For example, one for _each_ client,
     one for _all_ clients, etc.  *How does this compare to the answer to the
     behavior for a server-side UDP socket (see #6)?*

Make the following modifications, which mirror those made in part 2:

  - Modify `server.c` such that it sleeps for 2 seconds immediately after
    calling `recv()` on the socket.
  - Modify `client.c` such that it does not attempt to read from the
    socket--or print what it read--after writing to the socket.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the top-left "remote" pane.

While the server is running on the remote host in the top-left "remote"
pane), run the following in the top-right "local" pane:

```bash
$ ./client -4 hostname port foo bar abc123
```

 17. *How many total calls to `send()` / `write()` were made by the client?*
     Refer to `client.c`.
 18. *How many messages were received by the kernel of the server-side process
     _before_ the server called `recvfrom()` the second time?*
 19. *How many total calls to `recvfrom()` were required for the server process
     to read all the messages/bytes that were sent?*
 20. *How and why does the answer to #18 differ from that from #9?*
     Hint: see the man page for `tcp`, specifically within the first paragraph
     of the "DESCRIPTION" section.


## Part 3: Making Your Client Issue HTTP Requests

Modify `client.c` such that instead of looping through each command-line
argument and writing it to the socket, it does the following, immediately after
the socket connection is established:

 - Write a loop to read (using `fread()`) input from standard input (`stdin`)
   into a buffer (`char []`) until EOF is reached (max total bytes 4096).  You
   can designate a specific "chunk" size (e.g., 512 bytes) to read from the
   file with each loop iteration.
 - Keep track of the bytes that were read from `stdin` during each iteration
   and in total.  Hint: see the return value of `fread()`.  With each iteration
   of the loop, you will want to offset the buffer (the one to which you are
   writing data read from `stdin`) by the number of total bytes read, so the
   bytes read from `stdin` are placed in the buffer immediately following the
   previous bytes read.
 - After _all_ the data has been read from `stdin` (i.e., EOF has been
   reached), loop to send all the data that was received (i.e., the bytes you
   just stored in the buffer), until it has all been sent.  You can designate a
   specific message size (e.g., 512 bytes) to send with each loop iteration.
   You can use `write()` or `send()` to send the bytes.

   Note that `write()` / `send()` will return the number of bytes actually
   sent, which might be less than the number you requested to be sent (see the
   `write()` man page for more!), so you need to keep track of the total bytes
   sent to ensure that all has been sent and write your loop termination test
   accordingly.

In the top-left "remote" pane, start a netcat (`nc` command) server listening
for incoming TCP connections on a port of your choosing, and such that its
output is piped to the `sha1sum` command:

```bash
$ nc -l port
```

(Replace `port` with a port of your choosing.)

Now test your client program by running the following in the top-right "local"
pane:

```bash
$ ./client < alpha.txt
```

Because the open file descriptor associated with `alpha.txt` will be duplicated
on the standard input of `./client`, the contents of `alpha.txt` should be read
and sent over the socket to `nc`, which should print them out to the console.

To ensure that all bytes from the file were sent by `client` and received by
`server`, re-run `nc`, this time piping its standard output to `sha1sum`:

```bash
$ nc -l port | sha1sum
```

Then re-run the client program:

```bash
$ ./client < alpha.txt
```

 21. *What is the output of `sha1sum`?*

     Hint: it should match the output of the following:
     ```bash
     $ sha1sum < alpha.txt
     ```

Modify `client.c`:

 - After _all_ the data read from `stdin` has been sent to the socket, write
   another loop to read (using `read()` or `recv()`) from the socket into a
   buffer (`char []`) until the remote host closes its socket--that is, the
   return value from `read()` / `recv()` is 0 (note that this is, effectively,
   EOF).  The maximum _total_ bytes that you will read fom the socket is 16384.
   You can designate a specific "chunk" size (e.g., 512 bytes) to read from the
   socket with each loop iteration.
 - Keep track of the bytes that were read from the socket during each iteration
   and in total.  Hint: see the return value of `read()` / `recv()`.  With each
   iteration of the loop, you will want to offset the buffer (the one to which
   you are writing data read from the socket) by the number of total bytes
   read, so the bytes read from the socket are placed in the buffer immediately
   following the previous bytes read.
 - After _all_ the data has been read from the socket (the remote host has
   closed its socket), write the contents of the buffer to `stdout`.  Because
   the data you have read will not necessarily be a string (i.e.,
   null-terminated), you should not use `printf()`. Alternatives are `fwrite()`
   or `write()`.  Similarly, if you want to perform _any_ string operations,
   then you will need to add the null byte yourself.

Now, execute your client program such that:
 - you are sending data to the standard HTTP port (80) at
   www-notls.imaal.byu.edu;
 - you are redirecting the contents of `http-bestill.txt` to the standard input
   of the client process (using input redirection on the shell); and
 - you are redirecting the output of your client process to `bestill.txt`.

What your program is doing is the following:
 1. Reading content from the file `http-bestill.txt` (redirected to standard
    input), which happens to be an HTTP request;
 2. Sending the request to an HTTP server;
 3. Reading the HTTP response from the server; and
 4. Writing the response to `bestill.txt` (to which standard output has been
    redirected).

In essence, your client program is acting as a _very_ simple HTTP client.

Note that after you have run your program, `bestill.txt` should contain:
 - the HTTP response code (200);
 - all HTTP headers returned in the HTTP response; and
 - all three verses to a hymn.


 22. *Show the command pipeline that you used to run your client program and
     issue the request.*

 23. *Show the output to the following:*
     ```bash
     $ cat bestill.txt
     ```

The previous command execution involved an HTTP request for a file of type
"text/plain", which, of course, is a plaintext file.  Now we will try to
retrieve an image file, with arbitrary byte values.  Execute your client
program such that:
 - you are sending data to the standard HTTP port (80) at
   www-notls.imaal.byu.edu;
 - you are redirecting the contents of `http-socket.txt` to the standard input
   of the client process (using input redirection on the shell);
 - you are redirecting the standard output of your client process to
   `./strip_http.py`; and
 - you are redirecting the standard output of `./strip_http.py` to `socket.jpg`.

The `strip_http.py` script simply strips the HTTP response headers from the
output, so that you are left with just the content.  The file `socket.jpg`
should now contain a jpeg image that you can open and view with a suitable
program (e.g., a Web browser) to check its correctness.

 24. *Show the command pipeline that you used to run your client program and
     issue the request.*

 25. *Show the output to the following:*
     ```bash
     $ sha1sum socket.jpg
     ```


## Part 4: Review Questions

For this final set of questions, you are welcome to refer to previous
code/questions, set up your own experiments, and/or read the man pages for
`recv()` (especially), `tcp`, and `udp`.

 26. What happens when you call `read()` (or `recv()`) on an open socket (UDP
     or TCP), and there are no messages are available at the socket for reading?
     Hint: see the man page for `recv()`, especially the "DESCRIPTION" section.

 27. What happens when you call `read()` (or `recv()`) on an open socket (UDP
     or TCP), and the amount of data available is less than the requested
     amount?  Hint: see the man page for `recv()`, especially the "DESCRIPTION"
     section.

 28. What happens you you call `read()` (or `recv()`) on an open UDP socket,
     and you specify a length that is less than the length of the next
     datagram?  Hint: see the man page for `udp`, specifically within the first
     three paragraphs of the "DESCRIPTION" section.

Close down all the terminal panes in your `tmux` session to _close_ your `tmux`
session.


# Submission:

Upload `sockets.txt` to the assignment page on LearningSuite.
