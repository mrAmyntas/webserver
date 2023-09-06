#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <time.h>

#define PORT 8000

int main(int argc, char const *argv[])
{
    if (argc != 2)
        return 0;
    int sock = 0, n1 = fork(), n2 = fork(), n3 = fork(), n4 = fork(), totalRequests = 0;
    struct sockaddr_in serv_addr;
	char hello[500] = "GET /cgi-bin/uploadFile.pl HTTP/1.1\r\nHost: localhost\r\n\r\n";
    time_t start, now;
    float runTime = std::stof(argv[1]), elapsedTime = 0;

    time(&start);
    while (elapsedTime < runTime)
    {
        char buffer[6000] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return -1;
        }   
        send(sock, hello, sizeof(hello) , 0);
        std::cout << "\033[33m\033[1m" << "message sent\033[0m\n";
	    recv(sock, buffer, 6000, 0);
        std::cout << "\033[32m\033[1m" << "message received\033[0m\n";
        now = time(NULL);
        elapsedTime = difftime(now, start);
        if (n1 && n2 && n3 && n4)
            totalRequests++;
    }
    if (n1 && n2 && n3 && n4)
    {
        wait(NULL);
        std::cout << "\033[31m\033[1m" << "Requests sent to webserver: " << totalRequests * 5 << "\033[0m" << std::endl;
    }
    return 0;
}

// int main(int argc, char const *argv[])
// {
//     int sock = 0;
//     struct sockaddr_in serv_addr;
// 	char hello[500] = "DELETE /uploads/12345 HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
//     char buffer[6000] = {0};
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//     {
//         printf("\n Socket creation error \n");
//         return -1;
//     }
//     memset(&serv_addr, '0', sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(PORT);
//     // Convert IPv4 and IPv6 addresses from text to binary form
//     if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
//     {
//         printf("\nInvalid address/ Address not supported \n");
//         return -1;
//     }
//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
//     {
//         printf("\nConnection Failed \n");
//         return -1;
//     }   
//     send(sock, hello, sizeof(hello) , 0);
//     printf("Hello message sent\n");
// 	recv(sock, buffer, 6000, 0);
//     return 0;
// }