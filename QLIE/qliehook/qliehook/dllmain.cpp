// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"

#include <string>
#include <map>
#include <stdio.h>

#include "zconf.h"
#include "zlib.h"
#include "detours.h"

using namespace std;

#pragma pack (1)
typedef struct {
	char magic[8];			/* "FUYINPAK" */
	DWORD header_length;
	DWORD is_compressed;		
	DWORD index_offset;
	DWORD index_length;	
} header_t;

typedef struct {
	DWORD name_offset;
	DWORD name_length;		
	DWORD offset;
	DWORD length;
	DWORD org_length;			
} entry_t;

typedef struct {
	DWORD offset;
	DWORD length;
	DWORD org_length;
} info_t;

typedef struct {
	char name[MAX_PATH];
	info_t info;
} my_entry_t;

#pragma pack ()
//////////////ȫ�ֱ���//////////////////////////////////////////////////////////////

typedef map<string, info_t> StringMap;
StringMap MyNameDic;

char* Name = NULL;
DWORD NameLength = 0;

///////////////���ù���//////////////////////////////////////////////////////////////
void SetNopCode(BYTE* pnop,size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop,size,PAGE_EXECUTE_READWRITE,&oldProtect);
	for(size_t i=0;i<size;i++)
	{
		pnop[i] = 0x90;
	}
}

void memcopy(void* dest,void*src,size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest,size,PAGE_EXECUTE_READWRITE,&oldProtect);
	memcpy(dest,src,size);
}

///////////////Hook CreateFontIndirect//////////////////////////////////////////////////

PVOID pOldLogFont = (void*)0x493152;


void NewLogFont(char *lplf)
{
	lplf[0x17] = ANSI_CHARSET;
	strcpy(&lplf[0x1C],"����");
}

__declspec(naked)void _NewLogFont()
{
	__asm
	{
		pushad
		push eax
		call NewLogFont
		add esp,4 //���ֶ�ջƽ��
		popad
		jmp pOldLogFont
	}
}

/////////////����//////////////////////////////////////////////////////////////
void InstallBorderPatch()
{   
	//�߽���
	BYTE Patch1[] = {0xFE};
	BYTE Patch2[] = {0xFF};

	//һ����9���߽���
	memcopy((void*)0x492684, Patch1, sizeof(Patch1));
	memcopy((void*)0X46268C, Patch2, sizeof(Patch2));

	memcopy((void*)0x4D3FAE, Patch1, sizeof(Patch1));
	memcopy((void*)0x4D3FB6, Patch2, sizeof(Patch2));

	memcopy((void*)0x483506, Patch1, sizeof(Patch1));
	memcopy((void*)0x48350B, Patch2, sizeof(Patch2));

	memcopy((void*)0x4839BD, Patch1, sizeof(Patch1));
	memcopy((void*)0x4839C1, Patch2, sizeof(Patch2));

	memcopy((void*)0x4919C3, Patch1, sizeof(Patch1));
	memcopy((void*)0x4919CB, Patch2, sizeof(Patch2));

	memcopy((void*)0x492131, Patch1, sizeof(Patch1));
	memcopy((void*)0x492139, Patch2, sizeof(Patch2));

	memcopy((void*)0x4939DB, Patch1, sizeof(Patch1));
	memcopy((void*)0x4939E3, Patch2, sizeof(Patch2));

	memcopy((void*)0x4E5DD1, Patch1, sizeof(Patch1));
	memcopy((void*)0x4E5DD5, Patch2, sizeof(Patch2));

	memcopy((void*)0x565FB5, Patch1, sizeof(Patch1));
	memcopy((void*)0x565FB9, Patch2, sizeof(Patch2));

}

///////////////Copy�Լ����ļ����ڴ�//////////////////////////////////////////////////////////////


PVOID pNameLength = (PVOID)0x4875FE;
__declspec(naked)void GetNameLength()
{
	__asm
	{
		pushad
		mov NameLength, eax
		popad
		jmp pNameLength
	}
}

