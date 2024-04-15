#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024
#define MAX_CLIENTS 10

//클라이언트와 통신하는 함수
/*
`void *`은 C 언어에서 일반적으로 사용되는 포인터 형식입니다.
여기서 `handle_client` 함수는 스레드의 시작 루틴으로 사용됩니다.
스레드의 시작 루틴은 `void *` 형식의 매개변수를 받아야 합니다.
그러나 실제로는 이 매개변수를 사용하지 않을 수도 있습니다. 때문에 `handle_client` 함수의 매개변수는 `void *` 타입으로 지정됩니다. 

이러한 형태는 일반적으로 스레드 함수에서 특정 데이터나 구조체를 전달할 필요가 있는 경우에 사용됩니다.
예를 들어, 여기서는 클라이언트 소켓 디스크립터를 스레드로 전달하여 해당 클라이언트와의 통신을 처리합니다.
이를 위해 `void *` 형식의 포인터를 사용하여 클라이언트 소켓 디스크립터를 전달하고, 함수 내에서 이를 다시 적절한 형식으로 형변환하여 사용합니다.
*/
void *handle_client(void * arg){
	int cSockfd = *((int*)arg);
	char buf[BUFSIZE];

	//클라이언트와 메시지 주고받기
	while(1){
		// 클라이언트가 보낸 메시지 받기
		memset(buf, 0, sizeof(buf));
		
		// 메시지를 받는 것을 실패 한 경우
		if(read(cSockfd, buf, sizeof(buf)) == -1){
			// 시스템 오류 메시지를 출력하는 함수
			perror("read"); // read: Connection reset by peer
			close(cSockfd);
			pthread_exit(NULL); // 스레드 종료
		}

		// 받은 베시지 출력
		printf("Received from client: %s", buf);

		//받은 메시지를 다시 클라이언트에게 보내기(답장)
		printf("Input Response: ");
		fgets(buf, sizeof(buf), stdin);
		write(cSockfd, buf, strlen(buf));
	}

	close(cSockfd);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	int sockfd, cSockfd, port;
	struct sockaddr_in servaddr, cliaddr;

	// typedef int socklen_t;
	//소켓 API함수에서 주소 구조체의 크기를 전달하는데 사용되는 데이터 타입
	socklen_t len;

	//  POSIX스레드를 나타내는 데이터 형식, 각 스레드의 고유한 식별자
	// fork()함수의 pid와 비슷한 듯
	pthread_t tid[MAX_CLIENTS];
	int i = 0;

	if(argc != 2){
		fprintf(stderr, "Usage: %s <port\n>", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);

	// 소켓 생성
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket");
		return 1;
	}

	// 서버 주소 설정
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// 소켓과 서버 주소 바인딩
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
		perror("bind");
		return 1;
	}
	/*
	   bind(): 소켓에 주소를 할당하는 역할
	   소켓을 로컬 주소와 포트에 바인딩하여 해당 주소와 포트로 들어오는 연결을 수락하거나 데이터를 받을 수 있도록 준비한다.
	   sockfd: 바인딩할 소켓의 파일 디스크립터입니다. socket() 함수로 생성한 소켓의 파일 디스크립터를 전달합니다.
	   addr: 바인딩할 주소 정보를 담고 있는 struct sockaddr 구조체의 포인터입니다. 보통 struct sockaddr_in 구조체를 사용합니다.
	   addrlen: addr 구조체의 크기를 나타내는 값입니다.
	 */

	// 연결 요청 대기열 설정
	if (listen(sockfd, MAX_CLIENTS) == -1){
		perror("listen");
		return 1;
	}
	/*
	   listen(): 소켓이 들어오는 연결을 수신할 수 있도록 설정하는 역할
	   sockfd: 연결을 대기할 소켓의 파일 디스크립터입니다. socket() 함수로 생성한 소켓의 파일 디스크립터를 전달합니다.
	   backlog: 연결 요청 대기 큐의 최대 길이를 지정합니다. 이는 동시에 처리 가능한 최대 연결 요청 수를 결정합니다.
	*/

	// 클라이언트 연결 수락 및 통신
	while(1){
		len = sizeof(cliaddr);
		cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
		if(cSockfd == -1){
			perror("accept");
			return 1;
		}

		// 새로운 스레드를 생성하여 handle_client함수 실행
		if (pthread_create(&tid[i++], NULL, handle_client, &cSockfd) != 0){
			perror("pthread_create");
			return 1;
		}
		/*
		   thread: 새로 생성된 스레드의 식별자를 저장할 변수의 포인터입니다.
		   attr: 스레드의 속성을 지정하는데 사용되는 옵션입니다. 보통 NULL로 설정하여 기본 속성을 사용합니다.
		   start_routine: 새로운 스레드가 실행할 함수의 포인터입니다.
		   arg: start_routine 함수에 전달될 인자입니다.
		*/


		// 스레드와 클라이언트 관리
		if(i >= MAX_CLIENTS){
			i = 0;
			while(i < MAX_CLIENTS){
				pthread_join(tid[i++], NULL);
			}
			i = 0;
		}
	}
	close(sockfd);
	return 0;
}
