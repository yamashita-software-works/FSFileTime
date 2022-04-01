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

//----------------------------------------------------------------------------
//
//  GetFileDateTime()
//
//----------------------------------------------------------------------------
NTSTATUS GetFileDateTime( UNICODE_STRING *FilePath, FILE_BASIC_INFORMATION *pbi )
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
