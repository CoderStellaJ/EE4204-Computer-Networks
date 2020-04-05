#include "headsock.h"

// transmitting and receiving function
void str_ser(int sockfd);              

int main(int argc, char *argv[])
{
	int sockfd, socket_connection;
	struct sockaddr_in my_addr;

    //create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0)
	if (sockfd == -1) {			
		printf("error in socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);

    //bind socket
    socket_connection = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
	if (socket_connection == -1) {           
		printf("error in binding");
		exit(1);
	}
	printf("start receiving\n");

    // receive packets from socket
	while(1) {
        // send and receive
		str_ser(sockfd);                  
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
    char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end = 0, n = 0, len;
	long lseek=0;
    struct sockaddr_in addr;

    len = sizeof (struct sockaddr_in);

    while(!end) {
        //receive the packet
        if ((n= recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&addr, &len))==-1)  {
			printf("error when receiving\n");
			exit(1);
		}
        //if it is the end of the file
        if (recvs[n-1] == '\0')	{
			end = 1;
			n --;
		}
        memcpy((buf+lseek), recvs, n);
		lseek += n;

        // send acknowledgement
        ack.num = 1;
	    ack.len = 0;
        if ((n = sendto(sockfd, &ack, 2, 0, addr, len))==-1)
        {
            printf("send error!");								
            exit(1);
        }
    }

    //write data into file
    if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);

}
