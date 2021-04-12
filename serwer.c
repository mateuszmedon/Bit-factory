#include "function.h"

int main(int argc, char *argv[]) {

    char *o_arg = calloc(64, sizeof(char));
    float tempo = 0;
    char *host = calloc(32, sizeof(char));
    char *port_temp = calloc(32, sizeof(char));
    struct pollfd descriptors[200];
    int end_server=0, flaga;
    int prev=0;
    getOpt(argc, argv, o_arg, &tempo);
    in_port_t port = isAddrOk(o_arg, port_temp, host);
    printf("host: %s port: %s port %d \n", host, port_temp, port);
    free(o_arg);

    int pipefd[2], pipe_prod[2];
    if (pipe(pipefd) == -1 || pipe(pipe_prod)==-1) {
        fprintf(stderr, "pipe failure");
        exit(2);
    }
    int c_pid = fork();
    if (c_pid == -1) {
        fprintf(stderr, "fork failure");
        exit(2);
    }
    if (c_pid == 0) {
        if (fcntl(pipe_prod[0], F_SETFL, O_NONBLOCK) < 0) exit(2);
        close(pipefd[1]); //close unused
        close(pipe_prod[1]);
        int sock_fd, new_socket, struct_num=2;
        sock_fd=create_soc();
        register_addr(sock_fd, host, port);
        memset(descriptors, 0, sizeof(descriptors));

        descriptors[0].fd = sock_fd;
        descriptors[0].events = POLLIN;

        int timer_fd=create_and_set_timer();
        descriptors[1].fd = timer_fd;
        descriptors[1].events=POLLIN;

        /*
         * 1.while do którego wchodzę
         * 2. jeżeli ponad 13 to robię */
        do {
            if(poll(descriptors, struct_num, -1) < 0){
                fprintf(stdout, "poll error");
                break;
            }
            int current_size = struct_num;
            for (int i = 0; i < current_size; i++) {
                if (descriptors[i].revents == 0)
                    continue;
                if (descriptors[i].revents != POLLOUT && descriptors[i].fd!= timer_fd && descriptors[i].fd!= sock_fd) {
                    //sytuacja gdy klient zrobi ctrl + C
                    wasted(descriptors[i].fd, pipefd[0]);
                }
                if (descriptors[i].fd == timer_fd) {
                    int a;
                    if (read(descriptors[i].fd, &a, 8) == -1) { //odczyt budzika
                        fprintf(stdout, "read error");
                    } else{ //raport co 5 sec
                        get_raport(pipefd[0],  current_size-2, &prev);
                    }
                } else {
                    if (descriptors[i].fd == sock_fd) {
                        do {
                            new_socket = accept(sock_fd, NULL, NULL);
                            if (new_socket < 0) {
                                if (errno != EWOULDBLOCK) {
                                    fprintf(stdout, "new sock error");
                                    end_server = 1;
                                }
                                break;
                            }
                            descriptors[struct_num].fd = new_socket;
                            descriptors[struct_num].events = POLLOUT;
                            struct_num++;
                        } while (new_socket != -1);
                    } else {
                        can_read(pipefd[0], descriptors[i].fd);
                        can_write(pipefd[0], descriptors[i].fd);
                        descriptors[i].fd = can_close(descriptors[i].fd, &flaga);
                    }

                    if (flaga) {
                        flaga = 0;
                        for (i = 0; i < struct_num; i++) {
                            if (descriptors[i].fd == -1) {
                                for (int j = i; j < struct_num; j++) {
                                    descriptors[j].fd = descriptors[j + 1].fd;
                                }
                                printf("\n\n\n jak często to jest \n");
                                i--;
                                struct_num--;
                            }
                        }
                    }
                }
            }
        } while (end_server == 0);
    }else {
        //rodzic
        close(pipefd[0]);
        close(pipe_prod[0]);
        fabryka(pipefd[1], tempo, 'A');
        //fabryka bitów
    }

    return 0;
}
