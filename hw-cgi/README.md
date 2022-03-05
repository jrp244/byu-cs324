# CGI

The purpose of this assignment is to give you hands-on experience with
CGI.


# Instructions

Write a CGI program in C.  This is a program that would be executed by an HTTP server
using `fork()` and `execve()`.  Your program should do the following:

 - Finish sending the headers for the HTTP response to the client.  The Web
   server will have sent the first line of the response and some headers.  Your
   program should send the headers indicating the type of the content (which
   will be "text/plain" and the length of the content, which is just the length
   (in bytes) of the response body.  For example:

   ```
   Content-type: text/plain
   Content-length: 5
   ```
 - Send the following string to the client as a response body:
   ```
   The query string is: Q
   ```
   But replace `Q` with the actual query string provided by the client.

Name your file `cgiprog.c`.  

Remember the following:
 - The value of the `Content-length` header needs to match the length of the
   response body.
 - End every header line with `\r\n`.
 - Ensure that the following four-character sequence exists beteween HTTP
   response headers and response body: `\r\n\r\n`.
 - Use the CGI specification to see how the query string and client socket are
   made available to the CGI program by the HTTP server.

You should recall that there is a way that you can test your CGI program
*without* a Web server (Hint: think of HW1 and HW2).

Note: this whole program will be much simpler than you might think--perhaps
even simpler than the program you wrote for your first homework assignment.
But you might need to review CGI from the book and slides.
