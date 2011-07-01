#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../mftpd/command.h"
#include "../mftpd/util.h"

#define BUFSIZE 1024

int data_io(int soc, Command* cmd)
{
	char buf[BUFSIZE];
	FILE* fp;
	ssize_t slen;
	size_t len;
	int i, n;

	switch ( cmd->type ) {
	case GET:
		if ( (fp = fopen(cmd->value, "w")) == NULL ) {
			perror("fopen");
			return 0;
		}
		for ( i = 0; i < cmd->size/sizeof(buf) + 1; i++ ) {
			if ( i == cmd->size/sizeof(buf) ) {
				len = cmd->size % sizeof(buf);
			}
			else {
				len = sizeof(buf);
			}
			slen = recv(soc, buf, len, 0);
			if ( slen < 0 ) {
				perror("recv");
				fclose(fp);
				return 0;
			}
			len = fwrite(buf, sizeof(char), slen, fp);
			if ( len < sizeof(char)*slen ) {
				if ( ferror(fp) ) {
					perror("fwrite");
					fclose(fp);
					return 0;
				}
			}
		}
		fclose(fp);
		break;

	case PUT:
		if ( (fp = fopen(cmd->value, "r")) == NULL ) {
			perror("fopen");
			return 0;
		}
		while ( !feof(fp) ) {
			len = fread(buf, sizeof(char), sizeof(buf), fp);
			if ( len < sizeof(char)*sizeof(buf) ) {
				if ( ferror(fp) ) {
					perror("fread");
					fclose(fp);
					return 0;
				}
			}
			slen = send(soc, buf, len, 0);
			if ( slen < 0 ) {
				perror("send");
				fclose(fp);
				return 0;
			}
		}
		fclose(fp);
		break;

	case LIST:
		n = sizeof(buf) - 1;
		for ( i = 0; i < cmd->size/n + 1; i++ ) {
			if ( i == cmd->size/n ) {
				len = cmd->size % n;
			}
			else {
				len = n;
			}
			slen = recv(soc, buf, len, 0);
			if ( slen < 0 ) {
				perror("recv");
				return 0;
			}
			buf[slen] = '\0';
			printf("%s", buf);
		}
		break;

	default:
		break;
	}
	return 1;
}
