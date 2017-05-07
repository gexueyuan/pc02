/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : eg_tcp.c
 @brief  : tcp lclient
 @author : gexueyuan
 @history:
           2016-12-11    gexueyuan    Created file
           ...
******************************************************************************/


#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <netdb.h>  
#include "cv_cms_def.h"
#include <time.h>
#include <arpa/inet.h>


//#define  USE_STR

#define    PORT 8030  
#define    MAXDATASIZE 100000
#define    SERVERADDR  "172.16.32.152"

#define    CONNECT_STR "01010101010006AABBCCDDEEFF0300E704"
#define    CONNECT_ACK_STR  "01010101010001000300E704"

#define    HBM_REC  "010101010500000300E704"
#define    HBM_SEND "0101010105001BAABBCCDDEEFF0300E704" 


extern   void usb_transparent(unsigned char *data_u,int length);
//extern void back_poll(void);

extern void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

int  sockfd;  
char  buf[MAXDATASIZE];  


char connect_array[] = {0x01,0x01,0x01,0x01,0x01,0x00,0x06,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x03,0x00,0xE7,0x04,0x24,0x5F};


    #define  MSG_HEAD_str "01010101"
    #define  ACL_DL "4B"
    

    char MSG_HEAD[4]={0x01,0x01,0x01,0x01};
    


    #define    DATA_END     0x03
    #define    FRAME_END    0x04 

    #define    CMD_CONNECT  0x01
    #define    CMD_POLL     0x05
    #define    CMD_ACL_DL   0x4B

    

typedef struct _data_cs {

    char msg_head[4];
    uint8_t cmd;
    uint16_t length;
    uint8_t data_end;
    uint16_t crc;
    uint8_t frame_end;
    uint8_t data[0];
} data_cs_t;



char sendByte[32];
char getByte[2048];

int print_time()
{

    time_t now;    //实例化time_t结构
    struct tm  *timenow;    //实例化tm结构指针

    time(&now);//time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
    timenow = localtime(&now);//localtime函数把从time取得的时间now换算成你电脑中的时间(就是你设置的地区)
    printf("Local time is %s\n",asctime(timenow));//asctime函数把时间转换成字符，通过printf()函数输出

    return 0;




}

/* 
 * 将字符转换为数值 
 * */  
int c2i(char ch)  
{  
        // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2  
        if(isdigit(ch))  
                return ch - 48;  
  
        // 如果是字母，但不是A~F,a~f则返回  
        if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )  
                return -1;  
  
        // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10  
        // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10  
        if(isalpha(ch))  
                return isupper(ch) ? ch - 55 : ch - 87;  
  
        return -1;  
} 



/*
// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//	[IN] pbSrc - 字符串
//	[IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value: 
// remarks : 将字符串转化为16进制数
*/
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char h1,h2;
    unsigned char s1,s2;
    int i;

    for (i=0; i<nLen; i++)
    {
    h1 = pbSrc[2*i];
    h2 = pbSrc[2*i+1];

    s1 = toupper(h1) - 0x30;
    if (s1 > 9) 
    s1 -= 7;

    s2 = toupper(h2) - 0x30;
    if (s2 > 9) 
    s2 -= 7;

    pbDest[i] = s1*16 + s2;
}
}



static bool_t net_state = 1;
int get_netstate(void)
{
    return net_state;

}

int reconnect()
{
    //指定server地址    
    struct sockaddr_in server;  
    bzero(&server,sizeof(server));  
    server.sin_family= AF_INET;  
    server.sin_port = htons(PORT);


//创建套接字
  if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){  
      printf("socket()error\n");  
      return -1; 
  }  

  
  if( inet_pton(AF_INET, SERVERADDR, &server.sin_addr) <= 0){
      printf("inet_pton error for %s\n",SERVERADDR);
      //exit(0);
      return -1; 
  }

  //发起连接操作
  if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){  
      printf("connect()error\n");  
      //exit(1); 
      net_state = 0;
      return -1; 
  }

    return 1;


}



int eg_tcp_send(char *data,int len)
{


    int ret;

    //char str_cmp = "$_";

    //char data_temp[4096];

    //memcpy(data_temp,data,len);
    //memcpy(&data_temp[len],str_cmp,2);
    if(net_state == 0){
       printf("disconnect with server!\n");
       
       reconnect();
        return -2;
    }
    if( ret = send(sockfd, data, len, 0) < 0){
    //if( ret = send(sockfd, data_temp, len + 2, 0) < 0){
    
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
        
    }else{

        
        printf("send msg len is %d\n",len);
        //printf("send msg is %s,len is %d\n",data,len);
    }

    return 1;


}

int check_IDcard(void)
{
  return  eg_tcp_send(connect_array,sizeof(connect_array));
}

void eg_tcp_main_proc(int cmd,char *data,int length)
{

    switch (cmd) {
        
    case CMD_CONNECT:

        if(!data[0])

            printf("connect to server\n");

        else
            printf("connect failed");
        
        break;

    case CMD_POLL:

        //eg_tcp_send(data,length);
        printf("send data to usb,len is %d\n",length);
        //usb_transparent(data,length);
        //printf("back to usb state\n");
        //back_poll();
        break;

    case CMD_ACL_DL:

        break;
    
    default:
        break;
    }




}

