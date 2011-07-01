CC = gcc
CFLAGS = -g -Wall
OBJS = mftp.o analyze.o communi.o io.o ../mftpd/util.o
SRCS = $(OBJS:%.o=%.c)

mftp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	-rm *.o
	-rm *~
	-rm mftp