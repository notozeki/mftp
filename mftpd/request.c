#include <stdlib.h>
#include <string.h>
#include "request.h"
#include "util.h"

int request_analyze(char* request, Command* cmd)
{
	char** vector;
	int count, ret;

	if ( string_split(request, ' ', &count, &vector) < 0 ) return 0;

	// 既定値
	cmd->type = BAD;
	cmd->value = NULL;
	cmd->size = 0;
	ret = 0;

	if ( count < 1 ) goto end;

	if ( strcmp("GET", vector[0]) == 0 ) { // GETメソッド
		if ( count != 2 ) goto end;
		cmd->type = GET;
		cmd->value = strdup(vector[1]);
	}
	else if ( strcmp("PUT", vector[0]) == 0 ) { // PUTメソッド
		if ( count != 3 ) goto end;
		cmd->type = PUT;
		cmd->value = strdup(vector[1]);
		cmd->size = strtol(vector[2], NULL, 10);
	}
	else if ( strcmp("LIST", vector[0]) == 0 ) { // LISTメソッド
		if ( count == 1 ) {
			cmd->type = LIST;
			cmd->value = ".";
		}
		else if ( count == 2 ) {
			cmd->type = LIST;
			cmd->value = strdup(vector[1]);
		}
		else {
			goto end;
		}
	}
	else if ( strcmp("EXIT", vector[0]) == 0 ) { // EXITメソッド
		if ( count != 1 ) goto end;
		cmd->type = EXIT;
	}
	else { // bad request
		goto end;
	}
	ret = 1;
 end:
	free_string_vector(count, vector);
	return ret;
}
