#ifndef COMMAND_H
#define COMMAND_H

typedef enum {
	BAD,
	GET,
	PUT,
	LIST,
	EXIT,
} CommandType;

typedef struct {
	CommandType type;
	char* value;
	int size;
} Command;

#endif // COMMAND_H
