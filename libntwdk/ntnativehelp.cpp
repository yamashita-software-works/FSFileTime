//***************************************************************************
//*                                                                         *
//*  ntnativehelp.cpp                                                       *
//*                                                                         *
//*  Create: 2021-04-12                                                     *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.                *
//*  Licensed under the MIT License.                                        *
//*                                                                         *
//***************************************************************************
#include <ntifs.h>
#include <stdio.h>
#include <malloc.h>
#include <strsafe.h>
#include <locale.h>
#include <conio.h>
#include <winerror.h>
#include "ntnativeapi.h"
#include "ntnativehelp.h"

HANDLE _GetProcessHeap()
{
    HANDLE HeapHandle;
    RtlGetProcessHeaps(1,&HeapHandle);
    return HeapHandle;
}

PVOID AllocMemory(SIZE_T cb)
{
    return RtlAllocateHeap(_GetProcessHeap(),HEAP_ZERO_MEMORY,cb);
}

PVOID ReAllocateHeap(PVOID pv,SIZE_T cb)
{
    return RtlReAllocateHeap(_GetProcessHeap(),0,pv,cb);
}

WCHAR *AllocStringBuffer(ULONG cch)
{
    return (WCHAR *)RtlAllocateHeap(_GetProcessHeap(),HEAP_ZERO_MEMORY,cch*sizeof(WCHAR));
}

VOID FreeMemory(PVOID ptr)
{
    RtlFreeHeap(_GetProcessHeap(),0,ptr);
}

NTSTATUS AllocateUnicodeString(UNICODE_STRING *pus,PCWSTR psz)
{
    UNICODE_STRING us;
    RtlInitUnicodeString(&us,psz);
    return RtlDuplicateUnicodeString(
        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE|RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
        &us,pus);
}

NTSTATUS DuplicateUnicodeString(UNICODE_STRING *pusDup,UNICODE_STRING *pusSrc)
{
    return RtlDuplicateUnicodeString(
        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE|RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
        pusSrc,pusDup);
}

PWSTR AllocateSzFromUnicodeString(UNICODE_STRING *pus)
{
    PWSTR psz;
    psz = (PWSTR)AllocMemory( pus->Length + sizeof(WCHAR) );
    if( psz )
    {
        RtlCopyMemory(psz,pus->Buffer,pus->Length);
    }
    return psz;
}

//
// "aaabbbccc"
//  |       |
//  str     end
//
BOOLEAN _UStrMatchI(const WCHAR *ptn,const WCHAR *str,const WCHAR *end)
{
    switch( *ptn )
    {
        case L'\0':
            return (L'\0' == *str) || (str > end);
        case L'*':
            return _UStrMatchI( ptn+1, str, end ) || ((L'\0' != *str) && (str <= end) && _UStrMatchI( ptn, str+1, end ));
        case L'?':
            return (L'\0' != *str) && (str <= end) && _UStrMatchI( ptn+1, str+1, end );
        default:
            return (RtlUpcaseUnicodeChar(*ptn) == RtlUpcaseUnicodeChar(*str)) && (((L'\0' != *str) && (str <= end)) && _UStrMatchI( ptn+1, str+1, end ));
    }
}

BOOLEAN _UStrMatch(const WCHAR *ptn,const WCHAR *str,const WCHAR *end)
{
    switch( *ptn )
    {
        case L'\0':
            return (L'\0' == *str) || (str > end);
        case L'*':
            return _UStrMatch( ptn+1, str, end ) || ((L'\0' != *str) && (str <= end) && _UStrMatch( ptn, str+1, end ));
        case L'?':
            return (L'\0' != *str) && (str <= end) && _UStrMatch( ptn+1, str+1, end );
        default:
            return (*ptn == *str) && (((L'\0' != *str) && (str <= end)) && _UStrMatch( ptn+1, str+1, end ));
    }
}

