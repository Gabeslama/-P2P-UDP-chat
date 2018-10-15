//
//  main.c
//  clientserver
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TOPORT 14950
#define MYPORT 1043

//Counter & mutex & servaddr
pthread_mutex_t lock;
int udp_counter;
char send_data;
struct sockaddr_in servaddr;

//UDP client thread.
int udp_client(int argn, char* argv[]){
    int sockfd,port;
    
    char send_data[1024];
    //maybe change from int
    
    if (argn > 3) {
        fprintf(stderr, "Usage: %s [server.ip.address [port]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* Is the server IP address provided? If not, use the localhost */
    char * server = (argn > 1) ? argv[1] : "127.0.0.1";
    

    /* Create a new UDP socket */
    if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0))) {
        perror("socket creation");
        return EXIT_FAILURE;
    }
    
    
    
    if (argn == 3) {
        char *endp;
        port = strtol(argv[2], &endp, 10);
        if (endp == argv[2] || *endp) {
            fputs("Port number must be integer\n", stderr);
            return EXIT_FAILURE;
        }
    } else
        port = 8088;
    
    /* Initialize the server address */
    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8088);
    
    if (INADDR_NONE == (servaddr.sin_addr.s_addr = inet_addr(server))) {
        perror(server);
        return EXIT_FAILURE;
    }
    
    
   

    
    while (1)
    {
        printf("(%d > ) ",udp_counter );
        gets(send_data);
        
        sendto(sockfd, send_data, sizeof(send_data), 0,
               (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
        
        pthread_mutex_lock(&lock);
        
        udp_counter--; //counter deincrement when sending message
        pthread_mutex_unlock(&lock);
        
        
        //Stoping the client.
        if ((strcmp(send_data , "%stop") == 0))
            break;
    }
   
    close(sockfd);
    return EXIT_SUCCESS;
    
}//udp client ends

struct client_param {   //making the function a void by passing the parameters to void
    int argn;
    char* argv;
    int send_data;
};

void *client_thr(void *arg)
{
    struct client_param *params = arg;
    params->send_data = udp_client(params->argn, &params->argv);
    return NULL;
}



//udp server starts

int udp_server(int argn, char* argv[])
{
    int sockfd, port;
    char buffer[1024];
    struct sockaddr_in addr;
    
    /* Is the port number provided? If not, use 8088 */
    if (argn == 2) {
        char *endp;
        port = strtol(argv[2], &endp, 10);
        if (endp == argv[2] || *endp) {
            fputs("Port number must be integer\n", stderr);
            return EXIT_FAILURE;
        }
    }else
        port = 8088;
    
    
    
    /* Create a new UDP socket */
    if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0))) {
        perror("socket creation");
        return EXIT_FAILURE;
    }

    /* Initialize the address and bind the socket */
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8089);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
    //binding the addresss
    if ( bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ){
        perror("bind");
        addr.sin_port = htons(8088);
        servaddr.sin_port = htons(8089);
        bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
}

    //Starting the listening loop
    while (1)
    {
        int bytes;
        socklen_t addr_len=sizeof(addr);
        memset(buffer,0, 1024);
        bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addr_len);
        printf("Server: msg recieved %s\n", buffer);

        pthread_mutex_lock(&lock);
        udp_counter++;
        pthread_mutex_unlock(&lock);

        //stoping the server
        if (strncmp(buffer, "%stop", 5) == 0)
            break;
    }
    
    close(sockfd);
    return EXIT_SUCCESS;

}//udp server ends


struct server_param {   //making the function a void by passing the parameters to void
    int argn;
    char* argv;
    int send_data;
};

void *server_thr(void *arg)
{
    struct server_param *params = arg;
    params->send_data = udp_server(params->argn, &params->argv);
    return NULL;
}


int main(int argn, char* argv[])
{
    
    
    struct client_param params;
    struct server_param paramss;
    pthread_t send, rec;

    printf("type '%%stop' (less the quotes) to quit: ");
    
    //initalizing the mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    
    pthread_create(&send, NULL, client_thr, &params); //passed paramaters to void
    pthread_create(&rec, NULL, server_thr, &paramss);
    
    // Waiting for the threads to finish
    pthread_join(send, NULL);
    pthread_join(rec, NULL);
    exit(0);
    
    
}

