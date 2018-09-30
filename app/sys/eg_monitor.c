/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : eg_monitor.c
 @brief  : thread for monitor usb status
 @author : gexueyuan
 @history:
           2018-9-30    gexueyuan    Created file
           ...
******************************************************************************/

#include <stdlib.h>

#include <errno.h> 
#include <fcntl.h>
#include <time.h> 
#include <errno.h>  
#include <sys/types.h>
#include <asm/types.h>

#include <sys/select.h>
#include <sys/socket.h>  
#include <linux/netlink.h>  
#include "cv_cms_def.h"

#define UEVENT_BUFFER_SIZE 2048  

#define ADD_STR "add@"
#define REMOVE_STR "remove@"

#define USB_DISCONNET "USB disconnect"
#define KILL_STR "kill -9 "

#define CHECK_112_USB "ls /sys/devices/platform/ehci-platform/driver/ehci-platform/usb1/1-1/ -l |grep \"1-1.2\"|wc -l"
#define CHECK_113_USB "ls /sys/devices/platform/ehci-platform/driver/ehci-platform/usb1/1-1/ -l |grep \"1-1.3\"|wc -l"

char pc02322_init[] = "killall pc02322";
  
int check_usb(char* cmd)
{
	
	FILE *fstream=NULL;       
    char buff[1024];     
    memset(buff,0,sizeof(buff));     
    if(NULL==(fstream=popen(cmd,"r")))       
    {      
        fprintf(stderr,"execute command failed: %s",strerror(errno));       
        return -1;       
    }      
    if(NULL!=fgets(buff, sizeof(buff), fstream))      
    {      
        printf("322 num + %s \n",buff);
		//strncpy(pid,buff,16);
		if(strncmp(buff,"1",1) == 0){
			pclose(fstream);
			return 1;
		}	
		else if(strncmp(buff,"0",1) == 0){
			
			
			pclose(fstream);
			return 0;
		}
		else{
			
			//printf("else  is %d \n",atoi(buff));
			pclose(fstream);
			return atoi(buff);
		}
    }      
    else     
    {     
        pclose(fstream);     
        return -1;     
    }     
    pclose(fstream);     
    return 0;   		
}   
void *eg_usb_monitor_entry(void *parameter)
{  
	int sockfd;
	struct sockaddr_nl sa;
	int len;
	char buf[4096];
	struct iovec iov;
	struct msghdr msg;
	int i;
	char* p_f;
	char sum_322 = 0;

	memset(&sa,0,sizeof(sa));
	sa.nl_family=AF_NETLINK;
	sa.nl_groups=NETLINK_KOBJECT_UEVENT;
	sa.nl_pid = 0;//getpid(); both is ok
	memset(&msg,0,sizeof(msg));
	iov.iov_base=(void *)buf;
	iov.iov_len=sizeof(buf);
	msg.msg_name=(void *)&sa;
	msg.msg_namelen=sizeof(sa);
	msg.msg_iov=&iov;
	msg.msg_iovlen=1;
	
	sockfd=socket(AF_NETLINK,SOCK_RAW,NETLINK_KOBJECT_UEVENT);
	if(sockfd==-1)
		printf("socket creating failed:%s\n",strerror(errno));
	if(bind(sockfd,(struct sockaddr *)&sa,sizeof(sa))==-1)
		printf("bind error:%s\n",strerror(errno));


	printf("begin to monitor usb\n");  
    while (1) {  
		
		len=recvmsg(sockfd,&msg,0);
		if(len<0){
			printf("receive error\n");
			continue;
		}
		else if(len<32||len>sizeof(buf)){
			printf("invalid message\n");
			continue;
		}
		for(i=0;i<len;i++)
			if(*(buf+i)=='\0')
				buf[i]='\n';
		printf("received %d bytes\n%s\n",len,buf);
		p_f = strstr(buf,"add");
		if(p_f){
				sum_322 = 0;
				if(check_usb(CHECK_112_USB)){

				
					sum_322++;
					printf("1-1.2 exist\n");

				}

				if(check_usb(CHECK_113_USB)){
					
					
					sum_322++;
					printf("1-1.3 exist\n");

				}

			if(controll_eg.cnt_322 < sum_322){

				printf("new 322 come in,init again\n");
				system(pc02322_init);

			}
			else{

				
				printf("no new 322 insert!\n");
			}

		}

			
        }  
    close(sockfd);  
    return 0;  
}

void eg_monitor_init(void)

{
	osal_task_t *tid;

	tid = osal_task_create("task monitor for usb",
						eg_usb_monitor_entry,
						NULL,PC02_MSG_THREAD_STACK_SIZE, PC02_USB_THREAD_PRIORITY);

	osal_assert(tid != NULL);
}
