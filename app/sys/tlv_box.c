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
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "tlv_box.h"
int tlv_box_putobject (tlv_box_t * box, short type, void *value,
		       short length, int data_type);

static void
tlv_box_release_tlv (value_t value)
{
  tlv_t *tlv = (tlv_t *) value.value;
  free (tlv->value);
  free (tlv);
}

tlv_box_t *
tlv_box_create ()
{
  tlv_box_t *box = (tlv_box_t *) malloc (sizeof (tlv_box_t));
  box->m_list = key_list_create (tlv_box_release_tlv);
  box->m_serialized_buffer = NULL;
  box->m_serialized_bytes = 0;
  return box;
}

tlv_box_t *
tlv_box_parse (unsigned char *buffer, int buffersize)
{
  tlv_box_t *box = tlv_box_create ();

  unsigned char *cached = (unsigned char *) malloc (buffersize);
  memcpy (cached, buffer, buffersize);

  int offset = 0;
  short length = 0;
  while (offset < buffersize)
    {
      short type = (*(short *) (cached + offset));
      type = ntohs (type);
      offset += sizeof (short);
      short length = (*(short *) (cached + offset));
      length = ntohs (length);
      offset += sizeof (short);
      tlv_box_putobject (box, type, (void *) cached + offset, length,
			 TLV_DATA_TYPE_BYTES);
      offset += length;
    }

  box->m_serialized_buffer = cached;
  box->m_serialized_bytes = buffersize;

  return box;
}

int
tlv_box_destroy (tlv_box_t * box)
{
  key_list_destroy (box->m_list);
  if (box->m_serialized_buffer != NULL)
    {
      free (box->m_serialized_buffer);
    }

  free (box);

  return 0;
}

unsigned char *
tlv_box_get_buffer (tlv_box_t * box)
{
  return box->m_serialized_buffer;
}

int
tlv_box_get_size (tlv_box_t * box)
{
  return box->m_serialized_bytes;
}

int
tlv_box_putobject (tlv_box_t * box, short type, void *value, short length,
		   int data_type)
{
  if (box->m_serialized_buffer != NULL)
    {
      return -1;
    }

  tlv_t *tlv = (tlv_t *) malloc (sizeof (tlv_t));
  tlv->type = type;
  tlv->length = length;
  tlv->value = (unsigned char *) malloc (length);
  tlv->data_type = data_type;
  memcpy (tlv->value, value, length);

  value_t object;
  object.value = tlv;

  if (key_list_add (box->m_list, type, object) != 0)
    {
      return -1;
    }
  box->m_serialized_bytes += sizeof (short) * 2 + length;

  return 0;
}

int
tlv_box_put_char (tlv_box_t * box, short type, char value)
{
  return tlv_box_putobject (box, type, &value, sizeof (char),
			    TLV_DATA_TYPE_CHAR);
}

int
tlv_box_put_short (tlv_box_t * box, short type, short value)
{
  return tlv_box_putobject (box, type, &value, sizeof (short),
			    TLV_DATA_TYPE_SHORT);
}

int
tlv_box_put_int (tlv_box_t * box, short type, int value)
{
  return tlv_box_putobject (box, type, &value, sizeof (int),
			    TLV_DATA_TYPE_INT);
}

int
tlv_box_put_long (tlv_box_t * box, short type, long value)
{
  return tlv_box_putobject (box, type, &value, sizeof (long),
			    TLV_DATA_TYPE_LONG);
}


int
tlv_box_put_string (tlv_box_t * box, short type, char *value)
{
  return tlv_box_putobject (box, type, value, strlen (value) + 1,
			    TLV_DATA_TYPE_STRING);
}

int
tlv_box_put_bytes (tlv_box_t * box, short type, unsigned char *value,
		   short length)
{
  return tlv_box_putobject (box, type, value, length, TLV_DATA_TYPE_BYTES);
}

int
tlv_box_put_object (tlv_box_t * box, short type, tlv_box_t * object)
{
  return tlv_box_putobject (box, type, tlv_box_get_buffer (object),
			    tlv_box_get_size (object), TLV_DATA_TYPE_OBJECT);
}

