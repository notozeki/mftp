#ifndef COMMUNI_H
#define COMMUNI_H

#include "../mftpd/command.h"

void send_request(int soc, Command* cmd);
int recv_response(int soc, Command* cmd);

#endif // COMMUNI_H
