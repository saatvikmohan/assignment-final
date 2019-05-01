#include "PracticalSocket.h" // For Socket and SocketException
#include <iostream>          // For cerr and cout
#include <cstdlib>           // For atoi()
#include <string.h>
#include<stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


using namespace std;

//max size of buffer
static const int MAX_SIZE = 1500;

//flag is set to 1 if Ctrl C is detected
int flag = 0;

//signal handler sets flag to 1 if SIGINT detected
void signal_callback_handler(int signum)
{
    flag = 1;
}


int
main (int argc, char *argv[])
{
    if ((argc < 2) || (argc > 3))
    { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Echo String> [<Server Port>]"
             << endl;
        exit (1);
    }

    string servAddress = argv[1];            // First arg: server address

    unsigned short echoServPort = (argc == 3) ? atoi (argv[2]) : 7; //Second arg: Server Port

    try {
        printf("To connect to server, type 'connect localhost:1234 <saat> <applebee>'");
        std::string request;
        getline(std::cin, request);
        while (request != "connect localhost:1234 <saat> <applebee>")
        {
            printf("Wrong username or password\n");
            getline(std::cin, request);
        }
        printf("Successful connection to server.\n");
        TCPSocket sock (servAddress, echoServPort);
        signal(SIGINT, signal_callback_handler);

        for (;;) {
            printf("\nCS3281$\n");
            printf("To disconnect to server, type 'disconnect localhost:1234'.\n"
                   "To cancel process, do 'Ctrl C' and click enter\n");
            getline(std::cin, request);
            //if flag = 1 detected, request is custom "end" command
            if (flag == 1)
            {
                request = "end";
            }

            if (request == "disconnect localhost:1234")
            {
                printf("Disconnecting...\n");
                break;
            }

            //since this is the max size the server can receive
            if (request.length() > 99)
            {
                printf("Buffer overflow. Send a different command");
                continue;
            }

            //send request
            sock.send(request.c_str(), request.length());
            //buffer for the recv
            char reply[MAX_SIZE] = {0};
            int length = 0;
            if ((length = sock.recv(reply, MAX_SIZE - 1)) > 0)
            {
                std::cout << reply << std::endl;
                if (length == 1499)
                {
                    printf("\nBuffer overflow\n");
                    break;
                }
            }

            //after SIGINT did its work on the server side, exit out of the client
            if (request == "end")
            {
                printf("Exiting the client...\n");
                break;
            }
        }
    }
        catch (SocketException &e)
        {
            cerr << e.what () << endl;
            exit (1);
        }

        return 0;
}
