/*
 *  COPYRIGHT NOTICE  
 *  Copyright (C) 2015, Jhuster, All Rights Reserved
 *  Author: Jhuster(lujun.hust@gmail.com)
 *  
 *  https://github.com/Jhuster/TLV
 *   
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.  
 */
#ifndef _TLV_BOX_H_
#define _TLV_BOX_H_

#include "key_list.h"

#define LOG(format,...) printf(format,##__VA_ARGS__)

enum TLV_DATA_TYPE
{
  TLV_DATA_TYPE_CHAR,
  TLV_DATA_TYPE_SHORT,
  TLV_DATA_TYPE_INT,
  TLV_DATA_TYPE_LONG,
  TLV_DATA_TYPE_FLOAT,
  TLV_DATA_TYPE_DOUBLE,
  TLV_DATA_TYPE_LONGLONG,
  TLV_DATA_TYPE_BYTES,
  TLV_DATA_TYPE_STRING,
  TLV_DATA_TYPE_OBJECT,
  TLV_DATA_TYPE_COUNT
};

typedef struct _tlv
{
  short type;
  short length;
  unsigned char *value;
  int data_type;
} tlv_t;

typedef struct _tlv_box
{
  key_list_t *m_list;
  unsigned char *m_serialized_buffer;
  int m_serialized_bytes;
} tlv_box_t;

tlv_box_t *tlv_box_create ();
tlv_box_t *tlv_box_parse (unsigned char *buffer, int buffersize);
int tlv_box_destroy (tlv_box_t * box);

unsigned char *tlv_box_get_buffer (tlv_box_t * box);
int tlv_box_get_size (tlv_box_t * box);

int tlv_box_put_char (tlv_box_t * box, short type, char value);
int tlv_box_put_short (tlv_box_t * box, short type, short value);
int tlv_box_put_int (tlv_box_t * box, short type, int value);
int tlv_box_put_long (tlv_box_t * box, short type, long value);
int tlv_box_put_longlong (tlv_box_t * box, short type, long long value);
int tlv_box_put_float (tlv_box_t * box, short type, float value);
int tlv_box_put_double (tlv_box_t * box, short type, double value);
int tlv_box_put_string (tlv_box_t * box, short type, char *value);
int tlv_box_put_bytes (tlv_box_t * box, short type, unsigned char *value,
		       short length);
int tlv_box_put_object (tlv_box_t * box, short type, tlv_box_t * object);
int tlv_box_serialize (tlv_box_t * box);

int tlv_box_get_char (tlv_box_t * box, short type, char *value);
int tlv_box_get_short (tlv_box_t * box, short type, short *value);
int tlv_box_get_int (tlv_box_t * box, short type, int *value);
int tlv_box_get_long (tlv_box_t * box, short type, long *value);
int tlv_box_get_longlong (tlv_box_t * box, short type, long long *value);
int tlv_box_get_float (tlv_box_t * box, short type, float *value);
int tlv_box_get_double (tlv_box_t * box, short type, double *value);
int tlv_box_get_string (tlv_box_t * box, short type, char *value,
			short *length);
int tlv_box_get_bytes (tlv_box_t * box, short type, unsigned char *value,
		       short *length);
int tlv_box_get_bytes_ptr (tlv_box_t * box, short type, unsigned char **value,
			   short *length);
int tlv_box_get_object (tlv_box_t * box, short type, tlv_box_t ** object);

#endif //_TLV_BOX_H_
