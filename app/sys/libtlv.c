#ifdef USE_LIBTLV
#include "global_sys.h"
#include "libtlv.h"

unsigned int UTIL_UInt16FromBytes(const unsigned char * input)
{
	return ((input[0] << 8) | input[1]);
}

int UTIL_UInt16ToBytes(unsigned int n, unsigned char * output)
{
	output[0] = (BYTE)((n >> 8) & 0xFF);
	output[1] = (BYTE)((n) & 0xFF);
	return 2;
}

unsigned int UTIL_UInt32FromBytes(const unsigned char * input)
{
	return ((input[0] << 24) | (input[1] << 16) | (input[2] << 8) | input[3]);
}

int UTIL_UInt32ToBytes(unsigned int n, unsigned char * output)
{
	output[0] = (BYTE)((n >> 24) & 0xFF);
	output[1] = (BYTE)((n >> 16) & 0xFF);
	output[2] = (BYTE)((n >> 8) & 0xFF);
	output[3] = (BYTE)((n) & 0xFF);
	return 4;
}

const unsigned char * UTIL_FindT1L2V(const unsigned char * tlvs, int tlvsLen, unsigned char tag, int * outLen)
{
	int i,len_t;
	for (i=0; i<tlvsLen; )
	{
		len_t = UTIL_UInt16FromBytes(tlvs+i+1);
		if (tlvs[i] == tag)
		{
			if ((i+3+len_t) > tlvsLen)
			{
				break;
			}
			*outLen = len_t;
			return (tlvs+i+3);
		}
		i += (3 + len_t);
	}
	* outLen = 0;
	return NULL;
}

int UTIL_CopyT1L2Vs(const unsigned char * tlvs, int tlvsLen, const unsigned char * tags, int tagsLen, unsigned char * out)
{
	int i,j,outOffset=0;
	for (i=0; i<tlvsLen; )
	{
		int len = UTIL_UInt16FromBytes(tlvs+i+1);
		for (j=0; j<tagsLen; j++)
		{
			if (tags[j] == tlvs[i])
			{
				memcpy(out+outOffset, tlvs+i, (3+len));
				outOffset += (3+len);
				break;
			}
		}
		i += (3 + len);
	}
	return outOffset;
}

const unsigned char * UTIL_FindT2L2V(const unsigned char * tlvs, int tlvsLen, unsigned int tag, int * outLen)
{
	int i, tag_t, len_t;
	for (i=0; i<tlvsLen; )
	{
		tag_t = UTIL_UInt16FromBytes(tlvs+i);
		len_t = UTIL_UInt16FromBytes(tlvs+i+2);
		if (tag_t == tag)
		{
			if ((i+4+len_t) > tlvsLen)
			{
				break;
			}
			*outLen = len_t;
			return (tlvs+i+4);
		}
		i += (4 + len_t);
	}
	* outLen = 0;
	return NULL;
}

int UTIL_GetT2L2V(const unsigned char * tlvs, int tlvsLen, unsigned int tag,  unsigned char * output)
{
	int len = 0;
	const unsigned char * p = UTIL_FindT2L2V(tlvs, tlvsLen, tag, &len);
	if ((p != NULL) && (output != NULL))
	{
		memcpy(output, p, len);
	}
	return len;
}

int UTIL_SetT2L2V(unsigned char * tlvs, int tlvsLen, unsigned int tag,  unsigned char * output)
{
	int i, tag_t, len_t;
	if(output == NULL)
		return -1;
		
	for (i=0; i<tlvsLen; )
	{
		tag_t = UTIL_UInt16FromBytes(tlvs+i);
		len_t = UTIL_UInt16FromBytes(tlvs+i+2);
		if (tag_t == tag)
		{
			if ((i+4+len_t) > tlvsLen)
			{
				break;
			}
			memcpy((tlvs+i+4),output,len_t);
			return 0;
		}
		i += (4 + len_t);
	}
	return 0;
}

int UTIL_AppendT2L2V(unsigned char * tlvs, unsigned int tag, const unsigned char * value, int vLen)
{
	UTIL_UInt16ToBytes(tag, tlvs);
	UTIL_UInt16ToBytes(vLen, tlvs+2);
	if (value != NULL)
	{
		memcpy(tlvs+4, value, vLen);
	}
	return 4+vLen;
}

