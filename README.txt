A Chat Application for Remote Message Exchange

This program acts as both a client and server for a simple chat messaging program. On runtime, the program will set up a server in a thread and wait for a client to connect. The client can do the following:
     1. help - This option displays the different choices for the interface.
     2. myip - The myip option displays the client's ip address.
     3. myport - The myport option displays the client's port number.
     4. connect <destination> <port no> - The connect option establishes a TCP connection to
	the server at the specified destination. The <destination> is filled out with the
	ip address and <port no> is filled out with the port number for the target server.
     5. list - The list option displays a list of connection id numbers, ip addresses,
	and port numbers.
     6. terminate <connection id> - The terminate option closes the connection to target
	server. The <connection id> is chosen from the list option.
     7. send <connection id> <message> - The send option sends a message to the targeted
	server. The <connection id> is chosen from the list option, and <message> is the
	string that will be send to the chosen server.
     8. exit - The exit option exits the program.

How To Use:
Compile the program using a command console or terminal and type: gcc -pthread -o chat chat.c
There is no installation required for the chat application. The program is executed with a port number in the main argument. For example, ./chat 4444.

In order to be able to send messages, the user needs to use the connect() command (detailed above in no. 4) first.
Note: Both devices intending to send/reply messages to one another is required to manually connect to the other device. 

Use the list() command (detailed above in no. 5) to get the id of a connection. 
To send a message, use the send() command (detailed above in no. 7).

Use exit() command when done. 



Member contributions:

Worked on together:
-The basic structure of server and client was made together in seperate files and compiled together into one file.

Chris:
-menu() - A basic menu was created with if and else if statements.
-myip() - A function that returns a character array representing the ip address.
-myport() - A function that returns a port number in the form of an integer.
-exit() - A simple command to exit the program.
-README.txt - Created for the purpose of instruction and listing contributions.

Marc: 
-connect() - A function that connects the client(user) to a server with the ip address and port number manually inputed by the user.
-list() - A function that generates a list of connection ids, ip addresses, and port numbers.
-terminate() - A simple function that terminates the connection between server and client.
-send() - A function that sends a character array message to the chosen connection id.
-server() - A thread containing the server function. This is meant to run alongside the client.


