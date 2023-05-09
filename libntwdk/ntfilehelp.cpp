//***************************************************************************
//*                                                                         *
//*  ntfilehelp.cpp                                                         *
//*                                                                         *
//*  Create: 2022-03-29                                                     *
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

HRESULT GetNtPath(PCWSTR DosPathName,PWSTR *NtPath,PCWSTR *NtFileNamePart)
{
    UNICODE_STRING NtPathName = {0};
    if( RtlDosPathNameToNtPathName_U(DosPathName,&NtPathName,NtFileNamePart,NULL) )
    {
        *NtPath = NtPathName.Buffer;
        return S_OK;
    }
    return E_FAIL;
}

HRESULT GetNtPath_U(PCWSTR DosPathName,UNICODE_STRING *NtPath,PCWSTR *NtFileNamePart)
{
    if( RtlDosPathNameToNtPathName_U(DosPathName,NtPath,NtFileNamePart,NULL) )
    {
        return S_OK;
    }
    return E_FAIL;
}

//----------------------------------------------------------------------------
//
//  OpenFile_W()
//
//----------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
OpenFile_W(
    PHANDLE phFile,
    HANDLE hRoot,
    PCWSTR PathName,
    ULONG DesiredAccess,
    ULONG ShareAccess,
    ULONG OpenOptions
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus = {0};
    NTSTATUS Status;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    UNICODE_STRING usPath;

    RtlInitUnicodeString(&usPath,PathName);

    InitializeObjectAttributes(&ObjectAttributes,&usPath,0,hRoot,NULL);

    Status = NtOpenFile(&hFile,
                    DesiredAccess,
                    &ObjectAttributes,
                    &IoStatus,
                    ShareAccess,
                    OpenOptions);

    if( Status != STATUS_SUCCESS )
    {
        ;//todo:
    }

    *phFile = hFile;

    return Status;
}

//----------------------------------------------------------------------------
//
//  OpenFile_U()
//
//----------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
OpenFile_U(
    PHANDLE phFile,
    HANDLE hRoot,
    UNICODE_STRING *PathName,
    ULONG DesiredAccess,
    ULONG ShareAccess,
    ULONG OpenOptions
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus = {0};
    NTSTATUS Status;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    InitializeObjectAttributes(&ObjectAttributes,PathName,0,hRoot,NULL);

    Status = NtOpenFile(&hFile,
                    DesiredAccess,
                    &ObjectAttributes,
                    &IoStatus,
                    ShareAccess,
                    OpenOptions);

    if( Status != STATUS_SUCCESS )
    {
        ;//todo:
    }

    *phFile = hFile;

    return Status;
}

//----------------------------------------------------------------------------
//
//  CreateFile_U()
//
//----------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
CreateFile_U(
    PHANDLE FileHandle,
    HANDLE hRoot,
    UNICODE_STRING *NtFilePath,
    PVOID SecurityDescriptor,
    PLARGE_INTEGER AllocationSize,
    ULONG DesiredAccess,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes,NtFilePath,0,hRoot,SecurityDescriptor);

    Status = NtCreateFile(FileHandle,
                        DesiredAccess,
                        &ObjectAttributes,
                        &IoStatus,
                        AllocationSize,
                        FileAttributes,
                        ShareAccess,
                        CreateDisposition,
                        CreateOptions,
                        EaBuffer,
                        EaLength);

    if( Status != STATUS_SUCCESS )
    {
        ;//todo:
    }
    return Status;
}

//----------------------------------------------------------------------------
//
//  CreateDirectory_W()
//
//----------------------------------------------------------------------------
EXTERN_C
LONG
NTAPI
CreateDirectory_W(
    HANDLE hRoot,
    LPCWSTR NewDirectory,
    SECURITY_ATTRIBUTES *SecurityAttributes
    )
{
    HANDLE hDir = NULL;
    IO_STATUS_BLOCK IoStatus = {0};
    UNICODE_STRING NtPathName = {0,0,NULL};
    NTSTATUS Status;

    RtlInitUnicodeString( &NtPathName, NewDirectory );

    Status = CreateFile_U(
                &hDir,
                hRoot,
                &NtPathName,
                SecurityAttributes != NULL ? SecurityAttributes->lpSecurityDescriptor : NULL,
                NULL,
                SYNCHRONIZE|FILE_LIST_DIRECTORY,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_CREATE,
                FILE_OPEN_REPARSE_POINT|FILE_OPEN_FOR_BACKUP_INTENT|
                FILE_SYNCHRONOUS_IO_NONALERT|FILE_DIRECTORY_FILE,
                NULL,0);

    if( hDir != NULL && hDir != INVALID_HANDLE_VALUE )
        NtClose(hDir);

    return Status;
}