int eg_tcp_client(void)  
{  
    int  sockfd, num;  
    char  buf[MAXDATASIZE];  
    struct hostent *he;  
 
  //创建套接字
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){  
        printf("socket()error\n");  
        exit(1);  
    }  

    //指定server地址    
    struct sockaddr_in server;  
    bzero(&server,sizeof(server));  
    server.sin_family= AF_INET;  
    server.sin_port = htons(PORT);  
    if( inet_pton(AF_INET, SERVERADDR, &server.sin_addr) <= 0){
        printf("inet_pton error for %s\n",SERVERADDR);
        exit(0);
    }

    //发起连接操作
    if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){  
        printf("connect()error\n");  
        exit(1);  
    } 

    //读写操作
    while(1){
        printf("ready to get data!\n");
    if((num=recv(sockfd,buf,MAXDATASIZE,0)) == -1){  
        printf("recv() error\n");
        
        close(sockfd);  
        exit(1);  
    } 
    
    buf[num]='\0';  
    printf("Server Message: %s\n",buf); 
        }
    //if()

    
    close(sockfd);  
    return 0;  
}  

void * net_thread_entry (void *parameter)
{
    int ret;

    int len;

    char cmd;

    int i;

    unsigned char array_data[256] ={0} ;

    //指定server地址    
    struct sockaddr_in server;  
    bzero(&server,sizeof(server));  
    server.sin_family= AF_INET;  
    server.sin_port = htons(PORT);

    //data_cs_t* data_rev;
    
    //eg_tcp_send(CONNECT_STR,sizeof(CONNECT_STR)-1);
    //eg_tcp_send(connect_array,sizeof(connect_array));
    
#if 1
    while(1){
        memset(getByte,0,sizeof(getByte));
        printf("ready for rec\n");
        //if((ret=recv(sockfd,getByte,sizeof(MSG_HEAD)+3,0)) == -1){  
        if((ret=recv(sockfd,getByte,strlen(MSG_HEAD_str)+6,0)) == -1){ 
            
            printf("recv  header error\n");
            
            //close(sockfd);  
            //exit(1);
            continue;
        } 
/*
        while(ret == 0){

            printf("server disconnect ,connect ag!\n");
           if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){  
                printf("connect()error!,1 sec  retry! \n");  
                sleep(1);
                continue;
           }
           break;

        }
*/
        printf("Interpret data\n");
        //if((ret != (sizeof(MSG_HEAD)+3))||(memcmp(getByte,MSG_HEAD,4) != 0)){
        if((ret != (strlen(MSG_HEAD_str)+6))||(memcmp(getByte,MSG_HEAD_str,strlen(MSG_HEAD_str)) != 0)){

            printf("recv head error 2!,head is %s,ret is %d\n",getByte,ret);
            continue;

        }

/*
        cmd = getByte[4];
        len = ntohs(*(int*)(&getByte[5]));
*/

        cmd = getByte[9] - '0';

        len = ((0x00ff&c2i(getByte[10]))<<12)|((0x00ff&c2i(getByte[11]))<<8)|((0x00ff&c2i(getByte[12]))<<4)|(c2i(getByte[13]));

        printf("cmd is %d\n",cmd);
        printf("data len is %d,%c%c%c%c,str need 2X\n",len,getByte[10],getByte[11],getByte[12],getByte[13]);
        
        if((ret=recv(sockfd,getByte,len*2 + 8,0)) == -1){  //data package end 8byte
            
            printf("recv  data error\n");
            
            //close(sockfd);  
            //exit(1); 
            continue;
        } 

        
         // eg_tcp_main_proc(cmd,getByte,len*2);

        StrToHex(array_data,getByte,len);
        eg_tcp_main_proc(cmd,array_data,len);
       

    }

#else

    while(1){
        memset(getByte,0,sizeof(getByte));
        printf("ready for rec\n");
        //if((ret=recv(sockfd,getByte,sizeof(MSG_HEAD)+3,0)) == -1){  
        if((ret=recv(sockfd,getByte,sizeof(getByte),0)) == -1){ 
            
            printf("recv  header error\n");
            
            //close(sockfd);  
            //exit(1);  
        } 
        printf("explain data\n");

        for(i = 0;i < ret;i++){

            printf("%c",getByte[i]);

        }
       printf("\n");

    }
#endif

}

void eg_net_init(void)
{
    osal_task_t *tid;

    //创建套接字
      if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){  
          printf("socket()error\n");  
          //exit(1); 
          goto out; 
      }  
    
      //指定server地址    
      struct sockaddr_in server;  
      bzero(&server,sizeof(server));  
      server.sin_family= AF_INET;  
      server.sin_port = htons(PORT);  
      if( inet_pton(AF_INET, SERVERADDR, &server.sin_addr) <= 0){
          printf("inet_pton error for %s\n",SERVERADDR);
          //exit(0);
          goto out; 
      }
    
      //发起连接操作
      if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){  
          printf("connect()error\n");  
          //exit(1); 
          net_state = 0;
          goto out; 
      }
    
    tid = osal_task_create("tk_net",
                        net_thread_entry,
                        NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)

out:
   printf("jumpout tcp connect!\n");

}


