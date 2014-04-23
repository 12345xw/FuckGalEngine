#ifndef __RCT_H__
#define __RCT_H__

#include <Windows.h>
#include <stdio.h>
#include <string>

#include "zlib.h"
#include "zconf.h"
#include "png.h"
#include "pngconf.h"


using namespace std;

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

 typedef struct _pic_data
{
	unsigned int width, height; /* �ߴ� */
	int bit_depth;  /* λ�� */
	int flag;   /* һ����־����ʾ�Ƿ���alphaͨ�� */

	unsigned char *rgba; /* ͼƬ���� */
 } pic_data;

#pragma pack ()		

#define PNG_BYTES_TO_CHECK 4
#define HAVE_ALPHA 1
#define NO_ALPHA 0


class RCT
{
public:
	RCT();
	bool LoadRCT(string fname);
	void RCT2PNG();
	~RCT();
private:
	DWORD rc8_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);
	DWORD rct_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);

	int dump_rc8(rc8_header_t *rc8, BYTE *&ret_rgb);
	int dump_rct(rct_header_t *rct, BYTE *&ret_rgb);

	int read_png_file(string filepath, pic_data *out);
	int write_png_file(string file_name, pic_data *graph);

	string fn_rct; //��ǰ������rct�ļ���
	FILE *f_rct;
	BYTE* p_rct;
	DWORD size_rct;
	rct_header_t *h_rct; //��LoadRCT�г�ʼ����ָ���ַ�� p_rct ��ͬ

	string fn_rc8; //��ǰ������rc8�ļ���
	FILE *f_rc8;
	BYTE* p_rc8;
	DWORD size_rc8;
	rc8_header_t *h_rc8; //��LoadRCT�г�ʼ����ָ���ַ�� p_rc8 ��ͬ

	pic_data png_info;
};
#endif