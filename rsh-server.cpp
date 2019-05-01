#include "PracticalSocket.h" // For Socket, ServerSocket, and SocketException
#include <iostream>          // For cerr and cout
#include <cstdlib>           // For atoi()
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sstream>


using namespace std;

pid_t pid2;

int flag = 0;

int MAX_SIZE = 100;

void handleConnection (TCPSocket *sock); // TCP client handling function

void execute(char* const args[], TCPSocket *sock);

void sigChld(int arg)
{
    printf ("From process %d, a child process has finished execution\n",
            getpid ());
    int status = 0;
    // Wait for any child process.
    int child_pid = 1;
    while (child_pid > 0)
    {
        child_pid = waitpid (-1, &status, WNOHANG);
        if (child_pid >= 0)
        {
            printf ("State of process %d changed - ", child_pid);
            if (WIFEXITED (status))
            {
                printf ("exited, status=%d\n", WEXITSTATUS (status));
            }
            else if (WIFSIGNALED (status))
            {
                printf ("killed by signal %d\n", WTERMSIG (status));
            }
        }
    }
    printf("\n");
}



int main (int argc, char *argv[])
{
  if (argc != 2)
    { // Test for correct number of arguments
      cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
      exit (1);
    }

  unsigned short echoServPort = atoi (argv[1]); // First arg: local port

  try
    {
      TCPServerSocket servSock (echoServPort); // Server Socket object
        for (;;)
        {                                       // Run forever
            handleConnection(servSock.accept()); // Wait for a client to connect
        }
    }
  catch (SocketException &e)
  {
      cerr << e.what () << endl;
      exit (1);
  }
    // NOT REACHED

    return 0;
}

void handleConnection(TCPSocket *sock)
{
    cout << "Handling client ";
    try
    {
        cout << sock->getForeignAddress () << ":";
    }
    catch (SocketException e)
    {
        cerr << "Unable to get foreign address" << endl;
    }
    try
    {
        cout << sock->getForeignPort ();
    }
    catch (SocketException e)
    {
        cerr << "Unable to get foreign port" << endl;
    }
    cout << endl;

    pid_t pid = fork();

    if (pid > 0) { //parent
        int status = 0;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) == 0)
            printf("*** Child exited sucessfully\n");
        else
            printf("*** Child exited with %d\n", WEXITSTATUS(status));
    }
    else if (pid < 0) { //failed fork
        printf("*** Fork failed\n");
    }

    else
    {
        char buffer[MAX_SIZE];
        int messageLength;

        while ((messageLength = sock->recv (buffer, MAX_SIZE-1)) > 0) {

            buffer[messageLength] = '\0';
            if (buffer[0] == 'c' && buffer[1] == 'd') {
                std::string path;
                for (int i = 3; i < messageLength; ++i) {
                    path += buffer[i];
                }
                std::string outMessage;
                if (chdir(path.c_str()) != 0) {
                    outMessage = "Invalid path";
                } else {
                    outMessage = "Successful change directory.";
                }
                sock->send(outMessage.c_str(), outMessage.length());
            }

            else if (buffer[0] == 'e' && buffer[1] == 'c' && buffer[2] == 'h' && buffer[3] == 'o') {
                std::string path;
                //ignoring $ char
                for (int i = 6; i < messageLength; ++i) {
                    path += buffer[i];
                }
                char *env;
                std::string outMessage;
                env =  getenv(path.c_str());
                if (env != nullptr) {
                    sock->send(env, strlen(env));
                } else {
                    outMessage = "Invalid getenv";
                    sock->send(outMessage.c_str(), outMessage.length());
                }
            }

            else if (buffer[0] == 'b' && buffer[1] == 's') {
                    kill(pid2, SIGINT);
                    std:: string outMessage = "Process killed.";
                    sock->send(outMessage.c_str(), outMessage.length());
            }
            else {

                char* shell_argv[MAX_SIZE + 1];
                int shell_argc;
                char* pos = buffer;
                for (shell_argc = 0; shell_argc <= MAX_SIZE; shell_argc++) {
                    // skip the spaces
                    while (*pos == ' ' || *pos == '\t')
                        pos++;
                    // if finished scanning
                    if (*pos == '\n' || *pos == '\0')
                        break;

                    // store the address of the argument
                    shell_argv[shell_argc] = pos;

                    // skip the characters of the argument
                    while (*pos != ' ' && *pos != '\t' && *pos != '\n' && *pos != '\0')
                        pos++;

                    // write zero at end to zero terminate
                    if (*pos != '\0')
                        *(pos++) = '\0';
                }
                shell_argv[shell_argc] = nullptr;

                execute(shell_argv, sock);
                if (buffer[0] == 's' && buffer[1] == 'l' && buffer[2] == 'e' && buffer[3] == 'e' && buffer[4] == 'p')
                {
                    std::string str = "Trying Sleep Command";
                    sock->send(str.c_str(), str.length());
                }
            }
        }

    }
    delete sock;
}

void execute(char* const shell_argv[], TCPSocket *sock)
{
    signal(SIGCHLD, sigChld);
    pid2 = fork();
    if (pid2 == 0) {
        close(STDOUT_FILENO);
        int ret = dup2(sock->getsockDesc(), STDOUT_FILENO);
        if (ret < 0)
        {
            printf("dup2 failed");
            exit(1);
        }
        close(sock->getsockDesc());
        if (execvp(shell_argv[0], shell_argv) < 0) {
            printf("\nCould not execute");
            exit(1);
        }
    }
    else if (pid2 < 0){
        printf("*** Fork failed\n");
        exit(1);
    }
}