//----------------------------------------------------------------------------
//
//  GetFileDateTime()
//
//----------------------------------------------------------------------------
NTSTATUS GetFileDateTime_U( UNICODE_STRING *FilePath, FILE_BASIC_INFORMATION *pbi )
{
    if( FilePath == NULL || pbi == NULL )
        return STATUS_INVALID_PARAMETER;

    OBJECT_ATTRIBUTES ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes,FilePath,0,NULL,NULL);

    HANDLE Handle;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus = {0};

    Status = NtOpenFile(&Handle,FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES,
                    &ObjectAttributes,&IoStatus,FILE_SHARE_READ|FILE_SHARE_WRITE,0);

    if( Status == STATUS_SUCCESS )
    {
        FILE_BASIC_INFORMATION fbi = {0};

        Status = NtQueryInformationFile(Handle,&IoStatus,&fbi,sizeof(fbi),FileBasicInformation);

        if( Status == STATUS_SUCCESS )
        {
            *pbi = fbi;
        }

        NtClose(Handle);
    }

    return Status;
}

//----------------------------------------------------------------------------
//
//  MakeSureDirectoryPathExistsW()
//
//----------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
MakeSureDirectoryPathExistsW(
    PCWSTR pszFullPath
    )
{
    UNICODE_STRING RootDirectory;
    UNICODE_STRING RootRelativePath;
    NTSTATUS Status;

    if( !SplitRootRelativePath(pszFullPath,&RootDirectory,&RootRelativePath) )
    {
        return STATUS_INVALID_PARAMETER;
    }

    HANDLE hParentDir;
    Status = OpenFile_U(&hParentDir,NULL,&RootDirectory,FILE_GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_DIRECTORY_FILE);

    if( Status == STATUS_SUCCESS )
    {
        WCHAR seps[] = L"\\";
        WCHAR *token = NULL;
        WCHAR *next_token = NULL;
        HANDLE hCreatedDir;

        token = wcstok_s((PWSTR)RootRelativePath.Buffer,seps,&next_token);

        while( token != NULL )
        {
            UNICODE_STRING token_u;
            RtlInitUnicodeString(&token_u,token);

            if( !PathFileExists_UEx(hParentDir,&token_u,NULL) )
            {
                Status = CreateDirectory(hParentDir,token,NULL);
                if( Status != STATUS_SUCCESS && Status != STATUS_OBJECT_NAME_COLLISION )
                    break;
            }

            Status = OpenFile_U(&hCreatedDir,hParentDir,&token_u,
                        FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,
                        FILE_DIRECTORY_FILE);

            if( Status != STATUS_SUCCESS )
                break;

            NtClose(hParentDir);
            hParentDir = hCreatedDir;

            token = wcstok_s(NULL,seps,&next_token);
        }

        NtClose(hParentDir);
    }

    return Status;
}

//----------------------------------------------------------------------------
//
//  MoveDirectoryEntry()
//
//----------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
MoveDirectoryEntry(
    HANDLE hExistingDirectory,
    PCWSTR pszExistingFileName,
    HANDLE hDestinationDirectory,
    PCWSTR pszNewFileName,
    BOOLEAN ReplaceIfExists
    )
{
    NTSTATUS Status;
    HANDLE hSrcFile;
    UNICODE_STRING us;

    if( hExistingDirectory == NULL || pszExistingFileName == NULL || 
        hDestinationDirectory == NULL || pszNewFileName == NULL )
    {
        _SetLastStatusDos( STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    ULONG cbMoveBufferLength = sizeof(FILE_RENAME_INFORMATION) + WIN32_MAX_PATH_BYTES;
    FILE_RENAME_INFORMATION *pMoveBuffer = (FILE_RENAME_INFORMATION *)AllocMemory( cbMoveBufferLength );

    RtlInitUnicodeString(&us,pszExistingFileName);

    //
    // Target source file handle is must be source directory relative open.
    //
    Status = OpenFile_U(&hSrcFile,hExistingDirectory,
                    &us,
                    FILE_GENERIC_READ|FILE_GENERIC_WRITE|DELETE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    FILE_OPEN_FOR_BACKUP_INTENT);

    if( Status == STATUS_SUCCESS )
    {
        IO_STATUS_BLOCK IoStatus;

        pMoveBuffer->ReplaceIfExists = ReplaceIfExists;
        pMoveBuffer->RootDirectory = hDestinationDirectory;
        pMoveBuffer->FileNameLength = us.Length;
        RtlCopyMemory(pMoveBuffer->FileName,us.Buffer,us.Length);

        Status = NtSetInformationFile(hSrcFile,&IoStatus,pMoveBuffer,cbMoveBufferLength,FileRenameInformation);

        NtClose(hSrcFile);
    }

    _SetLastStatusDos( Status );

    return Status;
}
