#include <stdio.h>
#include <string.h>
#include "../mftpd/command.h"
#include "../mftpd/util.h"

int input_analyze(char* str, Command* cmd)
{
	int count;
	char** vector;
	FILE* fp;

	if ( string_split(str, ' ', &count, &vector) < 0 ) return -1;

	cmd->type = BAD;
	cmd->value = NULL;
	cmd->size = 0;

	if ( count < 1 ) goto end;

	if ( strcmp("get", vector[0]) == 0 ) {
		if ( count != 2 ) {
			fprintf(stderr, "usage: get filename\n");
			goto end;
		}
		cmd->type = GET;
		cmd->value = strdup(vector[1]);
	}
	else if ( strcmp("put", vector[0]) == 0 ) {
		if ( count != 2 ) {
			fprintf(stderr, "usage: put filename\n");
			goto end;
		}
		if ( (fp = fopen(vector[1], "r")) == NULL ) {
			perror("fopen");
			goto end;
		}
		cmd->type = PUT;
		cmd->value = strdup(vector[1]);
		fseek(fp, 0, SEEK_END);
		cmd->size = ftell(fp);
		fclose(fp);
	}
	else if ( strcmp("list", vector[0]) == 0 ) {
		if ( count > 2 ) {
			fprintf(stderr, "usage: list {dirname}\n");
			goto end;
		}
		if ( count == 1 ) {
			cmd->type = LIST;
			cmd->value = ".";
		}
		else {
			cmd->type = LIST;
			cmd->value = strdup(vector[1]);
		}
	}
	else if ( strcmp("exit", vector[0]) == 0 ) {
		if ( count != 1 ) {
			fprintf(stderr, "usage: exit\n");
			goto end;
		}
		cmd->type = EXIT;
	}
	else if ( strcmp("help", vector[0]) == 0 ) {
		printf("== mini-ftp client program ==\n");
		printf(" get filename   : download remote file to local\n");
		printf(" put filename   : upload local file to remote\n");
		printf(" list {dirname} : show list of remote directory\n");
		printf(" exit           : exit connection;\n");
		printf(" help           : show this help\n");
		goto end;
	}
	else {
		fprintf(stderr, "unknown command: %s\n", vector[0]);
		fprintf(stderr, "type 'help' to show help\n");
		goto end;
	}
	free_string_vector(count, vector);
	return 0;

 end:
	free_string_vector(count, vector);
	return -1;
}
