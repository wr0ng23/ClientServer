#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <stdlib.h>
#include <termios.h>
#include "../circleAndPoint.h"

int sock;

void f_obr1(int sig);
void f_obr2(int sig);

int main() {
    int codeErr, ans_len = 0;
    const char server_host[] = "localhost";
    struct hostent *h;
    struct sockaddr_in client_addr, server_addr;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = f_obr1;
    sigaction(SIGINT, &sa, 0);

    struct sigaction sa2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_handler = f_obr2;
    sigaction(SIGPIPE, &sa2, 0);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Error when socket was in process of creating!\n");
        exit(EXIT_FAILURE);
    }
    memset((char *)&client_addr, '\0', sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = CLIENT_PORT;

    codeErr = bind(sock, (struct sockaddr*)&client_addr, sizeof(client_addr));
    memset((char *)&client_addr, '\0', sizeof(server_addr));

    h = gethostbyname(server_host);
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr, h->h_addr_list[0], h->h_length);
    server_addr.sin_port = SERVER_PORT;

    codeErr = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (codeErr == -1) {
        printf("Connection failed!\n");
        exit(EXIT_FAILURE);
    }

    // Getting authentication message
    char messageAboutPass[SIZE];
    ans_len = recv(sock, messageAboutPass, sizeof(messageAboutPass), 0);
    printf("%s", messageAboutPass);
    //write(1, messageAboutPass, ans_len);

    // Entering a password
    char password[SIZE];
    memset((char *)&password, '\0', sizeof(password));
    struct termios defresett, newrsett;
    tcgetattr(fileno(stdin), &defresett);
    newrsett = defresett;
    newrsett.c_lflag &= ~ECHO;

    if (tcsetattr(fileno(stdin), TCSAFLUSH, &newrsett) != 0) {
        printf("\nError when hidding password\n");
        exit(EXIT_FAILURE);
    } else {
        fgets(password, sizeof(password), stdin);
        tcsetattr(fileno(stdin), TCSANOW, &defresett);
    } 
    size_t len = strlen(password);
    password[len - 1] = '\0';

    // Send password to server
    send(sock, password, sizeof(password), 0);

    // Getting answer about the correctness  of entered password
    char answerAboutPass[SIZE];
    ans_len = recv(sock, answerAboutPass, sizeof(answerAboutPass), 0);
    printf("%s", answerAboutPass);
    //write(1, answerAboutPass, ans_len);

    if (strcmp(answerAboutPass, "\nYou entered a wrong password!\n") == 0) {
        //shutdown(sock, SHUT_RDWR);
        close(sock);
        exit(EXIT_SUCCESS);
    }
    
    char messageFromServer[SIZE];
    char messageForServer[SIZE];
    struct point point;
    struct circle circle;

    while (1) {
        // Entering message for sendig in to server
        //memset((char *)&messageForServer, '\0', sizeof(messageForServer));
        printf("\nEnter the coordinates of the point\n");
        printf("Enter coordinate x of point: ");
        scanf("%f", &point.x);
        printf("Enter coordinate y of point: ");
        scanf("%f", &point.y);
        printf("\nEnter coordinate x of circle: ");
        scanf("%f", &circle.x);
        printf("Enter coordinate y of circle: ");
        scanf("%f", &circle.y);
        printf("Enter radius of circle: ");
        scanf("%f", &circle.r);
        //fgets(messageForServer, sizeof(messageForServer), stdin);
        send(sock, &point, sizeof(point), 0);
        send(sock, &circle, sizeof(circle), 0);
        /* if (strcmp(messageForServer, "exit\n") == 0) {
            //sleep(1);
            //shutdown(sock, SHUT_RDWR);
            close(sock);
            break;
        } */

        // Getting result from server
        memset((char *)&messageFromServer, '\0', sizeof(messageFromServer));
        ans_len = recv(sock, messageFromServer, sizeof(messageFromServer), 0);
        printf("%s", messageFromServer);
        //write(1, messageFromServer, ans_len);
    }

    return 0;
}

void f_obr1(int sig) {
    if (sig == SIGINT) {
        //shutdown(sock, SHUT_RDWR);
        close(sock);

        printf("\nExit from client cause SIGINT!\n");
        exit(EXIT_SUCCESS); 
    }
}

void f_obr2(int sig) {
    if (sig == SIGPIPE) {
        //shutdown(sock, SHUT_RDWR);
        close(sock);

        printf("Exit from client cause SIGPIPE!\n");
        exit(EXIT_SUCCESS); 
    }
}
