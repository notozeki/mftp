#ifndef REQUEST_H
#define REQUEST_H

#include "command.h"

/* ==== mini-ftp request プロトコル ====
 *   get /path/to/source\n
 *   put /path/to/destination size\n
 *   dir /psth/to/dir\n
 *   それぞれOKが返ればデータを送受信する。
 *
 *
 * ==== mini-ftp response プロトコル ====
 *   ready\n
 *   ok size\n
 *   bad\n
 */
int request_analyze(char* request, Command* cmd);

#endif // REQUEST_H
