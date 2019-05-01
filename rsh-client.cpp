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

static const int MAX_SIZE = 1500;

int flag = 0;

void signal_callback_handler(int signum);

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

    unsigned short echoServPort = (argc == 3) ? atoi (argv[2]) : 7;

    try {
        printf("To connect to server, type 'connect ipaddress:port <username> <password>'");
        std::string request;
        getline(std::cin, request);
        while (request != "connect")
        {
            printf("Wrong username or password");
            getline(std::cin, request);
        }
        printf("Successful connection to server");
        TCPSocket sock (servAddress, echoServPort);
        signal(SIGINT, signal_callback_handler);

        for (;;) {
            printf("CS3281$\n");
            printf("To disconnect to server, type 'disconnect'. To cancel process, do 'Ctrl C' and click enter\n");
            getline(std::cin, request);
            if (flag == 1)
            {
                request = "end";
            }

            if (request == "disconnect")
            {
                printf("Disconnecting...");
                break;
            }
            if (request.length() > MAX_SIZE)
            {
                printf("Buffer overflow. Send a different command");
                break;
            }
            if (request == "\n")
            {
                printf("\n");
                continue;
            }

            sock.send(request.c_str(), request.length());
            char reply[MAX_SIZE] = {0};
            int length = 0;
            if ((length = sock.recv(reply, MAX_SIZE - 1)) > 0)
            {
                std::cout << reply << std::endl;
                if (length == 1499)
                {
                    printf("\nBuffer may have overflowed. Keep clicking enter to receive the rest of the message.\n");
                }
            }

            if (request == "end")
            {
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
