//***************************************************************************
//*                                                                         *
//*  copydatetime.cpp                                                       *
//*                                                                         *
//*  Create: 2022-03-28                                                     *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.                *
//*  Licensed under the MIT License.                                        *
//*                                                                         *
//***************************************************************************
#include <ntifs.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <strsafe.h>
#include <locale.h>
#include <conio.h>
#include <winerror.h>
#include "fsfiletime.h"

typedef struct _COPY_CONTEXT_STRUCT
{
    CCommandRunParam *pcm;
    UNICODE_STRING DestinationPath;
} COPY_CONTEXT_STRUCT;

NTSTATUS
CopyDateTime(
    CCommandRunParam *pcm,
    UNICODE_STRING& SrcFilePath,
    UNICODE_STRING& DstFilePath
    )
{
    NTSTATUS Status;

    FILE_BASIC_INFORMATION biSrc = {0};
    FILE_BASIC_INFORMATION biDst = {0};

    Status = GetFileDateTime_U(NULL,&SrcFilePath,&biSrc);

    if( Status == STATUS_SUCCESS )
    {
        const WCHAR *pf = pcm->pszPrintType;
        while( *pf )
        {
            switch( *pf )
            {
                case L'W':
                    biDst.LastWriteTime.QuadPart = biSrc.LastWriteTime.QuadPart;
                    break;
                case L'C':
                    biDst.CreationTime.QuadPart = biSrc.CreationTime.QuadPart;
                    break;
                case L'A':
                    biDst.LastAccessTime.QuadPart = biSrc.LastAccessTime.QuadPart;
                    break;
                case L'H':
                    biDst.ChangeTime.QuadPart = biSrc.ChangeTime.QuadPart;
                    break;
                default:
                    // error
                    break;
            }
            pf++;
        }

        UpdateFileDateTime(pcm,&DstFilePath,&biDst);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////

//
// Process files in directory to recursive.
//
// If a file or directory with the same name as the copy source exists
// in the copy destination directory, copy is executed. 
//

static
VOID
PrepareCopyTimestamp(
    COPY_CONTEXT_STRUCT *pccs,
    PCWSTR Path,
    PCWSTR RelativePath,
    UNICODE_STRING *FileName
    )
{
    UNICODE_STRING srcDirPath;
    UNICODE_STRING srcFullPath;

    UNICODE_STRING dstDirPath;
    UNICODE_STRING dstFullPath;

    UNICODE_STRING usRelativePath;
    RtlInitUnicodeString(&usRelativePath,RelativePath);

    // Source
    RtlInitUnicodeString(&srcDirPath,Path);
    if( FileName )
        CombineUnicodeStringPath(&srcFullPath,&srcDirPath,FileName);
    else
        DuplicateUnicodeString(&srcFullPath,&srcDirPath); // for directory close

    // Destination
    CombineUnicodeStringPath(&dstDirPath,&pccs->DestinationPath,&usRelativePath);
    if( FileName )
        CombineUnicodeStringPath(&dstFullPath,&dstDirPath,FileName);
    else
        DuplicateUnicodeString(&dstFullPath,&dstDirPath); // for directory close

    // Copy date/time attribute
    CopyDateTime(pccs->pcm,srcFullPath,dstFullPath);

    RtlFreeUnicodeString(&srcFullPath);
    RtlFreeUnicodeString(&dstDirPath);
    RtlFreeUnicodeString(&dstFullPath);
}

static
NTSTATUS
CALLBACK
CopyTraverseDirectoryCallback(
    ULONG CallbackReason,
    PCWSTR Path,
    PCWSTR RelativePath,
    UNICODE_STRING *FileName,
    NTSTATUS Status,
    ULONG FileInfoType,  // Reserved always zero
    PVOID FileInfo,      // FILE_ID_BOTH_DIR_INFORMATION
    ULONG_PTR CallbackContext
    )
{
    COPY_CONTEXT_STRUCT *pccs = (COPY_CONTEXT_STRUCT *)CallbackContext;

    switch( CallbackReason )
    {
        case FFCBR_FINDFILE: // process file
        {
            if( pccs->pcm->UpdateFile )
                PrepareCopyTimestamp(pccs,Path,RelativePath,FileName);
            break;
        }
        case FFCBR_DIRECTORYSTART: // open directory
        {
            break;
        }
        case FFCBR_DIRECTORYEND: // close directory
        {
            if( pccs->pcm->UpdateDirectory )
                PrepareCopyTimestamp(pccs,Path,RelativePath,NULL);
            break;
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

//
// Process files directly under the directory.
//
// If a file or directory with the same name as the copy source exists
// in the copy destination directory, copy is executed. 
//
static
BOOLEAN
CALLBACK
CopyEnumFilesCallback(
    HANDLE hDirectory,
    PCWSTR DirectoryName,
    PVOID pInfoBuffer,
    ULONG_PTR Context
    )
{
	FILE_ID_BOTH_DIR_INFORMATION *pFileInfo = (FILE_ID_BOTH_DIR_INFORMATION *)pInfoBuffer;

    if( IS_RELATIVE_DIR_NAME_WITH_UNICODE_SIZE(pFileInfo->FileName,pFileInfo->FileNameLength) )
        return TRUE;

    COPY_CONTEXT_STRUCT *pccs = (COPY_CONTEXT_STRUCT *)Context;
    CCommandRunParam *pcm = pccs->pcm;

    if( pccs->pcm->UpdateDirectory == false && (pFileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        return TRUE;

    if( pccs->pcm->UpdateFile == false && ((pFileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) )
        return TRUE;

    // File name
    UNICODE_STRING usFileName;
    usFileName.Buffer = pFileInfo->FileName;
    usFileName.Length = usFileName.MaximumLength = (USHORT)pFileInfo->FileNameLength;

    // Source fully file path
    UNICODE_STRING usSrcFullPath;
    PWSTR pszFullPath;
    pszFullPath = CombinePath_U(DirectoryName,&usFileName);
    RtlInitUnicodeString(&usSrcFullPath,pszFullPath);

    // Destination fullyfile path
    UNICODE_STRING usDstFullPath;
    CombineUnicodeStringPath(&usDstFullPath,&pccs->DestinationPath,&usFileName);

    if( PathFileExists_U(&usDstFullPath,NULL) )
    {
#if _DEBUG_PRINT
        printf("Source      : %wZ\n",&usSrcFullPath);
        printf("Destination : %wZ\n",&usDstFullPath);
        printf("\n");
#endif
        NTSTATUS Status;
        Status = CopyDateTime(pcm,usSrcFullPath,usDstFullPath);
        if( Status != STATUS_SUCCESS )
        {
            PrintError(Status);
        }
    }

    RtlFreeUnicodeString(&usDstFullPath);

    FreeMemory(pszFullPath);

    return TRUE; // no abort, always continue
}

static
VOID
CopyEnumFiles(
    CCommandRunParam *pcm,
    UNICODE_STRING *Path,
    UNICODE_STRING *FileName,
    UNICODE_STRING *DestinationPath
    )
{
    PWSTR pszPath = AllocateSzFromUnicodeString(Path);
    PWSTR pszName = AllocateSzFromUnicodeString(FileName);

    COPY_CONTEXT_STRUCT ccs = {0};
    ccs.pcm = pcm;
    ccs.DestinationPath = *DestinationPath;

    NTSTATUS Status;
    Status = EnumFiles(NULL,pszPath,pszName,&CopyEnumFilesCallback,(ULONG_PTR)&ccs);

    if( NT_ERROR(Status) )
    {
        PrintError(Status, pszName);
    }

    FreeMemory(pszPath);
    FreeMemory(pszName);
}

/////////////////////////////////////////////////////////////////////////////

static HRESULT ProcessCopy(CCommandRunParam *pcm,CCommandRunPath *psrcPath,CCommandRunPath *pdstPath)
{
    HRESULT hr = E_FAIL;

    if( psrcPath->EnumDirectoryFiles == false )
    {
        // Single file date/time copy mode:
        //
        hr = HRESULT_FROM_NT( CopyDateTime(pcm,psrcPath->FullPath,pdstPath->FullPath) );
        return hr;
    }
    else if( pcm->Recursive )
    {
        // Recursive dirctory scan mode:
        // Enumerate files all directory under specified .
        // Do recursive traverse in sub-directory.
        // 
        COPY_CONTEXT_STRUCT ccs = {0};

        ccs.pcm = pcm;
        ccs.DestinationPath = pdstPath->FullPath;

        hr = HRESULT_FROM_NT( TraverseDirectory(psrcPath->FullPath,psrcPath->FileName,TRUE,0,&CopyTraverseDirectoryCallback,(ULONG_PTR)&ccs) );
    }
    else
    {
        // No recursive mode:
        // Enumerate files only under directory specified. 
        // Does not recursive traverse in sub-directory.
        // 
        CopyEnumFiles(pcm,&psrcPath->FullPath,&psrcPath->FileName,&pdstPath->FullPath);
        hr = S_OK;
    }

    return hr;
}

HRESULT CommandCopy(CCommandRunParam& cmd)
{
    HRESULT hr = E_FAIL;

    CFileItem **ppS = cmd.FileList.FirstFile();
    CFileItem **ppD = cmd.FileList.NextFile(ppS);

    CCommandRunPath srcPath;
    CCommandRunPath dstPath;

    if( ppS && ppD &&
        DeterminePathType(&cmd,**ppS,&srcPath) && 
        DeterminePathType(&cmd,**ppD,&dstPath) )
    {
        if( !PathFileExists_U(&srcPath.FullPath,NULL) )
        {
            PrintError( STATUS_NO_SUCH_FILE, srcPath.FullPath.Buffer );
            return hr;
        }

        if( !PathFileExists_U(&dstPath.FullPath,NULL) )
        {
            PrintError( STATUS_NO_SUCH_FILE, dstPath.FullPath.Buffer  );
            return hr;
        }

        if( (IsDirectory_U(&srcPath.FullPath) && srcPath.EnumDirectoryFiles) && 
            !IsDirectory_U(&dstPath.FullPath) )
        {
            PrintError( STATUS_INVALID_PARAMETER );
            return hr;
        }

        if( IsDirectory_U(&srcPath.FullPath) && !srcPath.EnumDirectoryFiles && cmd.Recursive )
        {
            // if cmd.Recursive == true, force setting recursive mode.
            RtlFreeUnicodeString( &srcPath.FileName );
            RtlCreateUnicodeString(&srcPath.FileName,L"*"); 
            srcPath.EnumDirectoryFiles = true;
        }

        // start copy a file.
        //
        hr = ProcessCopy(&cmd,&srcPath,&dstPath);
    }
    else
    {
        PrintError( STATUS_INVALID_PARAMETER );
    }

    return hr;
}
