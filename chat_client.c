#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void *recv_msg(void * arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
int chatting_mode = 0;

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;

    pthread_t recv_thread;
    char msg[BUF_SIZE];
    int str_len;

    if (argc != 4) {
        printf("Usage : %s <IP> <Port> <Name>\n", argv[0]);
        exit(1);
    }

    snprintf(name, NAME_SIZE, "[%s]", argv[3]);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_create(&recv_thread, NULL, recv_msg, (void*)&sock);
    pthread_detach(recv_thread);

    while(1)
    {
        if (!chatting_mode) {
            // 메뉴 선택
            fgets(msg, BUF_SIZE, stdin);
            write(sock, msg, strlen(msg));

            if (strncmp(msg, "3", 1) == 0) break;
        }
        else {
            // 채팅 모드
            fgets(msg, BUF_SIZE, stdin);
            msg[strcspn(msg, "\r\n")] = 0;

            if (strcmp(msg, "returnmenu") == 0) {
                chatting_mode = 0;
                write(sock, msg, strlen(msg));
                continue;
            }

            char send_msg[BUF_SIZE + NAME_SIZE];
            snprintf(send_msg, sizeof(send_msg), "%s %s\n", name, msg);
            write(sock, send_msg, strlen(send_msg));
        }
    }

    close(sock);
    return 0;
}

void *recv_msg(void * arg)
{
    int sock = *((int*)arg);
    char msg[BUF_SIZE];
    int str_len;

    while((str_len = read(sock, msg, BUF_SIZE -1)) > 0)
    {
        msg[str_len] = 0;
        fputs(msg, stdout);

        // 채팅방 진입 안내 받으면 상태 변경
        if (strstr(msg, "Enter chat. Type 'returnmenu' to exit.") != NULL) {
            chatting_mode = 1;
        }
        // 이름 바뀌었을 때 name도 갱신
        else if (strstr(msg, "Name changed to ") != NULL) {
            char *newname = strstr(msg, "Name changed to ");
            if (newname) {
                snprintf(name, NAME_SIZE, "[%s]", newname + 16); // 16 = strlen("Name changed to ")
                name[strcspn(name, "\r\n")] = 0;
            }
        }
    }
    return NULL;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
