#ifndef CLIENT_CFUNCTION_H
#define CLIENT_CFUNCTION_H
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

float decompose=0;
int owning=0;

void getOpt(int argc, char* argv[], int* storage, float* tempo,  char* o_arg);
in_addr_t isAddrOk(char* o_arg,  char* port, char* host);
void calcvalue(int* storage, float* tempo);
void sleep_after_comsumpcion(float tempo);
//void catcher(int sig);
void exitfunction(int status, void* message);
double calc_time(struct timespec t2, struct timespec t1);
//void set_alarm();
void raport();
void rubbish(double time);

void rubbish(double time){
    owning=owning-(int)(time*decompose);
    if(owning<0) owning=0;
    printf("owning %d zjedzono %f \n", owning, time*decompose);
}

void raport(){
    char text[200];
    struct timespec real;
    clock_gettime(CLOCK_REALTIME, &real);
    int PID=getpid();
    sprintf(text, "%%%%%%%%%%%%\n PID: %d TS: %ld %ld\n", PID, real.tv_sec, real.tv_nsec);

    int rr=on_exit(exitfunction, (void*)text);
    if(rr!=0){
        fprintf(stderr, "cannot set exit function");
        exit(1);
    }
}


void exitfunction(int status, void* message){
    printf("%s", (char*)message);

}


double calc_time(struct timespec t2, struct timespec t1)
{
    unsigned long long int sec, nsec;
    if(t2.tv_sec==t1.tv_sec)
    {
        sec=0;
        nsec=t2.tv_nsec-t1.tv_nsec;
    }
    else
    {
        unsigned long long int h1=t1.tv_sec*1000000000+t1.tv_nsec;
        unsigned long long int h2=t2.tv_sec*1000000000+t2.tv_nsec;
        unsigned long long int wynik=h2-h1;
        sec=wynik/1000000000;
        nsec=wynik-sec*1000000000;
    }
    double res=sec+(double)nsec/1000000000;
    return res;
}

void sleep_after_comsumpcion(float tempo) {

    struct timespec sleeptime;
    struct timespec wake={wake.tv_sec=0, wake.tv_nsec=0};
    double tmp = (1024)/tempo;
    sleeptime.tv_sec = (int)tmp;
    sleeptime.tv_nsec = (tmp-(int)tmp)*1000000000;
    nanosleep(&sleeptime, &wake);
    if(wake.tv_nsec!=0 || wake.tv_sec!=0) exit(1);

}

void calcvalue(int* storage, float* tempo){

    *storage=*storage*30*1024;
    *tempo=*tempo*4435;
    decompose=decompose*819;
}

in_addr_t isAddrOk(char* o_arg,  char* port, char* host)
{
    sscanf(o_arg, "%[^:]:%s", host, port);
    if(strtol(port, NULL, 0) == 0)
    {
        strcpy(port, host);
        strcpy(host, "localhost");
    }
    return strtol(port, NULL, 0);
}

void getOpt(int argc, char* argv[], int* storage, float* tempo, char* o_arg){
    if((argc<7))
    {
        fprintf(stderr, "Not enough args %s\n", argv[0]);
        exit(2);
    }
    char* endptr;
    int opt;
    while ((opt = getopt(argc, argv, "c:p:d:")) != -1) {
        switch (opt) {
            case 'c':
                *storage = strtol(optarg, NULL, 0);
                if(*storage<=0)
                {
                    fprintf(stderr, "value <=0 is not acceptable");
                    exit(2);
                }
                break;
            case 'p':
                *tempo= strtof(optarg, &endptr);
                if(*tempo<=0){
                    fprintf(stderr, "value <=0 is not acceptable");
                    exit(2);
                }
                break;
            case 'd':
                decompose=strtof(optarg, &endptr);
                if(decompose<=0){
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

#endif