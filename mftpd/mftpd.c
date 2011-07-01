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

#include "request.h"
#include "response.h"
#include "io.h"
#include "command.h"

#define BUFSIZE 256

int server_socket(int portno)
{
	int err, soc, opt;
	char portno_buf[BUFSIZE];
	char host_buf[BUFSIZE];
	char serv_buf[BUFSIZE];
	struct addrinfo hints, *ai;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // server

	// アドレス情報を生成
	snprintf(portno_buf, sizeof(portno_buf), "%d", portno);
	if ( (err = getaddrinfo(NULL, portno_buf, &hints, &ai)) != 0 ) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		goto ret_invalid_code;
	}

	// ポート番号取得（デバッグ用出力）
	if ( (err = getnameinfo(ai->ai_addr, ai->ai_addrlen, 
				host_buf, sizeof(host_buf), 
				serv_buf, sizeof(serv_buf), 
				NI_NUMERICHOST | NI_NUMERICSERV)) != 0 ) {
		fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(err));
		goto free_ai;
	}
	fprintf(stderr, "host=%s\n", host_buf);
	fprintf(stderr, "port=%s\n", serv_buf);

	// ソケットを生成
	if ( (soc = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 ) {
		perror("socket");
		goto free_ai;
	}

	// ソケットのオプションを設定
	opt = 1;
	if ( setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ) {
		perror("setsockopt");
		goto close_soc;
	}

	// ソケットにアドレスをbind
	if ( bind(soc, ai->ai_addr, ai->ai_addrlen) < 0 ) {
		perror("bind");
		goto close_soc;
	}

	// listen
	if ( listen(soc, SOMAXCONN) < 0 ) {
		perror("listen");
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

void send_recv_loop(int soc) // このループの中で勝手に死んではならない
{
	ssize_t len;
	char buf[BUFSIZE];
	char* p;
	Command cmd;
	int occur_io;

	while ( 1 ) {
		// リクエスト受信
		if ( (len = recv(soc, buf, sizeof(buf), 0)) < 0 ) {
			perror("recv");
			break;
		}
		if ( len == 0 ) { // EOFを受信（通信の終わり）
			fprintf(stderr, "recv: EOF\n");
			break;
		}
		// 受信データを文字列として表示
		buf[len] = '\0';
		if ( (p = strpbrk(buf, "\r\n")) != NULL ) {
			*p = '\0';
		}
		fprintf(stderr, "[client]%s\n", buf);
		// リクエスト解析
		request_analyze(buf, &cmd);

		// 応答
		occur_io = make_response(buf, sizeof(buf), &cmd);
		//strncpy(buf, "OK.\n", sizeof(buf));
		if ( (len = send(soc, buf, strlen(buf), 0)) < 0 ) {
			perror("send");
			break;
		}
		if ( cmd.type == EXIT ) break;
		// データ送受信
		if ( occur_io ) {
			data_io(soc, &cmd);
		}
		
	}
}

int main(int argc, char* argv[])
{
	int port;
	
	if ( argc < 2 ) {
		fprintf(stderr, "Usage: %s portno {document_root}\n", argv[0]);
		return -1;
	}
	port = strtol(argv[1], NULL, 10);
	// ワーキングディレクトリ変更
	if ( argc == 3 ) {
		if ( chdir(argv[2]) < 0 ) {
			perror("chdir");
			return -1;
		}
	}

	
	int soc, acc, err;
	char host_buf[BUFSIZE];
	char serv_buf[BUFSIZE];
	struct sockaddr_storage from;
	socklen_t len;

	// 通信受付ソケットを開く
	acc = server_socket(port);
	if ( acc < 0 ) {
		fprintf(stderr, "socket error\n");
		return -1;
	}

	// 通信受付ループ
	while ( 1 ) {
		len = (socklen_t)sizeof(from);
		if ( (soc = accept(acc, (struct sockaddr*)&from, &len)) < 0 ) {
			perror("accept");
			close(acc);
			return -1;
		}

		// デバッグ用出力
		if ( (err = getnameinfo((struct sockaddr*)&from, len,
					host_buf, sizeof(host_buf),
					serv_buf, sizeof(serv_buf),
					NI_NUMERICHOST | NI_NUMERICSERV)) != 0 ) {
			fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(err));
			break;
		}
		fprintf(stderr, "accept: %s:%s\n", host_buf, serv_buf);

		send_recv_loop(soc);
		close(soc);
	}

	close(acc);
	return 0;
}
