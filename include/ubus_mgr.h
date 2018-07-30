#ifndef UBUS_MGR_H
#define UBUS_MGR_H
#include <stdio.h>  
#include <errno.h>  
#include <unistd.h>  
#include <pthread.h>  

#include <libubus.h>  
#include <libubox/uloop.h>  
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <libubox/list.h>  
#include <libubox/blobmsg_json.h>  
#include <json-c/json.h>  

//#include "321_mgr.h"


#include <libubox/ustream.h>  
#include <libubox/utils.h>  

 
struct event_base *evloop;
struct event *e;

#define TAG_KEY_AGREEMENT			0x009E
#define TAG_GEN_RANDOM				0x0085

#define TAG_UPDATE_WHITELIST		0x8017
#define TAG_UPDATE_WHITELIST_CFG	0x8018

#define TAG_GET_WHITELIST			0x0001
#define TAG_GET_WHITELIST_CFG		0x0002
#define TAG_UPTATE_RTC				0x0003
#define TAG_GET_SECURE_CHANNEL_PKC	0x0004
#define TAG_GET_WHITELIST_PKC		0x0005

int ubus_init(void);  
int ubus_release(void);

int add_ubus_event(void);
int remove_ubus_event(void);

int add_alarm_timer_event(void);
int remove_alarm_timer_event(void);

#endif //UBUS_MGR_H