int
tlv_box_serialize (tlv_box_t * box)
{
  if (box->m_serialized_buffer != NULL)
    {
      return -1;
    }

  int offset = 0;
  unsigned char *buffer = (unsigned char *) malloc (box->m_serialized_bytes);
  key_list_foreach (box->m_list, node)
  {
    tlv_t *tlv = (tlv_t *) node->value.value;
    short type = htons (tlv->type);
    memcpy (buffer + offset, &type, sizeof (short));
    offset += sizeof (short);
    short length = htons (tlv->length);
    memcpy (buffer + offset, &length, sizeof (short));
    offset += sizeof (short);

    if (tlv->data_type == TLV_DATA_TYPE_SHORT)
      {
	short t = *(short *) tlv->value;
	t = htons (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else if (tlv->data_type == TLV_DATA_TYPE_INT)
      {
	int t = *(int *) tlv->value;
	t = htonl (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else if (tlv->data_type == TLV_DATA_TYPE_LONG)
      {
	long t = *(long *) tlv->value;
	t = htonl (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else if (tlv->data_type == TLV_DATA_TYPE_LONGLONG)
      {
	long long t = *(long long *) tlv->value;
	t = htonl (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else if (tlv->data_type == TLV_DATA_TYPE_FLOAT)
      {
	float t = *(float *) tlv->value;
	t = htonl (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else if (tlv->data_type == TLV_DATA_TYPE_DOUBLE)
      {
	double t = *(double *) tlv->value;
	t = htonl (t);
	memcpy (buffer + offset, &t, tlv->length);
      }
    else
      {
	memcpy (buffer + offset, tlv->value, tlv->length);
      }

    offset += tlv->length;
  }

  box->m_serialized_buffer = buffer;

  return 0;
}

int
tlv_box_get_char (tlv_box_t * box, short type, char *value)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  *value = (*(char *) (tlv->value));
  return 0;
}

int
tlv_box_get_short (tlv_box_t * box, short type, short *value)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  *value = (*(short *) (tlv->value));
  *value = ntohs (*value);
  return 0;
}

int
tlv_box_get_int (tlv_box_t * box, short type, int *value)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  *value = (*(int *) (tlv->value));
  *value = ntohl (*value);
  return 0;
}

int
tlv_box_get_long (tlv_box_t * box, short type, long *value)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  *value = (*(long *) (tlv->value));
  *value = ntohl (*value);
  return 0;
}

int
tlv_box_get_string (tlv_box_t * box, short type, char *value, short *length)
{
  return tlv_box_get_bytes (box, type, value, length);
}

int
tlv_box_get_bytes (tlv_box_t * box, short type, unsigned char *value,
		   short *length)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  if (*length < tlv->length)
    {
      return -1;
    }
  memset (value, 0, *length);
  *length = tlv->length;
  memcpy (value, tlv->value, tlv->length);
  return 0;
}

int
tlv_box_get_bytes_ptr (tlv_box_t * box, short type, unsigned char **value,
		       short *length)
{
  value_t object;
  if (key_list_get (box->m_list, type, &object) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) object.value;
  *value = tlv->value;
  *length = tlv->length;
  return 0;
}

int
tlv_box_get_object (tlv_box_t * box, short type, tlv_box_t ** object)
{
  value_t value;
  if (key_list_get (box->m_list, type, &value) != 0)
    {
      return -1;
    }
  tlv_t *tlv = (tlv_t *) value.value;
  *object = (tlv_box_t *) tlv_box_parse (tlv->value, tlv->length);
  return 0;
}


#if 0
//#ifdef DEVEL
#define TEST_TYPE_0 0x601
#define TEST_TYPE_1 0x602
#define TEST_TYPE_2 0x603
#define TEST_TYPE_3 0x604
#define TEST_TYPE_4 0x605
#define TEST_TYPE_5 0x606
#define TEST_TYPE_6 0x607
#define TEST_TYPE_7 0x608
#define TEST_TYPE_8 0x609
#define TEST_TYPE_9 0x60a

#define LOG(format,...) printf(format,##__VA_ARGS__)

int
main_test (void)
{
  tlv_box_t *box = tlv_box_create ();
  tlv_box_put_char (box, TEST_TYPE_1, 'x');
  tlv_box_put_short (box, TEST_TYPE_2, (short) 2);
  tlv_box_put_int (box, TEST_TYPE_3, (int) 3);
  tlv_box_put_long (box, TEST_TYPE_4, (long) 4);
  tlv_box_put_string (box, TEST_TYPE_7, (char *) "hello world !");
  unsigned char array[6] = { 1, 2, 3, 4, 5, 6 };
  tlv_box_put_bytes (box, TEST_TYPE_8, array, 6);

  if (tlv_box_serialize (box) != 0)
    {
      LOG ("box serialize failed !\n");
      return -1;
    }

  LOG ("box serialize success, %d bytes \n", tlv_box_get_size (box));

  tlv_box_t *boxes = tlv_box_create ();
  tlv_box_put_object (boxes, TEST_TYPE_9, box);

  if (tlv_box_serialize (boxes) != 0)
    {
      LOG ("boxes serialize failed !\n");
      return -1;
    }

  LOG ("boxes serialize success, %d bytes \n", tlv_box_get_size (boxes));

  tlv_box_t *parsedBoxes =
    tlv_box_parse (tlv_box_get_buffer (boxes), tlv_box_get_size (boxes));

  LOG ("boxes parse success, %d bytes \n", tlv_box_get_size (parsedBoxes));

  tlv_box_t *parsedBox;
  if (tlv_box_get_object (parsedBoxes, TEST_TYPE_9, &parsedBox) != 0)
    {
      LOG ("tlv_box_get_object failed !\n");
      return -1;
    }

  LOG ("box parse success, %d bytes \n", tlv_box_get_size (parsedBox));

  {
    char value;
    if (tlv_box_get_char (parsedBox, TEST_TYPE_1, &value) != 0)
      {
	LOG ("tlv_box_get_char failed !\n");
	return -1;
      }
    LOG ("tlv_box_get_char success %c \n", value);
  }

  {
    short value;
    if (tlv_box_get_short (parsedBox, TEST_TYPE_2, &value) != 0)
      {
	LOG ("tlv_box_get_short failed !\n");
	return -1;
      }
    LOG ("tlv_box_get_short success %d \n", value);
  }

  {
    int value;
    if (tlv_box_get_int (parsedBox, TEST_TYPE_3, &value) != 0)
      {
	LOG ("tlv_box_get_int failed !\n");
	return -1;
      }
    LOG ("tlv_box_get_int success %d \n", value);
  }

  {
    long value;
    if (tlv_box_get_long (parsedBox, TEST_TYPE_4, &value) != 0)
      {
	LOG ("tlv_box_get_long failed !\n");
	return -1;
      }
    LOG ("tlv_box_get_long success %ld \n", value);
  }

  {
    char value[128];
    short length = 128;
    if (tlv_box_get_string (parsedBox, TEST_TYPE_7, value, &length) != 0)
      {
	LOG ("tlv_box_get_string failed !\n");
	return -1;
      }
    LOG ("tlv_box_get_string success %s \n", value);
  }

  {
    unsigned char value[128];
    short length = 128;
    if (tlv_box_get_bytes (parsedBox, TEST_TYPE_8, value, &length) != 0)
      {
	LOG ("tlv_box_get_bytes failed !\n");
	return -1;
      }

    LOG ("tlv_box_get_bytes success:  ");
    int i = 0;
    for (i = 0; i < length; i++)
      {
	LOG ("%d-", value[i]);
      }
    LOG ("\n");
  }

  tlv_box_destroy (box);
  tlv_box_destroy (boxes);
  tlv_box_destroy (parsedBox);
  tlv_box_destroy (parsedBoxes);

  return 0;
}

#endif
