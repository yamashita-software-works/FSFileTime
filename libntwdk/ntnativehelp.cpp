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

#define PRIVATE static

EXTERN_C
void
NTAPI
_SetLastStatusDos(
    NTSTATUS ntStatus
    )
{
    RtlSetLastWin32Error( RtlNtStatusToDosError(ntStatus) );
}

EXTERN_C
void
NTAPI
_SetLastWin32Error(
    ULONG Win32ErrorCode
    )
{
    RtlSetLastWin32Error( Win32ErrorCode );
}

EXTERN_C
void
NTAPI
_SetLastNtStatus(
    NTSTATUS ntStatus
    )
{
    RtlSetLastWin32Error( ntStatus );
}

HANDLE _GetProcessHeap()
{
    HANDLE HeapHandle;
    RtlGetProcessHeaps(1,&HeapHandle);
    return HeapHandle;
}

EXTERN_C PVOID NTAPI AllocMemory(SIZE_T cb)
{
    return RtlAllocateHeap(_GetProcessHeap(),HEAP_ZERO_MEMORY,cb);
}

EXTERN_C PVOID NTAPI ReAllocateHeap(PVOID pv,SIZE_T cb)
{
    return RtlReAllocateHeap(_GetProcessHeap(),0,pv,cb);
}

EXTERN_C WCHAR* NTAPI AllocStringBuffer(SIZE_T cch)
{
    return (WCHAR *)RtlAllocateHeap(_GetProcessHeap(),HEAP_ZERO_MEMORY,cch*sizeof(WCHAR));
}

EXTERN_C WCHAR* NTAPI AllocStringBufferCb(SIZE_T cb)
{
    return (WCHAR *)RtlAllocateHeap(_GetProcessHeap(),HEAP_ZERO_MEMORY,cb);
}

EXTERN_C VOID NTAPI FreeMemory(PVOID ptr)
{
    RtlFreeHeap(_GetProcessHeap(),0,ptr);
}

EXTERN_C NTSTATUS NTAPI AllocateUnicodeString(UNICODE_STRING *pus,PCWSTR psz)
{
    UNICODE_STRING us;
    RtlInitUnicodeString(&us,psz);
    return RtlDuplicateUnicodeString(
        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE|RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
        &us,pus);
}

EXTERN_C NTSTATUS NTAPI DuplicateUnicodeString(UNICODE_STRING *pusDup,UNICODE_STRING *pusSrc)
{
    return RtlDuplicateUnicodeString(
        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE|RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
        pusSrc,pusDup);
}

EXTERN_C NTSTATUS NTAPI AllocateUnicodeStringCbBuffer(UNICODE_STRING *pus,ULONG cb)
{
    pus->Length = 0;
    pus->MaximumLength = (USHORT)cb;
    pus->Buffer = (PWSTR)AllocMemory( cb );
    if( pus->Buffer == NULL )
    {
        pus->Length = pus->MaximumLength = 0;
        return STATUS_NO_MEMORY;
    }
    return STATUS_SUCCESS;
}

EXTERN_C NTSTATUS NTAPI AllocateUnicodeStringCchBuffer(UNICODE_STRING *pus,ULONG cch)
{
    return AllocateUnicodeStringCbBuffer(pus,WCHAR_BYTES(cch));
}

EXTERN_C PWSTR NTAPI AllocateSzFromUnicodeString(UNICODE_STRING *pus)
{
    PWSTR psz;
    psz = (PWSTR)AllocMemory( pus->Length + sizeof(WCHAR) );
    if( psz )
    {
        RtlCopyMemory(psz,pus->Buffer,pus->Length);
    }
    return psz;
}

EXTERN_C PWSTR NTAPI AllocStringLengthCb(PCWSTR psz,SIZE_T cb)
{
    PWSTR pszDup = AllocStringBuffer( (ULONG)(cb) + sizeof(WCHAR) );
    RtlCopyMemory(pszDup,psz,cb);
    return pszDup;
}

