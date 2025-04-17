#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define MAX_CLNT 256

typedef struct {
    int sock;
    char name[NAME_SIZE];
} client_t;

client_t *clients[MAX_CLNT];
int clnt_cnt = 0;
pthread_mutex_t mutx;

void *handle_clnt(void *arg);
void send_msg_all(char *msg);
void error_handling(char *msg);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;
        int option = 1;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option , sizeof(option));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        client_t *clnt = malloc(sizeof(client_t));
        clnt->sock = clnt_sock;
        strcpy(clnt->name, "anonymous");

        pthread_mutex_lock(&mutx);
        clients[clnt_cnt++] = clnt;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)clnt);
        pthread_detach(t_id);
    }
    close(serv_sock);
    return 0;
}

void *handle_clnt(void *arg) {
    client_t *clnt = (client_t *)arg;
    int str_len;
    char msg[BUF_SIZE], buf[BUF_SIZE + NAME_SIZE];

    while (1) {
        char menu[] =
            "===========================\n"
            "1. chatting room\n"
            "2. name setting\n"
            "3. exit\n";
        write(clnt->sock, menu, strlen(menu));

        str_len = read(clnt->sock, msg, BUF_SIZE - 1);
        if (str_len <= 0) break;
        msg[strcspn(msg, "\r\n")] = 0;

        if (strcmp(msg, "1") == 0) {
            sprintf(buf, "%s joined the chat.\n", clnt->name);
            send_msg_all(buf);

            while (1) {
                str_len = read(clnt->sock, msg, BUF_SIZE - 1);
                if (str_len <= 0) break;
                msg[strcspn(msg, "\r\n")] = 0;
                                printf("returnmenu << insert ");
                if (strcmp(msg, "returnmenu") == 0) break;

                sprintf(buf, "%s: %s\n", clnt->name, msg);
                send_msg_all(buf);
            }
        } else if (strcmp(msg, "2") == 0) {
            char prompt[] = "Enter new name: ";
            write(clnt->sock, prompt, strlen(prompt));

            str_len = read(clnt->sock, msg, NAME_SIZE - 1);
            if (str_len <= 0) break;
            msg[strcspn(msg, "\r\n")] = 0;
            strcpy(clnt->name, msg);

                        char flush_buf[BUF_SIZE];
                        while((str_len = read(clnt->sock, flush_buf, BUF_SIZE - 1)) > 0){
                                flush_buf[str_len] = 0;
                                if(strchr(flush_buf, '\n') || strchr(flush_buf, '\r')) break;
                        }

            sprintf(buf, "Name changed to %s.\n", clnt->name);
            write(clnt->sock, buf, strlen(buf));
        } else if (strcmp(msg, "3") == 0) {
            break;
        } else {
            char error[] = "Invalid menu. Try again.\n";
            write(clnt->sock, error, strlen(error));
        }
    }

    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; ++i) {
        if (clients[i] == clnt) {
            for (int j = i; j < clnt_cnt - 1; ++j)
                clients[j] = clients[j + 1];
            clnt_cnt--;
            break;
        }
    }
    pthread_mutex_unlock(&mutx);

    sprintf(buf, "%s has left.\n", clnt->name);
    send_msg_all(buf);

    close(clnt->sock);
    free(clnt);
    return NULL;
}

void send_msg_all(char *msg) {
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; ++i)
        write(clients[i]->sock, msg, strlen(msg));
    pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

