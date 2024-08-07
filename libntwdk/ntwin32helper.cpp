/*
 *
 *  Win32 API helper functions
 *
 *  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
 *  Licensed under the MIT License.
 *
 */
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <strsafe.h>
#include "ntwin32helper.h"

int WinGetDateString(ULONG64 DateTime,LPTSTR pszText,int cchTextMax,LPCTSTR DateFormat,BOOLEAN bTimeAsUTC,ULONG Flags)
{
	int cch = 0;
	BOOL bSuccess;
    SYSTEMTIME st;
    FILETIME ftLocal;

    if( bTimeAsUTC )
    {
        bSuccess = FileTimeToSystemTime((FILETIME*)&DateTime,&st);
    }
    else
    {
        bSuccess = FileTimeToLocalFileTime((FILETIME*)&DateTime,&ftLocal);
		if( bSuccess && ftLocal.dwHighDateTime < 0x80000000 )
			bSuccess = FileTimeToSystemTime(&ftLocal,&st);
		else
			bSuccess = FALSE;
    }

	if( bSuccess )
	{
		cch = GetDateFormat(LOCALE_USER_DEFAULT,
			        Flags,
				    &st, 
					DateFormat,
					pszText,cchTextMax);
	}

	return cch;
}

int WinGetTimeString(ULONG64 DateTime,LPTSTR pszText,int cchTextMax,LPCTSTR TimeFormat,BOOLEAN bTimeAsUTC,ULONG Flags)
{
	int cch = 0;
	BOOL bSuccess;
    SYSTEMTIME st;
    FILETIME ftLocal;

    if( bTimeAsUTC )
    {
        bSuccess = FileTimeToSystemTime((FILETIME*)&DateTime,&st);
    }
    else
    {
        bSuccess = FileTimeToLocalFileTime((FILETIME*)&DateTime,&ftLocal);
		if( bSuccess && ftLocal.dwHighDateTime < 0x80000000 )
	        bSuccess = FileTimeToSystemTime(&ftLocal,&st);
		else
			bSuccess = FALSE;
    }

	if( bSuccess )
	{
	    cch = GetTimeFormat(LOCALE_USER_DEFAULT,
                Flags,
                &st, 
                TimeFormat,
                pszText,cchTextMax);
	}

	return cch;
}

BOOLEAN
WINAPI
WinFileTimeToDosDateTime(
    __in   const LARGE_INTEGER *pliFileTime,
    __out  PUSHORT pFatDate,
    __out  PUSHORT pFatTime
    )
{
    FILETIME ft;
    ft.dwHighDateTime = pliFileTime->HighPart;
    ft.dwLowDateTime = pliFileTime->LowPart;
    return (BOOLEAN)FileTimeToDosDateTime(&ft,pFatDate,pFatTime);
}

BOOLEAN
WINAPI
WinDosDateTimeToFileTime(
    __in   USHORT wFatDate,
    __in   USHORT wFatTime,
    __out  LARGE_INTEGER *pliFileTime
    )
{
    FILETIME ft;
    if( DosDateTimeToFileTime(wFatDate,wFatTime,&ft) )
    {
        pliFileTime->HighPart = ft.dwHighDateTime;
        pliFileTime->LowPart = ft.dwLowDateTime;
        return TRUE;
    }
    return FALSE;
}

int WinGetSystemErrorMessageEx(ULONG ErrorCode,PWSTR *ppMessage,ULONG dwLanguageId)
{
    HMODULE hModule = NULL;
    DWORD f = 0;

    if( (ErrorCode & 0xC0000000) == 0xC0000000 )
    {
        hModule = GetModuleHandle(L"ntdll.dll");
        f = FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_IGNORE_INSERTS;
    }

    PWSTR pMessageBuf;
    DWORD cch;
    cch = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                f,
                hModule,
                ErrorCode,
                dwLanguageId,
                (LPWSTR)&pMessageBuf,
                0,
                NULL
                );

    if( cch != 0 )
    {
        *ppMessage = (PWSTR)pMessageBuf;
    }

    return cch;
}

int WinGetErrorMessage(ULONG ErrorCode,PWSTR *ppMessage)
{
    return WinGetSystemErrorMessageEx(ErrorCode,ppMessage,GetThreadLocale());
}

int WinGetSystemErrorMessage(ULONG ErrorCode,PWSTR *ppMessage)
{
    return WinGetSystemErrorMessageEx(ErrorCode,ppMessage,MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT));
}

void WinFreeErrorMessage(PWSTR pMessage)
{
    if( pMessage )
        LocalFree(pMessage);
}
