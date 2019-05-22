#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 3

void server(char * PORT);
char * myip();
int myport(char * PORT);
in_port_t get_in_port(struct sockaddr *sa); 
char* itoa(int num, char* str, int base);
void swap(char *xp, char *yp);

int main(int argc, char * argv[])
{
    int clientfd, ret, fdmax;
    socklen_t addr_size;
    fd_set fdList;

    FD_ZERO(&fdList);     

    char * PORT = argv[1];

    pthread_t serverThread;
    pthread_create(&serverThread, NULL, (void*)server, (void*)PORT);

    sleep(1);
    printf("Type \"help\" for options.\n");
    while(1) 
    {       
        while (1)
        {
            char inputStr[10];
            char ipAddress[16];
	        char portDes[6];
            int connectionId = 1;

            printf("Command: ");
            scanf("%s", inputStr);
		    if(strcmp(inputStr, "help") == 0) {			
		        printf("========================\n");
	            printf("myip -> display IP address\n");
	            printf("myport -> display port number\n");
	            printf("connect -> connect to another peer\n");
	            printf("list -> list all connected peers\n");
	            printf("terminate -> terminate the connection\n");
	            printf("send -> send messages to peers\n");
	            printf("exit -> exit the program\n");
	            printf("========================\n");
                fflush(stdin);
                break;
            }	
		    else if(strcmp(inputStr, "myip") == 0) 
            {	
			    printf("IP Address: %s\n", myip());
                fflush(stdin);
                break;
		    }
    		else if(strcmp(inputStr, "myport") == 0)
            {		
	    		printf("The program runs on port number %d\n", myport(PORT));
		    	fflush(stdin);	
                break;
		    }
		    else if(strcmp(inputStr, "connect") == 0) 
            {
    	        scanf("%s %s", ipAddress, portDes);
                struct addrinfo client;
                struct addrinfo *clientInfoPtr, *ptr;

                memset(&client, '\0', sizeof(client));
                client.ai_family = AF_UNSPEC;
                client.ai_socktype = SOCK_STREAM;
                client.ai_flags = AI_PASSIVE;

                if (getaddrinfo(ipAddress, portDes, &client, &clientInfoPtr) != 0)
                {
                    perror("getaddrinfo");
                    break;
                }

                int i = 1, j = 0;
                for(ptr = clientInfoPtr; ptr != NULL; ptr = ptr->ai_next)
                {
                    clientfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                    if(clientfd < 0)
                    {   
                         continue;
                    }
                    if (connect(clientfd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                    {
                        printf("System: Connected to %s at %s\n", ipAddress, portDes);
                        FD_SET(clientfd, &fdList);
                        if (clientfd > fdmax)
                        {
                            fdmax = clientfd;
                        }
                        continue;
                    }
                    else printf("Error: Could not make connection.\n");
                }               
			    fflush(stdin);
                break;
		    }
		    else if(strcmp(inputStr, "list") == 0)
            {   
                int port;
                char ip[14];
                struct sockaddr_storage addrStorage;
                socklen_t addrlen;
                addrlen = sizeof(addrStorage);

                printf("\nID \tIP Address \t \tPort\n");
                for (int i = 0, j = 1; i <= fdmax; i++)    
                {   
                    if (FD_ISSET(i, &fdList)) // add a j counter for id, i is for fd 
                    {
                        getpeername(i, (struct sockaddr*)&addrStorage, &addrlen);
                        if (addrStorage.ss_family == AF_INET) 
                        {   
                            struct sockaddr_in *s = (struct sockaddr_in *)&addrStorage;
                            port = ntohs(s->sin_port);
                            inet_ntop(AF_INET, &s->sin_addr, ip, INET_ADDRSTRLEN);
                        } 
                        else 
                        { 
                            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addrStorage;
                            port = ntohs(s->sin6_port);
                            inet_ntop(AF_INET6, &s->sin6_addr, ip, INET6_ADDRSTRLEN);
                        }
                        printf("%d \t%s \t \t%d\n", j, ip, port);
                        j++;
                    }
                }
                printf("\n");
                fflush(stdin);
                break;
		    }
		    else if(strcmp(inputStr, "terminate") == 0) 
            {
                int flagTerminate = 0;
                scanf("%d", &connectionId);
                for (int i = 0, j = 1; i <= fdmax; i++) 
                {
                    if (FD_ISSET(i, &fdList) > 0)
                    {
                        if (j == connectionId)
                        {
                            FD_CLR(i, &fdList);
                            flagTerminate = 1;
                        }
                        j++;
                    }
                }
                if (flagTerminate == 0)
                    printf("Error: Invalid connection id.\n");
                fflush(stdin);
                break;
		    }
		    else if(strcmp(inputStr, "send") == 0) 
            {
                char msg[100];
                scanf("%d %100[^\n]", &connectionId, msg);

                for (int i = 0, j = 1; i <= fdmax; i++)
                {

                    if (FD_ISSET(i, &fdList))
                    {
                        if (j == connectionId)
                        {
                            char promptIp[30];
                            strcpy(promptIp, "\n\t\tMessage received from ");
                            strcat(promptIp, myip());
                            if (send(i, promptIp, strlen(promptIp), 0) == -1)
                            {
                                perror("send. Error: IP Prompt: ");
                            }

                            char promptPort[30];
                            char senderPort[5];
                            itoa(myport(PORT), senderPort, 10);
                            strcpy(promptPort, "\n\t\tSender's Port: ");
                            strcat(promptPort, senderPort);
                            if (send(i, promptPort, strlen(promptPort), 0) == -1)
                            {
                                perror("send. Error: Port Prompt: ");
                            }

                            char promptMsg[120];
                            strcpy(promptMsg, "\n\t\tMessage: ");
                            strcat(promptMsg, msg);                    
                            if (send(i, promptMsg, strlen(promptMsg), 0) == -1)
                            {
                                perror("send. Error: Message Prompt: ");
                            }
                            else
                            {
                                printf("Message sent to %d", connectionId);
                            }
                        }
                        j++;
                    }   
                }     
                printf("\n");
                fflush(stdin);
                break;
		    }
		    else if(strcmp(inputStr, "exit") == 0)
            {
			    printf("Exiting Program.\n");
                shutdown(clientfd, SHUT_RDWR);
			    exit(0);
                break;
		    }
		    else 
            {
			    printf("Invalid input.\n");
			    fflush(stdin);
                break;
		    }

        } // End of menu loop
    } // End of main loop

    pthread_join(serverThread, NULL);
    pthread_exit(NULL);
    shutdown(clientfd, SHUT_RDWR);
  
} // End of main


char * myip() 
{
    FILE * f;
    char line[100], *p, *c;

    f = fopen("/proc/net/route", "r");
    while (fgets(line, 100, f))
    {
        p = strtok(line, "\t");
        c = strtok(NULL, "\t");
        if (p != NULL && c != NULL)
        {
            if (strcmp (c, "00000000") == 0)
            {
                    break;
            }
        }
    }

    // AF_INET or AF_INET6
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
	int family , s;
	static char ip[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	//Walk through linked list, maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
		{
			continue;
		}

		family = ifa->ifa_addr->sa_family;

		if(strcmp( ifa->ifa_name , p) == 0)
		{
			if (family == fm) 
			{
				s = getnameinfo( ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6) , ip , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
				
				if (s != 0) 
				{
					printf("getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				
			}
			
		}
	}

	freeifaddrs(ifaddr);

    return ip;
} 

int myport(char * PORT) 
{	
    struct addrinfo temp;
    struct addrinfo * ai;
    int rv;

    memset(&temp, 0, sizeof temp);
	temp.ai_family = AF_UNSPEC;
	temp.ai_socktype = SOCK_STREAM;
	temp.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &temp, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	return ntohs(get_in_port(ai->ai_addr));
}

in_port_t get_in_port(struct sockaddr *sa) 
{
	if (sa->sa_family == AF_INET) 
	{
		return (((struct sockaddr_in*)sa)->sin_port);
	}
	
	return (((struct sockaddr_in6*)sa)->sin6_port);
}

char* itoa(int num, char* str, int base) 
{ 
    int i = 0; 
    int isNegative = -1; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) 
    { 
        isNegative = 1; 
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    str[i] = '\0'; // Append string terminator 
  
    int start = 0; 
    int end = i -1; 
    while (start < end) 
    { 
        swap((char*)(str+start), (char*)(str+end)); 
        start++; 
        end--; 
    } 

    return str; 
} 

void swap(char *xp, char *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 



void server (char * PORT)
{
    int serverfd, newfd, ret, fdmax, yes = 1;
    struct addrinfo hints;
    struct addrinfo *ai, *p;
    socklen_t addr_size;
    char msg[1024];

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((ret = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) 
    {
        printf("Error: Failed to Connect. Exiting ... \n");
        exit(1);
	}
    printf("System: Server socket created.\n");
    int i = 0;
   	for(p = ai; p != NULL; p = p->ai_next) 
    {
        serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (serverfd < 0) 
        { 
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        int b = bind(serverfd, p->ai_addr, p->ai_addrlen);
        if (b < 0) 
        {
            perror("bind");
            close(serverfd);
            continue;
        }
        else
        {
            printf("Bound to port %s\n", PORT);
            break;
        }   
        i++;
	}

    if (listen(serverfd, BACKLOG) == 0)
    {
        printf("Listening . . .\n");
    }    
    else
    {
        printf("Error: Failed to find any bound connection.\n");
    }


    while (1)
    {            
        int port;
        char ip[14];
        struct sockaddr_storage addrStorage;
        socklen_t addrlen;
        addrlen = sizeof(addrStorage);
        
        memset(&addrStorage, 0, addrlen);

        newfd = accept(serverfd, (struct sockaddr*)&addrStorage, &addrlen);
        if (newfd < 0)
        {
            exit(1);
        }  

        getpeername(newfd, (struct sockaddr*)&addrStorage, &addrlen);
        if (addrStorage.ss_family == AF_INET) 
        {   
            struct sockaddr_in *s = (struct sockaddr_in *)&addrStorage;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ip, INET6_ADDRSTRLEN);
        } 
        else 
        {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addrStorage;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ip, INET_ADDRSTRLEN);
        }

        printf("\nSystem: Connected to %s: %d\n", ip, port);      
        fflush(stdout);    
        while (1) 
        {
            int rv = recv(newfd, msg, 101, 0);
            if (rv <= 0)   // awaits message to arrive
            {
                printf("\nSystem: Disconnected from %s: %d", ip, port);
                close(newfd);
                break;
            }
            else
            {
                printf("%s", msg);
                fflush(stdout);
            }
            bzero(msg, sizeof(msg));
        }
    } // End of main while
    shutdown(newfd, SHUT_RDWR);
    shutdown(serverfd, SHUT_RDWR);
}
