#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100

void error_handling(char * msg);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char msg[BUF_SIZE];
    int str_len;

    if (argc != 4) {
        printf("Usage : %s <IP> <Port> <Nickname>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    // 자동으로 닉네임 설정
    write(sock, "2\n", strlen("2\n"));
    str_len = read(sock, msg, BUF_SIZE - 1); msg[str_len] = 0;
    printf("%s", msg);
    write(sock, argv[3], strlen(argv[3]));
    str_len = read(sock, msg, BUF_SIZE - 1); msg[str_len] = 0;
    printf("%s", msg);

    while (1)
    {
        str_len = read(sock, msg, BUF_SIZE - 1);
        if (str_len <= 0) break;
        msg[str_len] = 0;
        printf("%s", msg);

        fgets(msg, BUF_SIZE, stdin);
        write(sock, msg, strlen(msg));

        if (strncmp(msg, "1", 1) == 0) {
            while (1) {
                str_len = read(sock, msg, BUF_SIZE - 1);
                if (str_len <= 0) break;
                msg[str_len] = 0;
                printf("%s", msg);

                fgets(msg, BUF_SIZE, stdin);
                write(sock, msg, strlen(msg));
                if (strncmp(msg, "returnmenu", 10) == 0)
                    break;
            }
        }
        else if (strncmp(msg, "2", 1) == 0) {
            str_len = read(sock, msg, BUF_SIZE - 1); msg[str_len] = 0;
            printf("%s", msg);
            fgets(msg, BUF_SIZE, stdin);
            write(sock, msg, strlen(msg));
            str_len = read(sock, msg, BUF_SIZE - 1); msg[str_len] = 0;
            printf("%s", msg);
        }
        else if (strncmp(msg, "3", 1) == 0) {
            break;
        }
        else {
            str_len = read(sock, msg, BUF_SIZE - 1);
            if (str_len <= 0) break;
            msg[str_len] = 0;
            printf("%s", msg);
        }
    }

    close(sock);
    return 0;
}

void error_handling(char * msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
