#include "headsock.h"

void str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);  
//calculate the time interval between out and in              
void tv_sub(struct timeval *out, struct timeval *in);	    

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in ser_addr;
	char **pptr;
	struct hostent *sh;
	struct in_addr **addrs;
    long len;
    FILE *fp;
    float ti, rt;

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}
    //get host's information
	if ((sh=gethostbyname(argv[1]))==NULL) {             
		printf("error when gethostbyname");
		exit(0);
	}

    //create socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++) {
        //printf socket information
        printf("the aliases name is: %s\n", *pptr);			
    }
		
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

    if((fp = fopen ("myfile.txt","r+t")) == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}
                    
    // receive and send
    ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);   
    //caculate the average transmission rate
	rt = (len/(float)ti);                                         
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

	close(sockfd);
	fclose(fp);
	exit(0);

}

void str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len)
{
    char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, m, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

    fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

    // allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

    // copy the file into the buffer.
	fread (buf,1,lsize,fp);

    /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time

    ////////////////////////
    while(ci<= lsize)
	{
		if ((lsize+1-ci) <= DATALEN) {
            slen = lsize+1-ci;
        } else {
            slen = DATALEN;
        }
			
		memcpy(sends, (buf+ci), slen);
        //send the packet to server
        n = sendto(sockfd, &sends, slen, 0, addr, addrlen);                         
		if(n == -1) {
			printf("send error!");								
			exit(1);
		}
		ci += slen;

        //receive the ack
        if ((m= recvfrom(sockfd, &ack, 2, 0, (struct sockaddr *)&addr, (socklen_t *)&addrlen))==-1) {
            printf("error when receiving\n");
            exit(1);
        }

        if (ack.num != 1|| ack.len != 0) {
            printf("error in transmission\n");
        }
	}

	//get current time
	gettimeofday(&recvt, NULL);
	*len= ci;          
    //get the whole trans time                                               
	tv_sub(&recvt, &sendt);                                                                 
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}


void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
