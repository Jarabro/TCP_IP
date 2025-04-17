#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char msg[BUF_SIZE];
    int str_len;

    if(argc != 3) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_msg, (void *)&sock);
    pthread_detach(recv_thread);

    while(1) {
        // 서버로부터 메뉴를 읽어서 출력
        str_len = read(sock, msg, BUF_SIZE - 1);
        if(str_len <= 0) break;
        msg[str_len] = 0;
        printf("%s", msg);

        // 사용자 입력 전송
        fgets(msg, BUF_SIZE, stdin);
        write(sock, msg, strlen(msg));

        if(strncmp(msg, "1", 1) == 0) {
            // 채팅방 입장
            while(1) {
                str_len = read(sock, msg, BUF_SIZE - 1);
                if(str_len <= 0) break;
                msg[str_len] = 0;
                printf("%s", msg);

                fgets(msg, BUF_SIZE, stdin);
                if(strncmp(msg, "returnmenu", 10) == 0) {
                    write(sock, msg, strlen(msg));
                    break;
                }
                write(sock, msg, strlen(msg));
            }
        }
        else if(strncmp(msg, "2", 1) == 0) {
            // 닉네임 변경
            str_len = read(sock, msg, BUF_SIZE - 1);
            if(str_len <= 0) break;
            msg[str_len] = 0;
            printf("%s", msg);

            fgets(msg, BUF_SIZE, stdin);
            write(sock, msg, strlen(msg));

            str_len = read(sock, msg, BUF_SIZE - 1);
            if(str_len <= 0) break;
            msg[str_len] = 0;
            printf("%s", msg);
        }
        else if(strncmp(msg, "3", 1) == 0) {
            // 종료
            break;
        }
        else {
            // 잘못된 입력
            str_len = read(sock, msg, BUF_SIZE - 1);
            if(str_len <= 0) break;
            msg[str_len] = 0;
            printf("%s", msg);
        }
    }

    close(sock);
    return 0;
}

void * send_msg(void * arg) {
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    while(1) {
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void * recv_msg(void * arg) {
    int sock = *((int*)arg);
    char msg[BUF_SIZE];
    int str_len;
    while((str_len = read(sock, msg, sizeof(msg) - 1)) > 0) {
        msg[str_len] = 0;
        fputs(msg, stdout);
        fflush(stdout);
    }
    return NULL;
}

void error_handling(char * msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
