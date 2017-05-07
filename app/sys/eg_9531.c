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


#include <sys/time.h>
#include <unistd.h>

#include "libubus.h"
#include "cv_cms_def.h"

long get_file_len(FILE *file)
{
    long len; 

    fseek (file , 0 , SEEK_END);  

    len =  ftell (file);  
    rewind(file);
    return len;

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
  //todo 添加异常处理  
    sprintf(cmd, "mv %s /root/cfg_reader", filename);
    system(cmd);

    sprintf(cmd, "rm %s", filename);
    system(cmd);

    return 1;
}


static struct ubus_context *ctx;
static struct blob_buf b;

static void test_client_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
	printf("test_client_subscribe_cb Start\n");
	fprintf(stderr, "Subscribers active: %d\n", obj->has_subscribers);
	printf("test_client_subscribe_cb End\n");
}

static struct ubus_object test_client_object = {
	.subscribe_cb = test_client_subscribe_cb,
};

static void test_client_notify_cb(struct uloop_timeout *timeout)
{
	printf("test_client_notify_cb Start\n");
	static int counter = 0;
	int err;
	struct timeval tv1, tv2;
	int max = 1000;
	long delta;

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "counter", counter++);

	gettimeofday(&tv1, NULL);
	err = ubus_notify(ctx, &test_client_object, "ping", b.head, 1000);
	gettimeofday(&tv2, NULL);
	if (err)
		fprintf(stderr, "Notify failed: %s\n", ubus_strerror(err));

	delta = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
	fprintf(stderr, "Avg time per iteration: %ld usec\n", delta / max);

	uloop_timeout_set(timeout, 10000);
	printf("test_client_notify_cb End\n");
}

static struct uloop_timeout notify_timer = {
	.cb = test_client_notify_cb,
};

static void client_main(void)
{
	uint32_t id;
	int ret;

	printf("client_main Start\n");
	ret = ubus_add_object(ctx, &test_client_object);
	if (ret) {
		fprintf(stderr, "Failed to add_object object: %s\n", ubus_strerror(ret));
		return;
	}

	if (ubus_lookup_id(ctx, "test", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "id", test_client_object.id);
	ubus_invoke(ctx, id, "watch", b.head, NULL, 0, 3000);
	test_client_notify_cb(&notify_timer);
	uloop_run();
	printf("client_main End\n");
}

void* eg_ubus_thread_entry(void *parameter)
{
	printf("main Start");
	const char *ubus_socket = NULL;
	int ch;

	uloop_init();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	client_main();

	ubus_free(ctx);
	uloop_done();

	printf("main End");
	return 0;
    sleep(10);
}



void eg_comm_init()
{

    osal_task_t *tid;


    tid = osal_task_create("tk_ubus",
                        eg_ubus_thread_entry,
                        NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)
    

}

