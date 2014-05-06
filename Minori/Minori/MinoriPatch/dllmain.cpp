// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"

#include <Windows.h>

#include "detours.h"


void SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

////////////�����ַ���////////////////////////////////////////////////////////


PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;
	strcpy(lplf->lfFaceName, "����");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}


HANDLE WINAPI FixedCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{


	 return CreateFileA(
	 lpFileName,
	 dwDesiredAccess,
	 dwShareMode,
	 lpSecurityAttributes,
	 dwCreationDisposition,
	 dwFlagsAndAttributes,
	 hTemplateFile);
}

PVOID pCreateFileA = (PVOID)0x0001945D;
__declspec(naked) void _CreateFileA()
{
	__asm
	{
		push    ebx			// hTemplateFile
		push    0x80		// Attributes = NORMAL
		push    0x3			// Mode = OPEN_EXISTING
		push    ebx			//pSecurity
		push    0x1			// ShareMode = FILE_SHARE_READ
		push    0x80000000	// Access = GENERIC_READ
		push    eax			// FileName
		call FixedCreateFileA
		jmp pCreateFileA
	}
}

//��װHook 
void SetHook()
{
	SetNopCode((PBYTE)0x0001945D, 23);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldCreateFontIndirectA = DetourFindFunction("GDI32.dll", "CreateFontIndirectA");
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&pCreateFileA, _CreateFileA);
	DetourTransactionCommit();
}

__declspec(dllexport)void WINAPI Dummy()
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
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

