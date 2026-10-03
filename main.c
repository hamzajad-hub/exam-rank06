#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
int max = 0 , next_id = 0 , ids[1024];
char msgs[1024][290000], wbuf[300000], rbuf[120000];
fd_set fds, rfds, wfds;
void sendtoall(int c, int sockfd){
    for(int i = 0 ; i <= max ; i++){
        if(FD_ISSET(i , &wfds)&& c != i && c != sockfd)
        send(i , wbuf, strlen(wbuf), 0);
    }
}
int main(int ac, char **av){
    if(ac != 2){
        write(2 , "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    struct sockaddr_in addr = {.sin_family = AF_INET , .sin_port = htons(atoi(av[1])), .sin_addr.s_addr = htonl(0x7F000001)};
    int sockfd = socket(AF_INET , SOCK_STREAM, 0);
    if(sockfd < 0 || bind(sockfd , (struct sockaddr *)&addr , sizeof(addr)) < 0 || listen(sockfd , 128) < 0){
        write(2, "Fatal error\n" , strlen("Fatal error\n"));
        exit(1);
    }
    FD_ZERO(&fds);
    max = sockfd;
    FD_SET(sockfd , &fds);
    while(1){
        rfds = wfds = fds;
        if(select(max + 1 , &rfds, &wfds, NULL , NULL) < 0 )continue;
        for(int s = 0 ; s <= max ; s++){
            if(!FD_ISSET(s, &rfds)) continue;
            if(s == sockfd){
                int c = accept(sockfd , NULL , NULL);
                if(c < 0) continue;
                max = (c > max) ? c : max ;
                ids[c] = next_id++;
                FD_SET(c , &fds);
                sprintf(wbuf, "server: client %d just arrived\n", ids[c]);
                sendtoall(c, sockfd);
                break;
            }
            int res = recv(s , rbuf, sizeof(rbuf), 0);
            if(res <= 0){
                sprintf(wbuf , "server: client %d just left\n", ids[s]);
                sendtoall(s, sockfd);
                FD_CLR(s, &fds);
                close(s);
                bzero(msgs[s], sizeof(msgs[s]));
                break;
            }
            for(int i =0 , j = strlen(msgs[s]); i < res ; i++, j++){
                msgs[s][j] = rbuf[i];
                if(msgs[s][j] == '\n'){
                    msgs[s][j] = '\0';
                    sprintf(wbuf, "client %d: %s\n", ids[s], msgs[s]);
                    sendtoall(s, sockfd);
                    bzero(msgs[s], sizeof(msgs[s]));
                    j = -1;
                }
            }
            break;
        }
    }
    return 0;
}