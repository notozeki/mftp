#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "analyze.h"
#include "communi.h"
#include "io.h"
#include "../mftpd/command.h"

#define BUFSIZE 256

int client_socket(char* host, int portno)
{
	int soc, err;
	char portno_buf[BUFSIZE];
	struct addrinfo hints, *ai;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP

	// アドレス情報を生成
	snprintf(portno_buf, sizeof(portno_buf), "%d", portno);
	if ( (err = getaddrinfo(host, portno_buf, &hints, &ai)) != 0 ) {
		fprintf(stderr, "getaddrinfo(): %s", gai_strerror(err));
		goto ret_invalid_code;
	}

	// ソケットを生成
	if ( (soc = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 ) {
		perror("socket");
		goto free_ai;
	}

	// こねくと！
	if ( connect(soc, ai->ai_addr, ai->ai_addrlen) < 0 ) {
		perror("connect");
		goto close_soc;
	}

	freeaddrinfo(ai);
	return soc;

 close_soc:
	close(soc);
 free_ai:
	freeaddrinfo(ai);
 ret_invalid_code:
	return -1;
}

void send_recv_loop(int soc)
{
	char buf[BUFSIZE];
	char* p;
	Command cmd;
	int occur_io;

	while ( 1 ) {
		// プロンプト表示
		fprintf(stderr, "mftp> ");

		// 入力を得る
		fgets(buf, sizeof(buf), stdin);
		if ( (p = strpbrk(buf, "\r\n")) != NULL ) {
			*p = '\0'; // 改行を削除
		}
		
		// 入力解析
		if ( input_analyze(buf, &cmd) < 0 ) {
			continue;
		}
		
		// リクエスト送信
		send_request(soc, &cmd);
		// 返答受信
		if ( (occur_io = recv_response(soc, &cmd)) < 0 ) {
			break;
		}

		// データ送受信
		if ( occur_io ) {
			data_io(soc, &cmd);
		}
	}
}

int main(int argc, char* argv[])
{
	char* host;
	int port;

	if ( argc < 3 ) {
		fprintf(stderr, "Usage: %s host portno\n", argv[0]);
		return -1;
	}
	host = argv[1];
	port = strtol(argv[2], NULL, 10);

	int soc;

	// サーバに接続
	if ( (soc = client_socket(host, port)) < 0 ) {
		fprintf(stderr, "socket error\n");
		return -1;
	}
	
	send_recv_loop(soc);

	close(soc);
	return 0;
}
