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

//global variable that is will be assigned the value of thegrandchild process so that it can be killed
// outside of execute
pid_t pid2;

//maximum size of command receivable
int MAX_SIZE = 100;

// TCP client handling function
void handleConnection (TCPSocket *sock);

//calls fork and exec to execute a command, similar to Assignment 2 code
void execute(char* const shell_argv[], TCPSocket *sock);

//sigChld handler that prints out to the server information about the process and if it executed fully or was killed
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
            //indicates if the process was killed by the SIGINT signal (2)
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

// TCP client handling function
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
        //fixed size of buffer to receive command
        char buffer[MAX_SIZE];
        int messageLength;
        while ((messageLength = sock->recv (buffer, MAX_SIZE-1)) > 0) {

            //null-terminate the buffer
            buffer[messageLength] = '\0';

            //checks to see if the SIGINT signal from client arrived (custom created "end" command)
            if (buffer[0] == 'e' && buffer[1] == 'n' && buffer[2] == 'd') {
                //send SIGINT to the pid of the granchild process
                kill(pid2, SIGINT);
                std::string outMessage = "Process killed.\n";
                sock->send(outMessage.c_str(), outMessage.length());
            }

            //built-in cd command
            else if (buffer[0] == 'c' && buffer[1] == 'd') {
                std::string outMessage;
                if (chdir(&buffer[3]) != 0) {
                    outMessage = "Invalid path\n";
                } else {
                    outMessage = "Successful change directory.\n";
                }
                sock->send(outMessage.c_str(), outMessage.length());
            }

            //built-in echo command
            else if (buffer[0] == 'e' && buffer[1] == 'c' && buffer[2] == 'h' && buffer[3] == 'o') {
                std::string outMessage;
                //ignoring $ char of variable name in buffer[5]
                char *env;
                env =  getenv(&buffer[6]);
                if (env != nullptr && buffer[5] == '$') {
                    sock->send(env, strlen(env));
                } else {
                    outMessage = "Invalid getenv\n";
                    sock->send(outMessage.c_str(), outMessage.length());
                }
            }

            //if not SIGINT or built-in, just a regular command, so need to do fork() and exec()
            else {
                //Code straight from Assignment 2 Main
                //place arguments into shell_argv
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

                //execute command
                execute(shell_argv, sock);

                //necessary when doing the sleep command because the client is expecting some output
                if (buffer[0] == 's' && buffer[1] == 'l' && buffer[2] == 'e' && buffer[3] == 'e' && buffer[4] == 'p')
                {
                    std::string str = "Trying Sleep Command";
                    sock->send(str.c_str(), str.length());
                }
            }
        }
        delete sock;
    }
}

//calls fork and exec to execute a command, similar to Assignment 2 code
void execute(char* const shell_argv[], TCPSocket *sock)
{
    //register the SIGCHLD handler
    signal(SIGCHLD, sigChld);
    pid2 = fork();
    if (pid2 < 0){ //Failed Fork
        printf("*** Fork failed\n");
        exit(1);
    }
    else if (pid2 == 0) { //child
        //dup the socket description to the stdout so that the exec output goes to the client
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

}