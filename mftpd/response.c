#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "response.h"
#include "command.h"

// 次にデータのやりとりが発生する場合は真を返す
int make_response(char* buf, int size, Command* cmd)
{
	FILE* fp;
	DIR* dp;
	int file_size;
	ssize_t slen;

	int pipefd[2];
	pid_t child;
	char* argv[] = {"ls", "-l", cmd->value, NULL};

	switch ( cmd->type ) {
	case BAD:
		strncpy(buf, "400 Bad Request\n", size);
		return 0;
		break;

	case GET:
		if ( (fp = fopen(cmd->value, "r")) != NULL ) {
			if ( fseek(fp, 0, SEEK_END) != 0 ) {
				perror("fseek");
				strncpy(buf, "500 Internal Server Error\n", size);
				fclose(fp);
				return 0;
			}
			if ( (file_size = ftell(fp)) < -1 ) {
				perror("ftell");
				strncpy(buf, "500 Internal Server Error\n", size);
				fclose(fp);
				return 0;
			}
			snprintf(buf, size, "201 OK %d\n", file_size);
			fclose(fp);
		}
		else {
			perror("fopen");
			strncpy(buf, "404 Not Found\n", size);
			return 0;
		}
		break;

	case PUT:
		strncpy(buf, "202 Ready\n", size);
		break;

	case LIST:
		file_size = 0;
		if ( (dp = opendir(cmd->value)) != NULL ) {
			// 返答サイズを実測する（かなり無駄！）
			if ( pipe(pipefd) < 0 ) {
				perror("pipe");
				strncpy(buf, "500 Internal Server Error\n", size);
				return 0;
			}

			child = fork();
			if ( child ) {
				close(pipefd[1]);
				while ( (slen = read(pipefd[0], buf, sizeof(buf))) > 0 ) {
					file_size += slen;
					if ( slen < 0 ) {
						perror("send");
						strncpy(buf, "500 Internal Server Error\n", size);
						close(pipefd[0]);
						return 0;
					}
				}
				if ( slen < 0 ) {
					perror("read");
					strncpy(buf, "500 Internal Server Error\n", size);
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

			snprintf(buf, size, "201 OK %d\n", file_size);
			closedir(dp);
		}
		else {
			perror("opendir");
			strncpy(buf, "404 Not Found\n", size);
			return 0;
		}
		break;

	case EXIT:
		strncpy(buf, "203 BYE\n", size);
		break;

	default:
		break;
	}
	return 1;
}
