#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "command.h"
#include "util.h"

#define BUFSIZE 1024

int data_io(int soc, Command* cmd)
{
	char buf[BUFSIZE];
	FILE* fp;
	ssize_t slen;
	size_t len;
	int i;

	int pipefd[2];
	pid_t child;
	char* argv[] = {"ls", "-l", cmd->value, NULL};

	switch ( cmd->type ) {
	case GET:
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

	case PUT:
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

	case LIST:
		if ( pipe(pipefd) < 0 ) {
			perror("pipe");
			return 0;
		}

		child = fork();
		if ( child ) {
			close(pipefd[1]);
			while ( (slen = read(pipefd[0], buf, sizeof(buf))) > 0 ) {
				slen = send(soc, buf, slen, 0);
				if ( slen < 0 ) {
					perror("send");
					close(pipefd[0]);
					return 0;
				}
			}
			if ( slen < 0 ) {
				perror("read");
				close(pipefd[0]);
				return 0;
			}
			close(pipefd[0]);
		}
		else if ( child == 0 ) {
			close(pipefd[0]);
			
			close(1);
			dup2(pipefd[1], 1);
			close(pipefd[1]);

			execvp(argv[0], argv);
			exit(0);
		}
		else {
			perror("fork");
			return 0;
		}
		break;

	default:
		break;
	}
	return 1;
}
