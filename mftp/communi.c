#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../mftpd/command.h"
#include "../mftpd/util.h"

#define BUFSIZE 256

void send_request(int soc, Command* cmd)
{
	char buf[BUFSIZE];

	switch ( cmd->type ) {
	case GET:
		snprintf(buf, sizeof(buf), "GET %s\n", cmd->value);
		break;

	case PUT:
		snprintf(buf, sizeof(buf), "PUT %s %d\n", cmd->value, cmd->size);
		break;

	case LIST:
		snprintf(buf, sizeof(buf), "LIST %s\n", cmd->value);
		break;

	case EXIT:
		snprintf(buf, sizeof(buf), "EXIT\n");
		break;

	default:
		break;
	}

	if ( send(soc, buf, strlen(buf), 0) < 0 ) {
		perror("send");
	}
}

int recv_response(int soc, Command* cmd)
{
	char buf[BUFSIZE];
	char c, *p;
	int count, status;
	char** vector;

	p = buf;
	while ( recv(soc, &c, 1, 0) >= 0 ) {
		if ( c == '\n' ) {
			*p = '\0';
			break;
		}
		*p = c;
		p++;
	}

	if ( string_split(buf, ' ', &count, &vector) < 0 ) return 0;
	status = strtol(vector[0], NULL, 10);

	printf("%s\n", buf);

	if ( status / 100 == 2 ) { // 200番台
		if ( status == 201 ) cmd->size = strtol(vector[2], NULL, 10);
		else if ( status == 203 ) return -1;
		return 1;
	}
	else {
		return 0;
	}
}
