#ifndef __RCT_H__
#define __RCT_H__

#include <Windows.h>
#include <stdio.h>

#include "png.h"
#include "zconf.h"
#include "zconf.h"


#pragma pack (1)

typedef struct {
	BYTE magic[8];		/* "����TC00" and "����TC01" and "����TS00" and "����TS01" */
	DWORD width;
	DWORD height;
	DWORD data_length;
} rct_header_t;

typedef struct {		/* ������0x300�ֽ��ǵ�ɫ�� */
	BYTE magic[8];		/* "����8_00" */
	DWORD width;
	DWORD height;
	DWORD data_length;
} rc8_header_t;

#pragma pack ()		


class RCT
{
public:

private:
	DWORD rc8_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);
	DWORD rct_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);
};
#endif