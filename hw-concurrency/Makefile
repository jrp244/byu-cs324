CC = gcc
CFLAGS = -g

all: echoserveri echoserverp echoservert echoservert_pre

echoserveri: echoserveri.c echo.c csapp.c
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c echo.c csapp.c -lpthread -lm

echoserverp: echoserverp.c echo.c csapp.c
	$(CC) $(CFLAGS) -o echoserverp echoserverp.c echo.c csapp.c -lpthread -lm

echoservert: echoservert.c echo.c csapp.c
	$(CC) $(CFLAGS) -o echoservert echoservert.c echo.c csapp.c -lpthread -lm

echoservert_pre: echoservert_pre.c sbuf.c echo.c echo_cnt.c csapp.c
	$(CC) $(CFLAGS) -o echoservert_pre echoservert_pre.c sbuf.c echo.c echo_cnt.c csapp.c -lpthread -lm
