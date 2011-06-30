#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "response.h"
#include "command.h"

// 次にデータのやりとりが発生する場合は真を返す
int make_response(char* buf, int size, Command* cmd)
{
	FILE* fp;
	DIR* dp;
	int file_size;

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
		if ( (dp = opendir(cmd->value)) != NULL ) {
			strncpy(buf, "200 OK\n", size);
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
