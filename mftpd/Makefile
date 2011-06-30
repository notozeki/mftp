CC = gcc
CFLAGS = -g -Wall
OBJS = mftpd.o request.o response.o io.o util.o
SRCS = $(OBJS:%.o=%.c)

mftpd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	-rm *.o
	-rm *~
	-rm mftpd