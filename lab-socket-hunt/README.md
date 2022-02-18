# Socket Hunt

The purpose of this assignment is to help you become more familiar with the
concepts associated with sockets, including UDP vs. TCP, local and remote port
assignment, IPv4 and IPv6, message parsing, and more.

# Table of Contents


# Getting Started


## Reading


## The Game

This lab involves a game between two parties: the *participant* and the
*director*.  The director runs on a CS lab machine, awaiting incoming
communications, acting initially as a server.  The participant initiates
communications with the director, acting as a client, requesting the first
chunk of the treasure, as well as directions to get the next chunk.  The
participant and director continue this pattern of requesting direction and
following direction, until the treasure has been received completeness.


TODO Example in C

### Initial Request

The very first message that the participant sends should be exactly 8 bytes
long and have the following format:

 - Byte 0: 0
 - Byte 1: an integer 0 through 4, corresponding to the *level* of the
   course.
 - Bytes 2 - 5: a `unsigned int` corresponding to the user ID of the user in
   network byte order (i.e., big-endian, most significant bytes first).  This
   can be retrieved with `id -g` on the CS lab machines:
   ```
   $ id -u
   ```
 - Bytes 6 - 7: an `unsigned short` used, along with the user ID, to seed the
   pseudo-random number generator used by the director, in network byte order.

   This is used to allow the participant to experience consistent behavior
   every time they interact with the director, to help with development and
   troubleshooting.


<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th><th>04</th><th>05</th><th>06</th><th>07</th></tr>
<tr>
<td colspan="1">0</td>
<td colspan="1">Level</td>
<td colspan="4">User ID</td>
<td colspan="2">Seed</td></tr>
</table>


### Directions Request

After the initial request, every subsequent request will be exactly four bytes
and will have the following format:

 - Bytes 0 - 3: an `unsigned int` having a value of one more than the nonce
   most recently sent by the director, in network byte order.  For example, if
   the director previously sent, 100, then this value would be 101.

<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th></tr>
<tr>
<td colspan="4">Nonce + 1</td></tr>
</table>


### Directions Response

Responses from the director are of variable length (but any given message will
consistent of fewer than 64 bytes).

 - Byte 0: an `unsigned char`.
   - If 0: the hunt is over.  All chunks of the treasure have been received in
     previous messages from the director.
   - If between 1 and 127: A chunk of the message, having length corresponding
     to the value of byte 0, immediately follows, beginning with byte 1.
   - If greater than 127: The director detected an error and is alerting the
     participant of the problem:
     - 129: The message was sent from an unexpected address or port (i.e., the
       source address or port of the packet received by the director).
     - 130: The message had an incorrect length.
     - 131: The value of the nonce was incorrect.
     - 133: After multiple tries the director was unable to bind properly to
       the address and port that it had attempted.
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
     port.  The result is that future communications *to* the director will be
     directed *to* this port (i.e., the *destination*) port), and future
     communications *from* the director will come *from* this port (i.e., the
     *source* port).
   - 2: Bind (i.e., using `bind()`) to the local port specified by the next two
     bytes (`n + 2` and `n + 3`), which is an `unsigned short` in network
     byte order.  The result is that future communications *to* the director
     will come *from* this port (i.e., the *source* port), and future
     communications *from* the director will be directed *to* that port (i.e.,
     the *destination* port).
   - 3: Read `m` datagrams from the socket (i.e., using the currently
     established local and remote ports), where `m` is specified by the next
     two bytes (`n + 2` and `n + 3`), which is an `unsigned short` in
     network byte order.  While `m` takes up two bytes for consistency with the
     other op-codes, its value will be only between 1 and 7.

     Each of the `m` datagrams received will be of length 2, and the contents
     of each will represent an `unsigned short` in network byte order.  The
     values of included in each of the `m` datagrams should be added together,
     and their sum is the nonce, whose value (plus 1) should be returned with
     the next communication to the director.  Note that the sum of these values
     might well sum to something that exceeds the 16 bits associated with an
     `unsigned short` (16 bits).  Thus, you will want to store the sum with an
     `unsigned int` (32 bits).
   - 4: Switch address families from using IPv4 (`AF_INET`) to IPv6
     (`AF_INET6`) or vice-versa.
 - Bytes `n + 2` - `n + 3`: These bytes, an `unsigned short` in network byte
   order is the parameter used in conjunction with the op-code.  This is
   included in all messages, for consistency, but it is only used for op-codes
   1 through 3; for op-codes 0 and 4, it exists but can be ignored.
 - Bytes `n + 4` - `n + 7`: These bytes, an `unsigned int` in network byte
   order, is the nonce, whose value, plus 1, should be returned in every
   communication back to the director.  In the case of op-code 4, this field is
   ignored.

<table border="1">
<tr>
<th>00</th><th>01</th><th>::</th><th>`n`</th><th>`n + 1`</th><th>`n + 2`</th><th>`n + 3`</th><th>`n + 4`</th>
<th>`n + 5`</th><th>`n + 6`</th><th>`n + 7`</th></tr>
<tr>
<td colspan="1">Chunk Length</td>
<td colspan="3">Chunk</td>
<td colspan="1">Op Code</td>
<td colspan="2">Op Param</td>
<td colspan="4">Nonce</td></tr>
</table>


## Resources Provided


# Automated Testing


# Evaluation


# Submission

Upload `participant.c` to the assignment page on LearningSuite.
