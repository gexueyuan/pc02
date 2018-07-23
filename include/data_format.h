/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : data_format.h
 @brief  : data format analyse
 @author : gexueyuan
 @history:
           2018-5-17    gexueyuan    Created file
           ...
******************************************************************************/
#ifndef __DATA_FORMAT_H__
#define __DATA_FORMAT_H__

#define __COMPILE_PACK__ __attribute__((packed))

/* 开门结果 */
typedef enum _OPEN_RESULT {
    OR_SUCCESS = 0x01,/* 成功 */
    OR_SUCCESS_C = 0x02,/* 成功，卡片jm算法异常 */
    OR_SUCCESS_C = 0x03,/* 成功，读卡器jm算法异常 */
    OR_INVALID_WL = 0xA0,		
    OR_ILLEGAL,
    OR_OUT_TIME,
    OR_INEFFECTIVE_TIME,
    




	
} E_OPEN_RESULT;


typedef enum _ALARM_LIST {
    ALARM_321_FIRE = 0x81,
    ALARM_321_BROKEN = 0x8E,
    ALARM_321_FIX = 0x8F,

    ALARM_card_icbroke = 0x90,

    ALARM_rd_fix = 0x91,
    ALARM_ts_bk = 0x92,
    ALARM_an_bk = 0x93,
    ALARM_rd_ic_bk = 0x94,
    ALARM_voltage = 0x95,
    ALARM_current = 0x96,
    ALARM_tem = 0x97,

    ALARM_322_overtime = 0xA9,
    ALARM_322_force = 0xAC,
    ALARM_322_abnormal= 0xAD,
	ALARM_322_failed= 0xAE,
    ALARM_322_offline= 0xAF,
} E_ALARM_LIST;

typedef struct _card_log {
    uint16_t version;
/*card record*/	
	uint8_t card_id[4];
	uint32_t card_cnt;
	uint8_t reader_id[4];
	uint32_t reader_random_num;
	uint8_t card_sign[72];
/*322 record*/
	uint8_t ctrl_322_id[4];
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint8_t RFU;
	uint8_t ctrl_sign[64];
} card_log_t;

typedef struct _remote_log {
    uint16_t version;
/*card record*/	
	uint32_t card_id;
	uint32_t card_cnt;
	uint32_t reader_id;//desktop reader
	uint32_t reader_random_num;
	uint8_t opera_type;
	uint32_t target_reader_id;
	uint8_t start_date[6];
	uint8_t end_date[6];
	uint8_t card_sign[72];
/*desktop reader*/
	uint32_t desktop_reader_cnt;
	uint8_t desktop_reader_time[6];
	uint8_t t_sign[64];
/*322 record*/
	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint16_t RFU;
	uint8_t ctrl_sign[64];
} remote_log_t;

typedef struct _alarm_log {//88 byte
    uint16_t version;
/*322 record*/
	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t alarm_src;
	uint8_t alarm_type;
	uint8_t power_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint8_t RFU[3];
	uint8_t ctrl_sign[64];
} alarm_log_t;

typedef struct _ID_log {//192
    uint16_t version;
/*card record*/	
	uint32_t reader_id;
	uint8_t opera_time[6];
	uint8_t id_dn[32];
	uint8_t card_sign[64];
/*322 record*/
	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint8_t RFU[3];
	uint8_t ctrl_sign[64];
} ID_log_t;


typedef struct _face_remote_log {//162
    uint16_t version;
/*card record*/	
	uint32_t card_id;
	uint8_t opera_time[6];
	uint32_t face_id;//desktop reader
	uint8_t card_sign[64];
/*322 record*/
	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint8_t RFU;
	uint8_t ctrl_sign[64];
} face_remote_log_t;

typedef struct _double_element_log {//162
    uint16_t version;
/*face record*/	
	uint32_t reader_id;
	uint8_t opera_time[6];
	uint32_t face_id;//desktop reader
	uint8_t card_sign[64];
/*card record*/
	uint32_t card_id;
	uint32_t card_cnt;
	uint32_t reader_id_2;
	uint32_t reader_random_num;
	uint8_t ctrl_sign[72];
/*322 record*/
	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t result;
	uint8_t RFU;
	uint8_t ctrl_sign[64];	
} double_element_log_t;

typedef struct _white_list_cnf {
	uint32_t card_id;
	uint8_t name[31];
	uint8_t door_e[5];
} white_list_cnf_t;


#endif
