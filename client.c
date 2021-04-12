#include "cfunction.h"


int main(int argc, char* argv[]) {

    char *o_arg = calloc(64, sizeof(char));
    float tempo = 0;
    int storage=0;
    char *host = calloc(32, sizeof(char));
    char *port_temp = calloc(32, sizeof(char));
    char addrstr[16];

    getOpt(argc, argv, &storage, &tempo, o_arg);
    in_port_t port=isAddrOk(o_arg, port_temp, host);
    calcvalue(&storage, &tempo);
    struct hostent *he;
    he=gethostbyname(host);
    strcpy(host, inet_ntoa(*(struct in_addr*)he->h_addr));
    //printf("storage %d, tempo %f, decompose %f, host %s, port %d \n", storage, tempo, decompose, host, port);
    struct timespec start;

        while (1) {

            struct sockaddr_in addresstruct;
            clock_gettime(CLOCK_REALTIME, &start);
            struct timespec  stop, time_start, time_stop, time_rel;
            addresstruct.sin_family = AF_INET;
            addresstruct.sin_port = htons(port);
            socklen_t size=sizeof(addresstruct);
            int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_fd == -1) {
                fprintf(stdout, "client cannot create socket");
                exit(2);
            }
            int res = inet_aton(host, &addresstruct.sin_addr);
            if (!res) {
                fprintf(stdout, "uncorret serwer addres");
                exit(2);
            }
            int proba = 11;
            while (--proba) {
                if (connect(sock_fd, (struct sockaddr *) &addresstruct, sizeof(addresstruct)) != -1) break;
            }
            getsockname(sock_fd, (struct sockaddr*)&addresstruct, &size);
            inet_ntop(AF_INET, &addresstruct.sin_addr, addrstr, sizeof(addrstr));

            clock_gettime(CLOCK_MONOTONIC, &time_start);

            if (!proba) {
                fprintf(stdout, "connection not accepted");
                exit(2);
            }

            char buf[1024];
            int r;
            for (int i = 0; i < 13;) {
                if ((r=recv(sock_fd, buf, 1024, 0)) == 1024) {
                    i++;
                    owning+=1024;
                }else if(r==0) exit(1);
                if(i==1) clock_gettime(CLOCK_MONOTONIC, &time_rel);

                sleep_after_comsumpcion(tempo);
            }

            clock_gettime(CLOCK_MONOTONIC, &time_stop);
            char* eachblock=calloc(150, sizeof(char));
            sprintf(eachblock, "\nconnection and first receive %f \nfirst portion and close connection %f\nadres: %s port: %d\n",
                    calc_time(time_rel, time_start), calc_time(time_stop, time_rel), inet_ntoa(addresstruct.sin_addr), ntohs(addresstruct.sin_port));

            int e=on_exit(exitfunction, (void*)eachblock);
            if(e!=0){
                fprintf(stdout, "cannot set exit function");
                exit(1);
            }

            clock_gettime(CLOCK_REALTIME, &stop);
            rubbish(calc_time(stop, start));
            start.tv_nsec=stop.tv_nsec;
            start.tv_sec=stop.tv_sec;

            if(owning >= storage-13*1024){
                if (close(sock_fd) == -1) {
                    fprintf(stdout, "\nklopoty\n");
                    exit(1);
                }
                raport();
                exit(0);
            }
        }

    }

