#ifndef _LIB_TLV_H_
#define _LIB_TLV_H_

//TLV-TYPE∂®“Â
#define TLV_TYPE_16    0x01     //UINT16
#define TLV_TYPE_32    0x02     //UINT32
#define TLV_TYPE_B     0x03     //BYTE

extern int iFindTlvList(unsigned char *pInBuf, int iBufLen, unsigned short usTag, int iDataType, unsigned char *pValue, int *pLen);
extern int iFindTlvListandSet(unsigned char *pInBuf, int iBufLen, unsigned short usTag, int iDataType, unsigned char *pValue);
extern int iAddTlvList(unsigned char *pInBuf, unsigned short usTag, int iDataType, unsigned char *pValue, int iLen);
extern const unsigned char * UTIL_FindT1L1V(const unsigned char * tlvs, int tlvsLen, unsigned char tag, int * outLen);
extern unsigned int UTIL_UInt16FromBytes(const unsigned char * input);
#endif
