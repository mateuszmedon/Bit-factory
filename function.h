#ifndef SERWER_FUNCTION_H

#define SERWER_FUNCTION_H
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/timerfd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//#include </usr/include/linux/fcntl.h>

int desc[200]={0};
int permits[200]={0};
int zmarnowano[200]={0};
int wydano=0;
struct addrinfo clients[200];

void getOpt(int argc, char* argv[], char* o_arg, float *tempo);
in_addr_t isAddrOk(char* o_arg,  char* port, char* host);
void register_addr(int sock_fd, char* host, in_port_t port);
int create_soc();
void fabryka(int pipefd, float tempo, char ASCI);
char* generate(char ASCI);
void can_read(int pipefd, int fd);
void can_write(int pipefd, int fd);
int can_close( int fd, int* flag);
void wasted(int fd, int pipefd);
int create_and_set_timer();
void get_raport(int pipefd, int size, int* prev);


void get_raport(int pipefd, int size, int* prev){
    int zajetosc;
    long pipe_size = (long)fcntl(pipefd, 1024+8); //1024+8 = GETPIPESZ
    struct timespec val;
    clock_gettime(CLOCK_REALTIME, &val);
    ioctl(pipefd, FIONREAD, &zajetosc);

    double Prc=(((double)zajetosc/(double)pipe_size)*100);
    int prod=zajetosc-*prev;
    if(prod<0) prod=0;
    printf("\n**********************\nTS: "
           "%ld %ld \nzajętość: %f %% pojemność: %d\nliczba klientów: %d"
           " przepływ: %d\n**********************\n\n",
           val.tv_sec, val.tv_nsec, Prc, zajetosc, size, prod-wydano*640);
    *prev=zajetosc;
    wydano=0;
}

int create_and_set_timer()
{
    int timer_fd=timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    if(timer_fd==-1) {
        fprintf(stdout, "timer create error");
        exit(1);
    }
    struct timespec timeForTimer = { .tv_sec = 5, .tv_nsec = 0};
    struct itimerspec new={ .it_interval = timeForTimer, .it_value = timeForTimer };
    if(timerfd_settime(timer_fd,  0, &new, NULL)==-1){
        fprintf(stdout, "settimer error");
        exit(1);
    }
    return timer_fd;
}


void wasted(int fd, int pipefd){
    char buf[1024];
    for(int j=desc[fd]; j<13; j++) read(pipefd, buf, 1024);
    zmarnowano[fd]=(13-desc[fd])*1024;
    printf("############\nzmarnowano %d \n", zmarnowano[fd]);
    desc[fd]=13;
}

int can_close( int fd, int* flaga) {

    if (desc[fd] == 13) {
        desc[fd] = 0;
        close(fd);
        *flaga = 1;
        permits[fd] = 0;
        zmarnowano[fd]=0;
        struct timespec timer;
        clock_gettime(CLOCK_REALTIME, &timer);
        printf("#############\nTS:%ld %ld \n#############\n\n", timer.tv_sec, timer.tv_nsec);
        return -1;
    }
    return fd;
}

void can_write(int pipefd, int fd){
    if(permits[fd]==1){
        int val;
        char buf[1024];
        read(pipefd, buf, 1024);
        if ((val = send(fd, buf, 1024, 0)) == -1) {
           printf("dsadsa");
           exit(3);
        }else {
            desc[fd]++;
            wydano++;
        }
      //  fprintf(stderr, "%d %d %d \n", val, desc[fd], fd);
    }
}

void can_read(int pipefd,  int fd){
    int zajetosc;
    if (ioctl(pipefd, FIONREAD, &zajetosc) != -1) {
        if (zajetosc > 1024 * 13) {
            permits[fd] = 1;
        } else {
            permits[fd] = 0;
        }
    }
}

char* generate(char ASCI){
    char* buf=calloc(640, sizeof(char));
    memset(buf, ASCI, 640);
  //  printf("%s\n", buf);
   // fprintf(stderr, "  %c  ", ASCI);
    return buf;
}

void fabryka(int pipefd, float tempo, char ASCI){//blok 640 bajtów tempo * 2662
    float onsec=tempo*2662;
    //ile na sekunde, zapisuje po bloku, śpie po sekundzie lub więcej jeżeli więcej zapisze niż na sec
    struct timespec req, rem;
    int written=0;
    int check;

    struct timespec time={time.tv_nsec=0, time.tv_sec=0};
    while(1){
        req.tv_sec=1, req.tv_nsec=0;
        if(ASCI>122) ASCI=65;
        if(ASCI>90 && ASCI<97) ASCI=97;
        char* buf=generate(ASCI);
        ASCI++;
        written+=check=write(pipefd, buf, 640);

        if(written>=(int)onsec){
            int wr=written-(int)onsec;
            float ad_sleep=wr/onsec;
            req.tv_sec=req.tv_sec+(int)ad_sleep;
            req.tv_nsec=req.tv_nsec+(ad_sleep-(int)ad_sleep)*1000000000;
            nanosleep(&req, &rem);
            written=0;
        }
    }
}

int create_soc(){
    int on=1;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        fprintf(stderr, "cannot create socket");
        exit(1);
    }
    int res = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &on, sizeof(on));
    if (res < 0) {
        perror("setsockopt() failed");
        close(sock_fd);
        exit(-1);
    }

    res = ioctl(sock_fd, FIONBIO, (char *) &on);
    if (res < 0) {
        perror("ioctl() failed");
        close(sock_fd);
        exit(-1);
    }
    return sock_fd;
}


void register_addr(int sock_fd, char* host, in_port_t port){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family= AF_INET;
    int res;
    int result = inet_aton(host,&addr.sin_addr);
    if( ! result ) {
        fprintf(stderr,"uncorrect addres: %s\n",host);
        exit(1);
    }
    addr.sin_port= htons(port);
    res = bind(sock_fd,(struct sockaddr *)&addr, sizeof(addr));
    if (res < 0)
    {
        perror("bind() failed");
        close(sock_fd);
        exit(-1);
    }
    res = listen(sock_fd, 32);
    if (res < 0) {
        perror("listen() failed");
        close(sock_fd);
        exit(-1);
    }
}

void getOpt(int argc, char* argv[], char* o_arg, float *tempo) {
    if(argc<2 || argc>4)
    {
        fprintf(stderr, "Not enough args %s\n", argv[0]);
        exit(2);
    }
    char* endptr;
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                *tempo = strtof(optarg, &endptr);
                if(*tempo<=0)
                {
                    fprintf(stderr, "value <=0 is not acceptable");
                    exit(2);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-f float]\n", argv[0]);
                exit(2);
        }
    }
    if(optind<argc)
    {
        strcpy(o_arg,argv[optind]);
    }
    else{
        fprintf(stderr, "Previous security doesnt work");
        exit(2);
    }
}

in_addr_t isAddrOk(char* o_arg,  char* port, char* host){
    sscanf(o_arg, "%[^:]:%s", host, port);
    if(strtol(port, NULL, 0) == 0)
    {
        strcpy(port, host);
        strcpy(host, "localhost");
    }
    return strtol(port, NULL, 0);
}


#endif //SERWER_FUNCTION_H
