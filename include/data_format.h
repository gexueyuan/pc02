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

typedef struct _card_log {
    uint16_t version;
	
	uint32_t card_id;
	uint32_t card_cnt;
	uint32_t reader_id;
	uint32_t reader_random_num;
	uint8_t card_sign[72];

	uint32_t ctrl_322_id;
	uint32_t ctrl_322_cnt;
	uint8_t ctrl_time[6];
	uint8_t door_num;
	uint8_t in_out;
	uint8_t 
} card_log_t;




#endif