BOOLEAN _UStrMatch_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end)
{
    switch( *ptn )
    {
        case L'\0':
            return (str > end);
        case L'*':
            return _UStrMatch_UStr( ptn+1, str, end ) || ((str <= end) && _UStrMatch_UStr( ptn, str+1, end ));
        case L'?':
            return (str <= end) && _UStrMatch_UStr( ptn+1, str+1, end );
        default:
            return (*ptn == *str) && ((str <= end) && _UStrMatch_UStr( ptn+1, str+1, end ));
    }
}

BOOLEAN _UStrMatchI_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end)
{
    switch( *ptn )
    {
        case L'\0':
            return (str > end);
        case L'*':
            return _UStrMatchI_UStr( ptn+1, str, end ) || ((str <= end) && _UStrMatchI_UStr( ptn, str+1, end ));
        case L'?':
            return (str <= end) && _UStrMatchI_UStr( ptn+1, str+1, end );
        default:
            return (RtlUpcaseUnicodeChar(*ptn) == RtlUpcaseUnicodeChar(*str)) && ((str <= end) && _UStrMatchI_UStr( ptn+1, str+1, end ));
    }
}

BOOLEAN _UStrMatch_U(const WCHAR *ptn,const UNICODE_STRING *pus)
{
    if( pus->Length == 0 || pus->Buffer == NULL )
        return FALSE;
    return _UStrMatch_UStr(ptn,pus->Buffer,&pus->Buffer[ (pus->Length/sizeof(WCHAR))-1 ]);
}

BOOLEAN _UStrMatchI_U(const WCHAR *ptn,const UNICODE_STRING *pus)
{
    if( pus->Length == 0 || pus->Buffer == NULL )
        return FALSE;
    return _UStrMatchI_UStr(ptn,pus->Buffer,&pus->Buffer[ (pus->Length/sizeof(WCHAR))-1 ]);
}

//
// C-Sz compare functions
//
BOOLEAN _UStrCmpI(const WCHAR *str1,const WCHAR *str2)
{
    while( *str1 && *str2 )
    {
        if( towupper( *str1 ) != towupper( *str2 ) )
        {
            return FALSE;
        }
        str1++;
        str2++;
    }
    return TRUE;
}

BOOLEAN _UStrCmp(const WCHAR *str1,const WCHAR *str2)
{
    while( *str1 && *str2 )
    {
        if( *str1 != *str2 )
        {
            return FALSE;
        }
        str1++;
        str2++;
    }
    return TRUE;
}

BOOLEAN IsNtDevicePath(PCWSTR pszPath)
{
    // In following case, it is determined to be the NT device path. 
    // "\Device\"
    // "\??\" 
    return (_wcsnicmp(pszPath,L"\\Device\\",8) == 0) ||  
           (_wcsnicmp(pszPath,L"\\??\\",4) == 0);
/*++
BOOLEAN 
RtlPrefixUnicodeString( 
    IN PUNICODE_STRING  String1, 
    IN PUNICODE_STRING  String2, 
    IN BOOLEAN  CaseInSensitive 
    );
--*/
}

BOOLEAN HasPrefix(PCWSTR pszPrefix,PCWSTR pszPath)
{
    UNICODE_STRING String1;
    UNICODE_STRING String2;
    RtlInitUnicodeString(&String1,pszPrefix);
    RtlInitUnicodeString(&String2,pszPath);
    return RtlPrefixUnicodeString(&String1,&String2,TRUE);
}

BOOLEAN HasPrefix_U(PCWSTR pszPrefix,UNICODE_STRING *String)
{
    UNICODE_STRING Prefix;
    RtlInitUnicodeString(&Prefix,pszPrefix);
    return RtlPrefixUnicodeString(&Prefix,String,TRUE);
}

BOOLEAN HasWildCardChar_U(UNICODE_STRING *String)
{
    int i;
    for(i = 0; i < (int)(String->Length/sizeof(WCHAR)); i++)
    {
        if( String->Buffer[i] == L'*' || String->Buffer[i] == L'?' )
            return TRUE;
    }
    return FALSE;
}

