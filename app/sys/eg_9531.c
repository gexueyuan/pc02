/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : eg_9531.c
 @brief  : ubus for process communication,msg for thread
 @author : gexueyuan
 @history:
           2017-4-28    gexueyuan    Created file
           ...
******************************************************************************/
#if 0

#include <sys/time.h>
#include <unistd.h>

#include "cv_cms_def.h"
#include "tlv_box.h"

int mv_file(unsigned char *filename,unsigned char *newname)
{
    char cmd[200];

    //todo 添加异常处理  
    sprintf(cmd, "cp %s /root/%s", filename,newname);
    system(cmd);

    sprintf(cmd, "rm %s", filename);
    system(cmd);


}

long get_file_len(FILE *file)
{
    long len; 

    fseek (file , 0 , SEEK_END);  

    len =  ftell (file);  
    rewind(file);
    return len;

}

int cfg_door_base(char *filename,int no)
{

    char *mem;
    FILE *fd;
    int i;
    char cmd[200];
    long file_len;
    short type,length;
    config_door_t *p_cfg;
    if ((fd = fopen(filename, "rb")) == NULL) {
        
        printf("F[%s] L[%d] open %s error!!!", __FILE__, __LINE__,filename);

        return (NULL);
    
    }

    printf("Reading door base cfg '%s'\n", filename);

    fflush(stdout);

    file_len = get_file_len(fd);
    
    //p_cfg = &controll_eg.door_cfg[no];

//    for(i = READER1_CFG;i < READER8_CFG; i++ ){
//               
//        p_reader_cfg = &controll_eg.reader_cfg[i];
//        
//        fread((char *)&type, sizeof(short), 1, fd );
//        fread((char *)&length, sizeof(short), 1, fd );      
//        fread((char *)p_reader_cfg, 5, 1, fd );

//    }


    fclose(fd);

    printf("Done\n");
    fflush(stdout);

    return 1;
}

int cfg_time(unsigned char *filename)
{
    char *buffer;
    FILE *fd;
    int result;
    short i;
    char cmd[200];
    long file_len;
    short type,length;
   // config_door_t *p_door_cfg;// = &p_controll_eg->door_cfg;

    uint8_t door_no = 0;
    uint16_t time_buffer_len = 0xffff;
    if ((fd = fopen(filename, "rb")) == NULL) {
        
        printf("F[%s] L[%d] open %s error!!!", __FILE__, __LINE__,filename);

        return (NULL);
    
    }

    printf("Reading time cfg '%s'\n", filename);

    fflush(stdout);

    file_len = get_file_len(fd);
    
    /* 分配内存存储整个文件 */   
    buffer = (char*) malloc (sizeof(char)*file_len);  
    if (buffer == NULL)  
    {  
        fputs ("Memory error",stderr);   
        return (2);  
    }  

    /* 将文件拷贝到buffer中 */  
    result = fread (buffer,1,file_len,fd);  
    if (result != file_len)  
    {  
        fputs ("Reading error",stderr);  
        return (3);  
    }  


    tlv_box_t *box = tlv_box_create ();

    box = tlv_box_parse(buffer,file_len);

    if (tlv_box_serialize (box) != 0){
        
        LOG ("box serialize failed !\n");
        return (-1);
    
    }

    tlv_box_get_char(box,TIME_CFG_DOOR_NO,&door_no);

   // p_door_cfg = &p_controll_eg->door_cfg[door_no];



    for(i = TIME_CFG_MON;i < TIME_CFG_DOOR_HOL + 1;i++){

        if(p_door_cfg->cfg_time_door[i - TIME_CFG_MON].data_buffer != NULL){

            free(p_door_cfg->cfg_time_door[i - TIME_CFG_MON].data_buffer);
        }
        time_buffer_len =  tlv_box_get_length(box,i);
        p_door_cfg->cfg_time_door[i - TIME_CFG_MON].len = time_buffer_len;

        p_door_cfg->cfg_time_door[i - TIME_CFG_MON].data_buffer = (unsigned char*)malloc(time_buffer_len);

        tlv_box_get_bytes(box,i,&p_door_cfg->cfg_time_door[i - TIME_CFG_MON].data_buffer,&p_door_cfg->cfg_time_door[i - TIME_CFG_MON].len);
        
    }

    tlv_box_destroy (box);

    return 1;

}

int cfg_reader_RD(char *filename)
{

    char *mem;
    FILE *fd;
    int i;
    char cmd[200];
    long file_len;
    short type,length;
    reader_cfg_t *p_reader_cfg;
    if ((fd = fopen(filename, "rb")) == NULL) {
        
        printf("F[%s] L[%d] open %s error!!!", __FILE__, __LINE__,filename);

        return (NULL);
    
    }

    printf("Reading cfg '%s'\n", filename);

    fflush(stdout);

    file_len = get_file_len(fd);
    

//    fread((char *) &n1, sizeof(int), 1, fd );
//    fread((char *) &n2, sizeof(int), 1, fd );
//    fread((char *) &n3, sizeof(int), 1, fd );
    for(i = READER1_CFG;i < READER8_CFG; i++ ){
        
        //p_reader_cfg = &(controll_eg.reader_cfg[i]);

        
        p_reader_cfg = &controll_eg.reader_cfg[i];
        
        //fseek(fd,4,SEEK_SET);
        fread((char *)&type, sizeof(short), 1, fd );
        fread((char *)&length, sizeof(short), 1, fd );      
        fread((char *)p_reader_cfg, 5, 1, fd );

    }

    fread((char*)&controll_eg.sys_ctrl,sizeof(char),1,fd);
    fread((char*)&controll_eg.legacy_WG,sizeof(char),1,fd);

    fclose(fd);

    printf("Done\n");
    fflush(stdout);

    return 1;
}
#endif