EXTERN_C PWSTR NTAPI DuplicateString(PCWSTR psz)
{
    SIZE_T cch = wcslen(psz);
    PWSTR pszDup = AllocStringBuffer( (ULONG)(cch + 1) );
    RtlCopyMemory(pszDup,psz,cch * sizeof(WCHAR) );
    return pszDup;
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
#if 0
    return (_wcsnicmp(pszPath,L"\\Device\\",8) == 0) ||  
           (_wcsnicmp(pszPath,L"\\??\\",4) == 0);
#else
    UNICODE_STRING usPath;
    UNICODE_STRING usGlobalPrefix = {8,8,L"\\??\\"};
    UNICODE_STRING usDevicePrefix = {16,16,L"\\Device\\"};

    RtlInitUnicodeString(&usPath,pszPath);

    return (RtlPrefixUnicodeString(&usGlobalPrefix,&usPath,TRUE) ||
            RtlPrefixUnicodeString(&usDevicePrefix,&usPath,TRUE));
#endif
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

NTSTATUS FindRootDirectory_U(UNICODE_STRING *pusFullyQualifiedPath,PWSTR *pRootDirectory)
{
    if( pusFullyQualifiedPath == NULL || pusFullyQualifiedPath->Buffer == NULL || pusFullyQualifiedPath->Length == 0 )
        return STATUS_INVALID_PARAMETER;

    int ParseStartPos = 0;

    UNICODE_STRING usPrefix;

    RtlInitUnicodeString(&usPrefix,L"\\??\\");

    if( RtlPrefixUnicodeString(&usPrefix,pusFullyQualifiedPath,TRUE) )
    {
        //
        // Find root directory from global root  prefixed.
        // "\??\C:\", "\??\HarddiskVolume1\", "\??\Volume{xxx...xxx}\"
        //
        ParseStartPos = 4;
    }
    else
    {
        RtlInitUnicodeString(&usPrefix,L"\\Device\\");

        if( RtlPrefixUnicodeString(&usPrefix,pusFullyQualifiedPath,TRUE) )
        {
            //
            // Find root directory from NT device path.
            // "\Device\HarddiskVolume1\", "\Device\CdRom1\"
            //
            ParseStartPos = 8;
        }
        else
        {
            //
            // Find root directory from DOS drive path.
            // "C:\"
            //
            if( (WCHAR_LENGTH(pusFullyQualifiedPath->Length) > 3)
                && (pusFullyQualifiedPath->Buffer[1] == L':')
                && ((L'a' <= pusFullyQualifiedPath->Buffer[0] && pusFullyQualifiedPath->Buffer[0] <= L'z')||
                    (L'A' <= pusFullyQualifiedPath->Buffer[0] && pusFullyQualifiedPath->Buffer[0] <= L'Z')) )
            {
                ParseStartPos = 2;
            }
        }
    }

    if( ParseStartPos != 0 )
    {
        //
        //  0123(Skip=4)
        //  |--|
        //  \??\C:\
        //      |-|
        //       root
        //  \??\HarddiskVolume1\
        //      |--------------|
        //                    root
        //  01234567(Skip=8)
        //  |------|
        //  \Device\HarddiskVolume1\
        //          |--------------|
        //                        root
        //  01(Len=2,no skip part)
        //  ||
        //  C:\
        //    |
        //    root
        //
        // Invalid format.
        // "\Device\", "\Device", "\Device\\", "\Device\\HarddiskVolume"
        // "\??\\", 
        //
        // Not root directory, this is device name.
        // "\Device\HarddiskVolume1", "\Device\CdRom1" , "\??\C:" , "C:"
        //

        int cch = (WCHAR_LENGTH(pusFullyQualifiedPath->Length)-ParseStartPos);
        if( cch <= 1 )
            return STATUS_OBJECT_PATH_INVALID;

        WCHAR *p = &pusFullyQualifiedPath->Buffer[ParseStartPos];

        for(int i = 0; i < cch; i++)
        {
            if( *p == L'\\' )
            {
                *pRootDirectory = p;
                return STATUS_SUCCESS; // root found
            }
            p++;
        }
        *pRootDirectory = NULL;
        return STATUS_OBJECT_PATH_NOT_FOUND; // root not found
    }

    *pRootDirectory = NULL;
    return STATUS_OBJECT_PATH_SYNTAX_BAD; // invalid path
}

BOOLEAN GetRootDirectory_U(UNICODE_STRING *pusFullyQualifiedPath)
{
    if( pusFullyQualifiedPath == NULL || pusFullyQualifiedPath->Buffer == NULL || pusFullyQualifiedPath->Length == 0 )
        return FALSE;

    PWSTR pRootDirectory = NULL;
    if( FindRootDirectory_U(pusFullyQualifiedPath,&pRootDirectory) == STATUS_SUCCESS )
    {
        pusFullyQualifiedPath->Length = (USHORT)((pRootDirectory - pusFullyQualifiedPath->Buffer) + 1) * sizeof(WCHAR);
        return TRUE;
    }

    return FALSE;
}

BOOLEAN GetVolumeName_U(UNICODE_STRING *pusFullyQualifiedPath)
{
    if( pusFullyQualifiedPath == NULL || pusFullyQualifiedPath->Buffer == NULL || pusFullyQualifiedPath->Length == 0 )
        return FALSE;

    PWSTR pRootDirectory = NULL;
    NTSTATUS Status = FindRootDirectory_U(pusFullyQualifiedPath,&pRootDirectory);

    if( Status == STATUS_SUCCESS )
    {
        pusFullyQualifiedPath->Length = (USHORT)((pRootDirectory - pusFullyQualifiedPath->Buffer) * sizeof(WCHAR));
        return TRUE;
    }
    else if( Status == STATUS_OBJECT_PATH_NOT_FOUND )
    {
        // The root directory not found. Specified path is volume name.
        return TRUE;
    }
    return FALSE;
}

BOOLEAN PathFileExists_U(UNICODE_STRING *pusPath,ULONG *FileAttributes)
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

BOOLEAN PathFileExists_UEx(HANDLE hParentDir,UNICODE_STRING *pusPath,ULONG *FileAttributes)
{
    OBJECT_ATTRIBUTES oa = {0};
    FILE_NETWORK_OPEN_INFORMATION fi = {0};

    InitializeObjectAttributes(&oa,pusPath,0,hParentDir,0);

    if( NtQueryFullAttributesFile(&oa,&fi) == STATUS_SUCCESS )
    {
        if( FileAttributes )
            *FileAttributes = fi.FileAttributes;
        return TRUE;
    }
    return FALSE;
}

BOOLEAN PathFileExists_W(PCWSTR pszPath,ULONG *FileAttributes)
{
    UNICODE_STRING usPath;
    RtlInitUnicodeString(&usPath,pszPath);
    return PathFileExists_U(&usPath,FileAttributes);
}

PRIVATE VOID GetCurrentDirectory()
{
    WCHAR PathBuffer[WIN32_MAX_PATH];
    RtlGetCurrentDirectory_U(PATH_BUFFER_LENGTH,PathBuffer);

    UNICODE_STRING NtPathName;
    if( RtlDosPathNameToNtPathName_U(PathBuffer,&NtPathName,NULL,NULL) )
    {
        RtlFreeUnicodeString(&NtPathName);
    }
}

PRIVATE PWSTR MakeFullPath(PCWSTR pszPath)
{
    WCHAR szFullPath[WIN32_MAX_PATH];
    RtlGetFullPathName_U(pszPath,ARRAYSIZE(szFullPath),szFullPath,NULL);
    return _wcsdup(szFullPath);
}

PWSTR DosPathNameToNtPathName(PCWSTR pszDosPath)
{
    PWSTR pszNtPath = NULL;
    UNICODE_STRING NtPathName;
    if( RtlDosPathNameToNtPathName_U(pszDosPath,&NtPathName,NULL,NULL) )
    {
        pszNtPath = AllocateSzFromUnicodeString(&NtPathName);
        RtlFreeUnicodeString(&NtPathName);
    }
    return pszNtPath;
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

BOOLEAN SplitRootRelativePath(PCWSTR pszFullPath,UNICODE_STRING *RootDirectory,UNICODE_STRING *RootRelativePath)
{
    UNICODE_STRING usRootDirectory;

    RtlInitUnicodeString(&usRootDirectory,pszFullPath);

    if( !GetRootDirectory_U(&usRootDirectory) )
    {
        return FALSE;
    }

    *RootDirectory = usRootDirectory;

    int cb = WCHAR_BYTES( (int)wcslen(pszFullPath) );
    if( ((cb - usRootDirectory.Length) >= 65536) || ((cb - usRootDirectory.MaximumLength) >= 65536) )
        return FALSE;

    RootRelativePath->Length        = (USHORT)(cb - usRootDirectory.Length);
    RootRelativePath->MaximumLength = (USHORT)(cb - usRootDirectory.MaximumLength);
    RootRelativePath->Buffer        = (PWCH)&pszFullPath[ WCHAR_LENGTH(usRootDirectory.Length) ];

#ifdef _DEBUG
    UNICODE_STRING us1,us2;
    RtlDuplicateUnicodeString(0x3,RootDirectory,&us1);
    RtlDuplicateUnicodeString(0x3,RootRelativePath,&us2);
    RtlFreeUnicodeString(&us1);
    RtlFreeUnicodeString(&us2);
#endif

    return TRUE;
}

BOOLEAN IsLastCharacterBackslash_U(UNICODE_STRING *pusPath)
{
    if( pusPath->Length < sizeof(WCHAR) || pusPath->MaximumLength == 0 )
        return FALSE;

    return ( pusPath->Buffer[WCHAR_LENGTH(pusPath->Length)-1] == L'\\' );
}

BOOLEAN IsLastCharacterBackslash(PCWSTR pszPath)
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

VOID RemoveFileSpec(PWSTR pszPath)
{
    UNICODE_STRING us;

    RtlInitUnicodeString(&us,pszPath);
    if( SplitPathFileName_U(&us,NULL) == STATUS_SUCCESS )
    {
        if( !IsRootDirectory_U(&us) )
            RemoveBackslash_U(&us);
        pszPath[ us.Length / sizeof(WCHAR) ] = UNICODE_NULL;
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

    cch += cchPath + wcslen(pszFileName) + 1;

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

EXTERN_C
NTSTATUS
NTAPI
StringFromGUID(
    __in  const GUID *Guid,
    __out LPWSTR lpszGuid,
    __in  int cchMax
    )
{
    UNICODE_STRING GuidString;
    NTSTATUS Status;

    Status = RtlStringFromGUID( *Guid, &GuidString );

    if( Status == STATUS_SUCCESS )
    {
        // cchGuid is must include terminate null.
        if( cchMax <= (int)(GuidString.Length/sizeof(WCHAR)) )
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            memcpy(lpszGuid,GuidString.Buffer,GuidString.Length);
            lpszGuid[ WCHAR_LENGTH(GuidString.Length) ] = L'\0';
        }
        RtlFreeUnicodeString(&GuidString);
    }
    
    Status = RtlNtStatusToDosError(Status);

    return Status;
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