/*++
  "C:\foo"   RtlPathTypeDriveAbsolute
  "foo"      RtlPathTypeRelative
  "C:"       RtlPathTypeDriveRelative
  "C:foo"    RtlPathTypeDriveRelative
  "C:\\"     RtlPathTypeDriveAbsolute
  "\\?\C:\"  RtlPathTypeLocalDevice
  "\\.\C:\"  RtlPathTypeLocalDevice
  "\\ServerName\" RtlPathTypeUncAbsolute
  "\Device\HarddiskVolume1\" RtlPathTypeRooted
  "\\?\GLOBALROOT\Device\Harddiskvolume1" RtlPathTypeLocalDevice
--*/
BOOLEAN IsRelativePath(PCWSTR pszPath)
{
    RTL_PATH_TYPE Type = RtlDetermineDosPathNameType_U( pszPath );
    return (Type == RtlPathTypeRelative)||(Type == RtlPathTypeDriveRelative);
}

BOOLEAN IsLocalDevicePath(PCWSTR pszPath)
{
    RTL_PATH_TYPE Type = RtlDetermineDosPathNameType_U( pszPath );
    return (Type == RtlPathTypeLocalDevice);
}

BOOLEAN IsDirectory(PCWSTR pszPath)
{
    OBJECT_ATTRIBUTES oa = {0};
    FILE_NETWORK_OPEN_INFORMATION fi = {0};

    UNICODE_STRING name;
    RtlInitUnicodeString(&name,pszPath);

    InitializeObjectAttributes(&oa,&name,0,0,0);

    if( NtQueryFullAttributesFile(&oa,&fi) == STATUS_SUCCESS )
    {
        return ((fi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }

    return FALSE;
}

BOOLEAN IsDirectory_U(UNICODE_STRING *pusPath)
{
    OBJECT_ATTRIBUTES oa = {0};
    FILE_NETWORK_OPEN_INFORMATION fi = {0};

    InitializeObjectAttributes(&oa,pusPath,0,0,0);

    if( NtQueryFullAttributesFile(&oa,&fi) == STATUS_SUCCESS )
    {
        return ((fi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }

    return FALSE;
}

BOOLEAN IsRootDirectory_U(UNICODE_STRING *pusFullyPath)
{
    if( pusFullyPath == NULL || pusFullyPath->Buffer == NULL || pusFullyPath->Length == 0 )
        return FALSE;

    int ReparsePoint = 0;

    UNICODE_STRING usPrefix;

    RtlInitUnicodeString(&usPrefix,L"\\??\\");

    if( RtlPrefixUnicodeString(&usPrefix,pusFullyPath,TRUE) )
    {
        //
        // Root directory.
        // "\??\C:\", "\??\HarddiskVolume1\", "\??\Volume{xxx...xxx}\"
        //
        // Invalid format.
        // "\??\", "\??\\",
        //
        // Not root directory, include sub-directroy.
        // "\??\C:\Foo\\", "\??\HarddiskVolume1\Foo\Bar"
        //
        // Not root directory, this is device name.
        // "\??\C:", "\??\HarddiskVolume1"
        //
        ReparsePoint = 4;
    }
    else
    {
        RtlInitUnicodeString(&usPrefix,L"\\Device\\");

        if( RtlPrefixUnicodeString(&usPrefix,pusFullyPath,TRUE) )
        {
            //
            // Root directory.
            // "\Device\HarddiskVolume1\", "\Device\CdRom1\"
            //
            // Invalid format.
            // "\Device\", "\Device", "\Device\\", "\Device\\HarddiskVolume"
            //
            // Not root directory, include sub-directroy.
            // "\Device\HarddiskVolume1\\foo\\", "\Device\CdRom1\\foo\bar"
            //
            // Not root directory, this is device name.
            // "\Device\HarddiskVolume1", "\Device\CdRom1"
            //
            ReparsePoint = 8;
        }
    }

    if( ReparsePoint != 0 )
    {
        WCHAR *p = &pusFullyPath->Buffer[ReparsePoint];
        int cch = (WCHAR_LENGTH(pusFullyPath->Length)-ReparsePoint);
        if( cch <= 1 )
            return FALSE;

        for(int i = 0; i < cch; i++)
        {
            if( *p == L'\\' )
            {
                ULONG_PTR cbRoot = (ULONG_PTR)(((p - pusFullyPath->Buffer) + 1) * sizeof(WCHAR));

                if( cbRoot == pusFullyPath->Length )
                {
                    return TRUE;
                }
                break;
            }
            p++;
        }
    }

    return FALSE;
}

BOOLEAN NtPathFileExists_U(UNICODE_STRING *pusPath,ULONG *FileAttributes)
{
    OBJECT_ATTRIBUTES oa = {0};
    FILE_NETWORK_OPEN_INFORMATION fi = {0};

    InitializeObjectAttributes(&oa,pusPath,0,0,0);

    if( NtQueryFullAttributesFile(&oa,&fi) == STATUS_SUCCESS )
    {
        if( FileAttributes )
            *FileAttributes = fi.FileAttributes;
        return TRUE;
    }
    return FALSE;
}

void GetCurrentDirectory()
{
    WCHAR PathBuffer[WIN32_MAX_PATH];
    RtlGetCurrentDirectory_U(PATH_BUFFER_LENGTH,PathBuffer);

    UNICODE_STRING NtPathName;
    if( RtlDosPathNameToNtPathName_U(PathBuffer,&NtPathName,NULL,NULL) )
    {
        RtlFreeUnicodeString(&NtPathName);
    }
}

PWSTR MakeFullPath(PCWSTR pszPath)
{
    WCHAR szFullPath[WIN32_MAX_PATH];
    RtlGetFullPathName_U(pszPath,ARRAYSIZE(szFullPath),szFullPath,NULL);
    return _wcsdup(szFullPath);
}

NTSTATUS GetFileNamePart_U(UNICODE_STRING *FilePath,UNICODE_STRING *FileName)
{
    NTSTATUS Status;
    ULONG cch = 0;
    // "C:\foo\bar.txt", length of "C:\foo\"
    Status = RtlGetLengthWithoutLastFullDosOrNtPathElement(0,FilePath,&cch);

    if( Status == STATUS_SUCCESS )
    {
        FileName->Length = FileName->MaximumLength = FilePath->Length - (USHORT)(cch * sizeof(WCHAR));
        FileName->Buffer = &FilePath->Buffer[cch];
    }

    RtlSetLastWin32Error( Status );

    return Status;
}

NTSTATUS SplitPathFileName_U(UNICODE_STRING *Path,UNICODE_STRING *FileName)
{
    NTSTATUS Status;
    ULONG cch = 0;

    // "C:\foo\bar.txt", length of "C:\foo\"
    Status = RtlGetLengthWithoutLastFullDosOrNtPathElement(0,Path,&cch);

    if( Status == STATUS_SUCCESS )
    {
        if( FileName )
        {
            FileName->Length = FileName->MaximumLength = Path->Length - (USHORT)(cch * sizeof(WCHAR));
            FileName->Buffer = &Path->Buffer[cch];
        }

        // truncate filename from path
        Path->Length = (USHORT)(cch * sizeof(WCHAR));
    }

    RtlSetLastWin32Error( Status );

    return Status;
}

BOOLEAN IsLastCharacterBackslash_U(UNICODE_STRING *pusPath)
{
    if( pusPath->Length < sizeof(WCHAR) || pusPath->MaximumLength == 0 )
        return FALSE;

    return ( pusPath->Buffer[WCHAR_LENGTH(pusPath->Length)-1] == L'\\' );
}

BOOLEAN IsLastCharacterBackslash(PWSTR pszPath)
{
    int cch = (int)wcslen(pszPath);
    if( cch > 0 )
    {
        return ( pszPath[cch-1] == L'\\' );
    }
    return FALSE;
}

VOID RemoveBackslash_U(UNICODE_STRING *pusPath)
{
    if( IsLastCharacterBackslash_U(pusPath) )
    {
        pusPath->Length -= sizeof(WCHAR);
    }
}

VOID RemoveBackslash(PWSTR pszPath)
{
    int cch = (int)wcslen(pszPath);
    if( cch > 0 )
    {
        if( pszPath[cch-1] == L'\\' )
            pszPath[cch-1] = L'\0';
    }
}

PWSTR CombinePath(PCWSTR pszPath,PCWSTR pszFileName)
{
    WCHAR *psz;
    SIZE_T cch,cchPath;

    cch = 0;

    cchPath = wcslen(pszPath);
    if( cchPath > 0 )
    {
        if( pszPath[cchPath-1] != L'\\' )
        {
            cch++;
        }
    }

    cch = cchPath + wcslen(pszFileName) + 1;

    psz = AllocStringBuffer((ULONG)cch);

    if( psz )
    {
        StringCchCopy(psz,cch,pszPath);
        if( pszPath[cchPath-1] != L'\\' )
            StringCchCat(psz,cch,L"\\");
        StringCchCat(psz,cch,pszFileName);
    }

    return psz;
}

PWSTR CombinePath_U(PCWSTR pszPath,UNICODE_STRING *pusFileName)
{
    WCHAR *psz;
    SIZE_T cch,cchPath;

    cch = 0;

    cchPath = wcslen(pszPath);
    if( cchPath > 0 )
    {
        if( pszPath[cchPath-1] != L'\\' )
        {
            cch++;
        }
    }

    cch += cchPath + (pusFileName->Length/sizeof(WCHAR)) + 1;

    psz = AllocStringBuffer((ULONG)cch);

    if( psz )
    {
        StringCchCopy(psz,cch,pszPath);
        if( pszPath[cchPath-1] != L'\\' )
            psz[cchPath++] = L'\\';
        memcpy(&psz[cchPath],pusFileName->Buffer,pusFileName->Length);
    }

    return psz;
}

NTSTATUS CombineUnicodeStringPath(UNICODE_STRING *CombinedPath,UNICODE_STRING *Path,UNICODE_STRING *FileName)
{
    if( CombinedPath == NULL || Path == NULL || FileName == NULL )
        return STATUS_INVALID_PARAMETER;

    if( Path->Buffer == NULL || (Path->Length == 0 && Path->MaximumLength == 0) )
        return STATUS_INVALID_PARAMETER;

    if( FileName->Buffer == NULL || (FileName->Length == 0 && FileName->MaximumLength == 0) )
        return STATUS_INVALID_PARAMETER;

    ULONG cbAlloc = 0;

    CombinedPath->Length = CombinedPath->MaximumLength = 0;
    CombinedPath->Buffer = NULL;

    cbAlloc += Path->Length;
    cbAlloc += FileName->Length;

    BOOLEAN addBackslash = (!IsLastCharacterBackslash_U(Path) && (FileName->Length > 0 && FileName->Buffer[0] != L'\\'));

    if( addBackslash )
    {
        cbAlloc += sizeof(WCHAR);
    }

    cbAlloc += sizeof(WCHAR); // for C string terminate null.

    CombinedPath->Buffer = AllocStringBuffer( WCHAR_LENGTH(cbAlloc) );

    if( CombinedPath->Buffer )
    {
        CombinedPath->Length = Path->Length;
        CombinedPath->MaximumLength = (USHORT)cbAlloc;

        RtlCopyMemory(CombinedPath->Buffer,Path->Buffer,Path->Length);

        if( addBackslash )
        {
            CombinedPath->Buffer[ WCHAR_LENGTH(Path->Length) ] = L'\\';
            CombinedPath->Length += sizeof(WCHAR);
        }

        RtlAppendUnicodeStringToString(CombinedPath,FileName);
    }
    else
    {
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

//
// Internal Functions
//
PCHAR allocAnsiString(const WCHAR *ws)
{
    UNICODE_STRING us;
    RtlInitUnicodeString(&us,ws);
    ANSI_STRING as;
    RtlUnicodeStringToAnsiString(&as,&us,TRUE);
    return as.Buffer;
}

VOID freeAnsiString(CHAR *s)
{
    ANSI_STRING as;
    RtlInitAnsiString(&as,s);
    RtlFreeAnsiString(&as);
}