int UTIL_AppendT2L2UInt32(unsigned char * tlvs, unsigned int tag, unsigned int value)
{
	UTIL_UInt16ToBytes(tag, tlvs);
	UTIL_UInt16ToBytes(4, tlvs+2);
	UTIL_UInt32ToBytes(value, tlvs+4);
	return 4+4;
}

int iFindTlvList(unsigned char *pInBuf, int iBufLen, unsigned short usTag, int iDataType, unsigned char *pValue, int *pLen)
{
	if(iDataType == TLV_TYPE_16)
	{
		unsigned short usTmp = 0;
		*pLen = UTIL_GetT2L2V(pInBuf, iBufLen, usTag, (unsigned char *)&usTmp);
		usTmp = ntohs(usTmp);
		memcpy(pValue, &usTmp, 2);
	}
	else if(iDataType == TLV_TYPE_32)
	{
		int iTmp = 0;
		*pLen = UTIL_GetT2L2V(pInBuf, iBufLen, usTag, (unsigned char *)&iTmp);
		iTmp = ntohl(iTmp);
		memcpy(pValue, &iTmp, 4);
	}
	else
	{
		*pLen = UTIL_GetT2L2V(pInBuf, iBufLen, usTag, pValue);
	}
	return 0;
}

int iFindTlvListandSet(unsigned char *pInBuf, int iBufLen, unsigned short usTag, int iDataType, unsigned char *pValue)
{
	if(iDataType == TLV_TYPE_16)
	{
		unsigned short usTmp = htons((short)*pValue);
		UTIL_SetT2L2V(pInBuf, iBufLen, usTag, (unsigned char *)&usTmp);
	}
	else if(iDataType == TLV_TYPE_32)
	{
		int iTmp = htonl((int)*pValue);
		UTIL_SetT2L2V(pInBuf, iBufLen, usTag, (unsigned char *)&iTmp);
	}
	else
	{
		UTIL_SetT2L2V(pInBuf, iBufLen, usTag, pValue);
	}
	return 0;
}

int iAddTlvList(unsigned char *pInBuf, unsigned short usTag, int iDataType, unsigned char *pValue, int iLen)
{
	if(iLen == 0)
	{
		return 0;
	}
	if(iDataType == TLV_TYPE_16)
	{
		unsigned short usTmp = 0;
		memcpy(&usTmp, pValue, 2);
		usTmp = htons(usTmp);
		return UTIL_AppendT2L2V(pInBuf, usTag, (unsigned char *)&usTmp, 2);
	}
	else if(iDataType == TLV_TYPE_32)
	{
		int iTmp = 0;
		memcpy(&iTmp, pValue, 4);
		iTmp = htonl(iTmp);
		return UTIL_AppendT2L2V(pInBuf, usTag, (unsigned char *)&iTmp, 4);
	}
	else
	{
		return UTIL_AppendT2L2V(pInBuf, usTag, pValue, iLen);
	}
}

int UTIL_GetCosLen(const unsigned char *tlvs, int tlvsLen)
{
	int unLength = 0;
	int j = 0;
  for (j=0; j<tlvsLen; j++)
  {
      unLength = (unLength << 8) + (*(tlvs++));
  }
  return unLength;
}


//Õë¶ÔCOSÊý¾Ý
const unsigned char * UTIL_FindT1L1V(const unsigned char * tlvs, int tlvsLen, unsigned char tag, int * outLen)
{
	int i,len_t,data_len;
	for (i=0; i<tlvsLen; )
	{
		if (tlvs[i] == tag)
		{
			if (tlvs[i+1] & 0x80) 
			{
				len_t = tlvs[i+1] & 0x7F;
				*outLen = UTIL_GetCosLen(tlvs+i+1+1, len_t);
				return (tlvs+i+1+1+len_t);
			}
			else
			{
				*outLen = tlvs[i+1] & 0x7F;
				return (tlvs+i+1+1);
			}
			break;
		}
		else
		{
			if(tlvs[i+1] & 0x80)
			{
				len_t = tlvs[i+1] & 0x7F;
				data_len = UTIL_GetCosLen(tlvs+i+1+1, len_t);
				i = i + 1 + 1 + len_t + data_len;
			}
			else
			{
				data_len = tlvs[i+1] & 0x7F;
				i = i + 1 + 1 + data_len;
			}
		}
		
	}
	
	* outLen = 0;
	return NULL;
}
#endif