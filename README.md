Assignment 8
============

**Due: May 1, 2019 11:59 PM US Central Time**. Push to GitHub to submit the assignment.

In this assignment you are creating a remote shell version of the simple shell assignment that you submitted in assignment 2. You will create two programs: `rsh-server` and `rsh-client`. The `rsh-server` program will listen for connections on a TCP server socket from instances of the `rsh-client` program. The `rsh-client` will provide a terminal-type program, similar to assignment 2, that sends commands to the `rsh-server` for remote execution and displays the results locally.

## Tasks

The `rsh-server` program should implement the following functionality:

* Start the `rsh-server` on the master machine.
* Listen and accept connections from `rsh-client` programs on a TCP server socket.
* For each `rsh-client`, the `rsh-server` will create a new child process and use the socket descriptor to communicate with the client.  In this child process, valid commands received from the `rsh-client` should be executed using `fork()` and `exec()` calls. 
* Implement the builtin commands `cd` and `echo $variable_name`. For example, when the client sends a command such as `cd ..`, you should execute this command in the parent on the `rsh-server` (this is why the `rsh-server` creates a separate child process for each connection). Consult the following man pages: [chdir](http://man7.org/linux/man-pages/man2/chdir.2.html) and [getenv](http://man7.org/linux/man-pages/man3/getenv.3.html).

The `rsh-client` program should provide a terminal program, similar to assignment 2, that implements the following functionality:

* Connect to an `rsh-server` with the command: connect ipaddress:port \<username\> \<password\>
* Upon a successful connection to an `rsh-server`, the `rsh-client` should enter a terminal loop, similar to assignment 2, that sends entered commands to the `rsh-server` and displays the results on the client terminal. This will require the use of the `dup2` system call.
* Disconnect from an `rsh-server` with the command: disconnect ipaddress:port
* Maintain the current connection status (i.e., is the client connected to an `rsh-server` or not). Note that a client can only connect to one server at a time.
* Handle the SIGINT signal: if `ctrl-c` is pressed on the client-side, it should result in a `SIGINT` signal being sent to the corresponding worker process on the `rsh-server`.

Write a report that:

* Describes your design in detail.
* Includes screenshots and explanations to describe how your code works.

## Evaluation

Your assignment will be graded according to the following criteria:

- **35 points** for the correct implementation of the `rsh-server`
- **35 points** for the correct implementation of the `rsh-client`
- **30 points** for the report
