#include <sys/types.h>
#include <netdb.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <stdlib.h>
#include "../circleAndPoint.h"

// Проверка принадлежности точки вещественной плоскости с координатами (X1, Y1) 
// к окружности, заданной координатами центра (X2, Y2) и радиусом R. 
// Реализовать проверку на ошибки: отрицательный радиус.

int sock1, sock2;

void f_obr1(int sig);
void f_obr2(int sig);
int isInsideTheCircle(struct point point, struct circle circle);

int main() {
    int ans_len, codeErr, total = 0;
    struct sockaddr_in server_addr, client_addr;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = f_obr1;
    sigaction(SIGINT, &sa, 0);

    struct sigaction sa2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_handler = f_obr2;
    sigaction(SIGPIPE, &sa2, 0);

    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock1 == -1) {
        printf("Error when socket was in process of creating!\n");
        exit(EXIT_FAILURE);
    }
    
    memset((char *)&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = SERVER_PORT;
    codeErr = bind(sock1, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (codeErr == -1) {
        printf("Error when bind function was called!\n");
        exit(1);
    }

    printf("Server is running...\n");
    codeErr = listen(sock1, 3);
    if (codeErr == -1) {
        printf("Error when listening function was activated!\n");
        exit(EXIT_FAILURE);
    }

    ans_len = sizeof(client_addr);
    sock2 = accept(sock1, (struct sockaddr*)&client_addr, &ans_len);
    if (sock2 == -1) {
        printf("Error when accept connection with sock2!\n");
        exit(EXIT_FAILURE);
    }

    // Request for password
    char msgToGetPass[] = "Enter password to enter in server: ";
    send(sock2, msgToGetPass, sizeof(msgToGetPass), 0);

    // Checking if password correct
    char password[SIZE];
    ans_len = recv(sock2, password, SIZE, 0);

    int resOfComparing = strcmp(password, "1234");
    if (resOfComparing == 0) {
        char messageAboutLogging[] = "\nYou were logged in to the server\n";
        send(sock2, messageAboutLogging, sizeof(messageAboutLogging), 0);

        char messageFromClient[SIZE];
        struct point point;
        struct circle circle;

        while (1) {
            //memset((char *)&messageFromClient, '\0', sizeof(messageFromClient));
            ans_len = recv(sock2, &point, sizeof(point), 0);
            ans_len = recv(sock2, &circle, sizeof(circle), 0);
            
            printf("%f\n", point.x);
            printf("%f\n", point.y);
            printf("%f\n", circle.x);
            printf("%f\n", circle.y);
            printf("%f\n", circle.r);
            int res = isInsideTheCircle(point, circle);
            
            if (res == 1) {
                char trueMessage[] = "\nYes, point in circle\n";
                send(sock2, trueMessage, sizeof(trueMessage), 0);
            } else if (res == 0) {
                char falseMessage[] = "\nNo, point not in circle\n";
                send(sock2, falseMessage, sizeof(falseMessage), 0);
            } else {
                char incorrectMessage[] = "\nRadius is negative\n";
                send(sock2, incorrectMessage, sizeof(incorrectMessage), 0);
            }
            /* if (strcmp(messageFromClient, "exit\n") == 0) {
                printf("Exit was entered!\n");
                //shutdown(sock2, SHUT_RDWR);
                close(sock1);
                break;
            } else {
                write(1, messageFromClient, ans_len);
                char defMessage[] = "Default message from server(enter \"exit\" to exit)\n";
                send(sock2, defMessage, sizeof(defMessage), 0);
            } */
        }

    } else {
        char messageAboutLogging[] = "\nYou entered a wrong password!\n";
        send(sock2, messageAboutLogging, sizeof(messageAboutLogging), 0);
        sleep(1);
        close(sock2);
        close(sock1); 
        exit(EXIT_SUCCESS);
    }

    return 0;
}

// (point.x-circle.x)^2+(point.y-circle.y)^2 <= (circle.r)^2
int isInsideTheCircle(struct point point, struct circle circle) {
    if (circle.r < 0) return -1;

    float xEx = (point.x - circle.x) * (point.x - circle.x);
    float yEx = (point.y - circle.y) * (point.y - circle.y);
    float res = xEx + yEx;
    float squareOfRadius = circle.r * circle.r;
    if (res <= squareOfRadius) return 1;
    else return 0;
}

void f_obr1(int sig) {
    if (sig == SIGINT) {
        //shutdown(sock2, SHUT_RDWR);
        close(sock1);
        
        printf("\nExit from server cause SIGINT!\n");
        exit(EXIT_SUCCESS); 
    }
}

void f_obr2(int sig) {
    if (sig == SIGPIPE) {
        //shutdown(sock2, SHUT_RDWR);
        close(sock1);  

        printf("\nExit from server cause SIGPIPE!\n");
        exit(EXIT_SUCCESS); 
    }
}