PVOID pGetName = (PVOID)0x487611;
__declspec(naked)void GetName()
{
	__asm
	{
		pushad
		mov Name, edi
		popad
		jmp pGetName
	}
}

void FillMyNameDic()
{
	HANDLE hfile=CreateFileA("GameData\\res.dat",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if(hfile)
	{
		DWORD size = GetFileSize(hfile,NULL);
		BYTE *buff = new BYTE[size];
		DWORD len=0;
		if(!ReadFile(hfile,(LPVOID)buff,size,&len,NULL))
		{
			MessageBoxA(NULL,"��ȡres.datʧ��!","Error",MB_OK);
			CloseHandle(hfile);
		}
		CloseHandle(hfile);


		header_t *header = (header_t*)buff;
		entry_t *entry = (entry_t*)(buff + header->index_offset);

		DWORD entry_count = header->index_length/sizeof(entry_t);

		my_entry_t * my_entry = new my_entry_t[entry_count];


		int i = 0;
		for(i=0; i < entry_count; i++)
		{
			memcpy(my_entry[i].name, buff+entry[i].name_offset, entry[i].name_length-1);

			my_entry[i].info.offset = entry[i].offset;
			my_entry[i].info.length = entry[i].length;
			my_entry[i].info.org_length = entry[i].org_length;

			MyNameDic.insert(StringMap::value_type(my_entry[i].name, my_entry[i].info));
		}

		delete []buff;
		delete[]my_entry;
	}
	else
	{
		MessageBoxA(NULL,"��ȡres.datʧ��!","Error",MB_OK);
	}
}

void WINAPI CopyMyFile(DWORD offset)
{
	static bool isPatched = false;
	string name(Name, NameLength);
	//�����ǰ�ļ������ֵ���:
	if(MyNameDic.count(name))
	{
		StringMap::iterator iter;
		iter = MyNameDic.find(name);
		info_t info = iter->second;
		
		DWORD off = info.offset;
		DWORD len = info.length;
		DWORD orglen = info.org_length;

		SetNopCode((PBYTE)0x484033, 6);
		isPatched = true;

		HANDLE hfile=CreateFileA("GameData\\res.dat",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
		SetFilePointer(hfile, off, NULL, FILE_BEGIN);
		BYTE *buff = new BYTE[len];
		DWORD readsize=0;
		if(!ReadFile(hfile,(LPVOID)buff,len,&readsize,NULL))
		{
			MessageBoxA(NULL,Name,"Error",MB_OK);
			CloseHandle(hfile);
		}
		BYTE *data = new BYTE[orglen];
		if(uncompress(data, &orglen, buff, len) == Z_OK)
		{
			memcpy((LPVOID)offset, data, orglen);
		}
		else
		{
			MessageBoxA(NULL,"��ѹʧ��","Error",MB_OK);
		}

		delete []data;
		delete []buff;
		CloseHandle(hfile);
		
	}
	else
	{
		if(isPatched)
		{
			BYTE Patch[] = {0x0F, 0x82, 0xF5, 0xFE, 0xFF, 0xFF};
			memcopy((PVOID)0x484033, Patch, sizeof(Patch));
			isPatched = false;
		}
	}
	
}

PVOID pCopyMyFile = (PVOID)0x483F10;
__declspec(naked)void _CopyMyFile()
{
	__asm
	{
		pushad
		push eax
		call CopyMyFile
		popad
		jmp pCopyMyFile
	}
}


void SetHook()
{

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pOldLogFont, _NewLogFont);
	DetourTransactionCommit();
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pCopyMyFile,_CopyMyFile);
	DetourTransactionCommit();
	

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pNameLength,GetNameLength);
	DetourTransactionCommit();
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pGetName,GetName);
	DetourTransactionCommit();
	
	InstallBorderPatch();

	FillMyNameDic();
}

void DropHook()
{
	MyNameDic.clear();
}


__declspec(dllexport)void WINAPI Empty()
{

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SetHook();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DropHook();
		break;
	}
	return TRUE;
}

