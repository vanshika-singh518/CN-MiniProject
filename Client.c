#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>

#include<errno.h>
#include<pthread.h>
#include<signal.h>


#define SA struct sockaddr

volatile sig_atomic_t flag = 0;
char string[5000],name[32],rno[32],post[32];
int sockfd=0,action=0;
long int array_size = 524280;		// 524280 bits == 64 Kilobytes
void str_overwite_stdout()
{
	printf("\r%s","> ");
	fflush(stdout);
}

void str_trim(char *arr, int len)
{
	for(int i=0;i<len;i++)
	{
		if(arr[i] == '\n')
		{
			arr[i] = '\0';
			break;
		}
	}
}

void ctrl_c(){
	flag = 1;
}

void send_f(){
	char buffer[5000] = {};
	char msg[5000+35] = {};					// name : msg
	while(1){
		str_overwite_stdout();
		fgets(buffer,5000,stdin);
		str_trim(buffer,5000);
		if(!action){
			if(strcmp(buffer,"exit") == 0){
				break;
			}else{
				if(strcmp(buffer,"set-admin")==0 || strcmp(buffer,"unset-admin")==0){
					send(sockfd,buffer,strlen(buffer),0);
				}else{
					sprintf(msg,"%s : %s\n",name,buffer);
					send(sockfd,msg,strlen(msg),0);
				}
			}
		}else if(action==1){
			
			FILE *fp;
			fp = fopen(buffer,"r");
			if(fp==NULL){
				printf("[+] failed to open given file ! Please enter the File Name again :\n");
				continue;
			}
			
			send(sockfd,buffer,strlen(buffer),0);
			
			char nc,filearray[array_size];
			bzero(filearray,sizeof(filearray));
			int pointer;
			pointer=0;
			printf("\n\nCode:\n");
			while(fscanf(fp,"%c",&nc)!=EOF){
				filearray[pointer]=nc;
				pointer++;
			}
			printf("%s\n",filearray);
			send(sockfd,filearray,sizeof(filearray),0);			
			
			action=0;
		}else if(action==3){
			send(sockfd,buffer,strlen(buffer),0);
		}
		else if(action==4){
			
			FILE *fp;
			fp = fopen(buffer,"r");
			if(fp==NULL){
				printf("[+] failed to open given file ! Please enter the File Name again :\n");
				continue;
			}
			send(sockfd,buffer,sizeof(buffer),0);
			
			printf("  [+] Sending %s file...[Press Enter To Continue...]\n",buffer);
			
			action =0;
		}
		bzero(buffer,5000);
		bzero(msg,5000+35);
	}
	ctrl_c(2);
}

void recv_f(){
	char	msg[5000];
	bzero(msg,5000);
	while(1){
		int r = recv(sockfd,msg,5000,0);
		if(r>0){
			
			if(strcmp(msg,"$[+]$incoming$file$from$admi$")!=0)
				printf("%s\n",msg);
			if(strcmp(msg,"[+] Enter file name: ")==0 && action==0)
				action=1;
			if(strcmp(msg,"$[+]$incoming$file$from$admi$")==0){

				bzero(msg,5000);
				char fileName[50],location[5000],directory[1000];

				bzero(fileName,50);	
				bzero(location,5000);
				bzero(directory,1000);
				
				recv(sockfd,fileName,50,0);
				
				printf("[+] fileName:- %s\n",fileName);
				sprintf(location,"Files/%s",fileName);
				
				char filearray[array_size];
				
				bzero(filearray,sizeof(filearray));
				recv(sockfd,filearray,sizeof(filearray),0);
				
				system("mkdir Files");
				FILE *fp;
				
				fp = fopen(location,"w");
				printf("[+] new program file created...(location: %s)\n",location);

				fprintf(fp,filearray);
				fclose(fp);
				
				printf("code:\n%s\n",filearray);
				
				}
			if(strcmp(msg,"[+] Enter Student name or roll number: ")==0)
				action=3;
			if(strcmp(msg,"[+] student is present.")==0)
				action=4;
			if(strcmp(msg,"[+] student is absent.")==0)
				action=0;
			str_overwite_stdout();
		} else if(r==0){
			break;
		}
		bzero(msg,5000);
	}
}


int main(int argc,char **argv)
{
	if(argc == 3 || argc == 2 || argc == 1){;}
	else
	{
		printf("\nInvalid number of arguments passed...\n(hint : syntax is like './a.out <port_no> <ip_address>')\n");
		return EXIT_FAILURE;
	}

	signal(SIGINT, ctrl_c);
whoAreYou:
	printf("Who are you? (1.teacher/2.student): ");
	fgets(post,32,stdin);
	str_trim(post,strlen(post));
	
	if(strcmp(post,"student")==0 || strcmp(post,"Student")==0 || strcmp(post,"2")==0){
	printf("Enter your name: ");
	fgets(name,32,stdin);
	str_trim(name,strlen(name));
  printf("Enter your Roll Number: ");
	fgets(rno,32,stdin);
	str_trim(rno,strlen(rno));
  }else if(strcmp(post,"teacher")==0 || strcmp(post,"Teacher")==0 || strcmp(post,"1")==0){
  	printf("Enter your name: ");
		fgets(name,32,stdin);
		str_trim(name,strlen(name));
  }else{
  	printf("please only select between given two options...\n");
  	goto whoAreYou;
  }
  
	if(strlen(name) > 32 || strlen(name) < 2)
	{
		printf("\nEnter the name correctly...\n");
		return EXIT_FAILURE;
	}
	
	if(strcmp(post,"student")==0 || strcmp(post,"Student")==0 || strcmp(post,"2")==0)
		if(strlen(rno) > 32 || strlen(rno) < 1)
		{
			printf("\nEnter the roll number correctly...\n");
			return EXIT_FAILURE;
		}


	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		printf("\nERROR: socket()\n");
		return EXIT_FAILURE;
	}

	servaddr.sin_family = AF_INET;
	if(argc == 3 || argc == 2)
  	servaddr.sin_port = htons(atoi(argv[1]));
  else
    servaddr.sin_port = htons(8080);
	if(argc == 3)
		servaddr.sin_addr.s_addr = inet_addr(argv[2]);
	else
	{
//		printf("\nserver ip not provided , so using local ip address...\n");
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	if (connect(sockfd,(SA*)&servaddr,sizeof(servaddr))<0)
	{
		printf("\nERROR: Connect()\nServer might be busy...\n");
		return EXIT_FAILURE;
	}
	
	if(strcmp(post,"teacher")==0 || strcmp(post,"Teacher")==0 || strcmp(post,"1")==0)
		strcpy(rno,"teacher");
		
			send(sockfd,name,sizeof(name),0);
			send(sockfd,rno,sizeof(rno),0);
	system("cls || clear");
	
	printf("\n~~~ Welcome To TSI ~~~\n");
	
	pthread_t send_t;
	
	if(pthread_create(&send_t , NULL , (void*)send_f, NULL)!=0)
	{
		printf("\nError: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_t;
	if(pthread_create(&recv_t , NULL , (void*)recv_f, NULL)!=0)
	{
		printf("\nError: pthread\n");
		return EXIT_FAILURE;
	}

	while(1)
	{
		if(flag)
		{
			printf("\nexited\n");
			break;
		}
	}

	close(sockfd);
	return EXIT_SUCCESS;
}
