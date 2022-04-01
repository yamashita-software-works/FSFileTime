//***************************************************************************
//*                                                                         *
//*  ntdirtraverse.cpp                                                      *
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

#define _PAGESIZE 4096

//---------------------------------------------------------------------------
// Directory Traverse
//---------------------------------------------------------------------------
typedef struct _DIRECTORY_TRAVERSE_PARAM
{
    WCHAR *Path;
    // Recursive call
    //
    BOOLEAN bRecursive;
    int RecursiveCallLevel;
    // User Callback
    //
    FINDFILECALLBACK pfnCallback;
    ULONG_PTR CallbackContext;

    UNICODE_STRING FileName;

    ULONG Flags;
} DIRECTORY_TRAVERSE_PARAM;

#define DTF_NO_PROCESS_WILDCARD 0x1

class __declspec(novtable) CTraverseDirectoryParam : public DIRECTORY_TRAVERSE_PARAM
{
    PWSTR m_pRootPoint;
    PWSTR m_pConcatenatePoint;
    ULONG m_cbCurDirNameLength;

public:
    CTraverseDirectoryParam()
    {
        Path = NULL;
        RecursiveCallLevel = 0;
        m_pRootPoint = NULL;
        m_pConcatenatePoint = NULL;
        m_cbCurDirNameLength = 0;
    }

    ~CTraverseDirectoryParam()
    {
        FreeMemory(Path);
    }

    NTSTATUS InitTraverseRoot(PCWSTR pszPath,ULONG cbLength)
    {
        ULONG cb = 32768 * sizeof(WCHAR);
        Path = (WCHAR *)AllocMemory( cb );
        if( Path == NULL )
            return STATUS_NO_MEMORY;
#ifdef _DEBUG
        memset(Path,0xcc,cb);
#else
        memset(Path,0,cb);
#endif
        int cch = cbLength / sizeof(WCHAR);
        RtlCopyMemory(Path,pszPath,cbLength);

        if( *(Path + (cch - 1)) != L'\\' ) // append backslash
        {
            *(Path + cch) = L'\\';
            cch++;
        }

        m_pConcatenatePoint = Path + cch;
        *m_pConcatenatePoint = L'\0';

        // Target is specified directory as current directory .
        //
        m_pRootPoint = m_pConcatenatePoint;

        return STATUS_SUCCESS;
    }

    VOID PushDirectory(PWSTR FileName,ULONG FileNameLength)
    {
        RtlCopyMemory(m_pConcatenatePoint,FileName,FileNameLength);

        m_pConcatenatePoint += (FileNameLength / sizeof(WCHAR));
        *m_pConcatenatePoint++ = L'\\';
        *m_pConcatenatePoint = L'\0';

        // hold current name length
        m_cbCurDirNameLength = FileNameLength;

        RecursiveCallLevel++;
    }

    VOID PopDirectory(ULONG FileNameLength)
    {
        m_pConcatenatePoint -= ((FileNameLength / sizeof(WCHAR)) + 1);
        *m_pConcatenatePoint = L'\0';
        RecursiveCallLevel--;
    }

    PCWSTR RefRelativeRootPtr() const
    {
        return m_pRootPoint; 
    }

    PCWSTR GetFullPath() const
    {
        return Path;
    }

    ULONG GetFullPathLength() const
    {
        return (ULONG)(wcslen(Path) * sizeof(WCHAR));
    }

    PCWSTR GetCurrentDirectoryName() const
    {
        ULONG cb = GetFullPathLength();
        int cch = (int)((cb - m_cbCurDirNameLength)/sizeof(WCHAR)) - 1;
        return &Path[cch];
    }

    int GetRecursiveCallLevel() const 
    {
        return RecursiveCallLevel;
    }
};

static NTSTATUS handleStartDirectory(FILE_ID_BOTH_DIR_INFORMATION *pDirectory,CTraverseDirectoryParam *pDTP)
{
    if( pDTP->pfnCallback )
    {
        UNICODE_STRING usFileName;
        usFileName.Buffer = (PWCH)pDirectory->FileName;
        usFileName.Length = usFileName.MaximumLength = (USHORT)pDirectory->FileNameLength;

        if( pDTP->FileName.Buffer != NULL && !(pDTP->Flags & DTF_NO_PROCESS_WILDCARD) )
        {
            NTSTATUS Status = STATUS_SUCCESS;
            PWSTR pszPattern = AllocateSzFromUnicodeString(&pDTP->FileName);
            if( _UStrMatchI_U(pszPattern,&usFileName) )
            {
                Status = pDTP->pfnCallback(FFCBR_DIRECTORYSTART,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pDirectory,pDTP->CallbackContext);
            }
            FreeMemory(pszPattern);
            return Status;
        }
        return pDTP->pfnCallback(FFCBR_DIRECTORYSTART,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pDirectory,pDTP->CallbackContext);
    }

    return STATUS_SUCCESS;
}

static NTSTATUS handleEndDirectory(FILE_ID_BOTH_DIR_INFORMATION *pDirectory,CTraverseDirectoryParam *pDTP)
{
    if( pDTP->pfnCallback )
    {
        UNICODE_STRING usFileName;
        usFileName.Buffer = (PWCH)pDirectory->FileName;
        usFileName.Length = usFileName.MaximumLength = (USHORT)pDirectory->FileNameLength;

        if( pDTP->FileName.Buffer != NULL && !(pDTP->Flags & DTF_NO_PROCESS_WILDCARD) )
        {
            NTSTATUS Status = STATUS_SUCCESS;
            PWSTR pszPattern = AllocateSzFromUnicodeString(&pDTP->FileName);
            if( _UStrMatchI_U(pszPattern,&usFileName) )
            {
                Status = pDTP->pfnCallback(FFCBR_DIRECTORYEND,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pDirectory,pDTP->CallbackContext);
            }
            FreeMemory(pszPattern);
            return Status;
        }
        return pDTP->pfnCallback(FFCBR_DIRECTORYEND,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pDirectory,pDTP->CallbackContext);
    }

    return STATUS_SUCCESS;
}

static NTSTATUS handleFile(FILE_ID_BOTH_DIR_INFORMATION *pFile,CTraverseDirectoryParam *pDTP)
{
    if( pDTP->pfnCallback )
    {
        UNICODE_STRING usFileName;
        usFileName.Buffer = (PWCH)pFile->FileName;
        usFileName.Length = usFileName.MaximumLength = (USHORT)pFile->FileNameLength;

        if( pDTP->FileName.Buffer != NULL && !(pDTP->Flags & DTF_NO_PROCESS_WILDCARD) )
        {
            NTSTATUS Status = STATUS_SUCCESS;
            PWSTR pszPattern = AllocateSzFromUnicodeString(&pDTP->FileName);
            if( _UStrMatchI_U(pszPattern,&usFileName) )
            {
                Status = pDTP->pfnCallback(FFCBR_FINDFILE,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pFile,pDTP->CallbackContext);
            }
            FreeMemory(pszPattern);
            return Status;
        }

        return pDTP->pfnCallback(FFCBR_FINDFILE,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,0,0,pFile,pDTP->CallbackContext);
    }

    return STATUS_SUCCESS;
}

static NTSTATUS handleError(FILE_ID_BOTH_DIR_INFORMATION *pDirectory,CTraverseDirectoryParam *pDTP,NTSTATUS Status)
{
    if( pDTP->pfnCallback )
    {
        UNICODE_STRING usFileName;
        usFileName.Buffer = (PWCH)pDirectory->FileName;
        usFileName.Length = usFileName.MaximumLength = (USHORT)pDirectory->FileNameLength;

        return pDTP->pfnCallback(FFCBR_ERROR,pDTP->GetFullPath(),pDTP->RefRelativeRootPtr(),&usFileName,Status,0,pDirectory,pDTP->CallbackContext);
    }
    return STATUS_SUCCESS;
}

static
NTSTATUS
_TraverseDirectoryImpl(
    HANDLE hParent,
    UNICODE_STRING *NtPathName,
    FILE_ID_BOTH_DIR_INFORMATION *pDirectory,
    CTraverseDirectoryParam *pDTP
    )
{
    HANDLE hDirectory;
    NTSTATUS Status;
    BOOLEAN bRestartScan = TRUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus = {0};

    if( (Status = handleStartDirectory(pDirectory,pDTP)) != STATUS_SUCCESS )
        return Status;

    // open directory
    //
    InitializeObjectAttributes(&ObjectAttributes,NtPathName,0,hParent,NULL);

    Status = NtOpenFile(&hDirectory,
                FILE_LIST_DIRECTORY|FILE_TRAVERSE|SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT
                // FILE_OPEN_REPARSE_POINT bypass reparse point processing for the file. 
                );

    if( Status == STATUS_SUCCESS )
    {
        FILE_ID_BOTH_DIR_INFORMATION *pBuffer = NULL;
        ULONG cbBuffer = _PAGESIZE * 8;

        pBuffer = (FILE_ID_BOTH_DIR_INFORMATION*)AllocMemory( cbBuffer );

        if( pBuffer == NULL )
        {
            return STATUS_NO_MEMORY;
        }

        for(;;)
        {
            Status = NtQueryDirectoryFile(hDirectory,
                                NULL,NULL,NULL,
                                &IoStatus,
                                pBuffer,cbBuffer,
                                FileIdBothDirectoryInformation,
                                FALSE,
                                NULL,
                                bRestartScan
                                );

            if( Status != STATUS_SUCCESS )
            {
                if( Status == STATUS_NO_MORE_FILES )
                    Status = STATUS_SUCCESS;
                break;
            }

            FILE_ID_BOTH_DIR_INFORMATION *p = pBuffer;

            while( Status == STATUS_SUCCESS )
            {
                if( !IS_RELATIVE_DIR_NAME_WITH_UNICODE_SIZE( p->FileName, p->FileNameLength ) )
                {
                    if( pDTP->bRecursive && (p->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                    {
                        pDTP->PushDirectory(p->FileName,p->FileNameLength);

                        UNICODE_STRING path;
                        path.Buffer = p->FileName;
                        path.Length = path.MaximumLength = (USHORT)p->FileNameLength;

                        Status = _TraverseDirectoryImpl(hDirectory,&path,p,pDTP);

                        pDTP->PopDirectory(p->FileNameLength);
                    }
                    else
                    {
                        Status = handleFile(p,pDTP);
                    }
                }

                if( p->NextEntryOffset == 0 )
                {
                    break;
                }

                ((ULONG_PTR&)p) += p->NextEntryOffset;
            }

            // NOTE: The RestartScan parameter is currently ignored.
            //
            bRestartScan = FALSE;
        }

        FreeMemory(pBuffer);

        NtClose(hDirectory);
    }
    else
    {
        handleError(pDirectory,pDTP,Status);
    }

    Status = handleEndDirectory(pDirectory,pDTP);

    return Status;
}

//---------------------------------------------------------------------------
//
//  QueryDirectoryEntryInfo()
//
//---------------------------------------------------------------------------
NTSTATUS
QueryDirectoryEntryInformation(
    HANDLE hRoot,
    UNICODE_STRING* pusPath,
    FILE_ID_BOTH_DIR_INFORMATION **DirectoryEntry
    )
{
    HANDLE hDirectory;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;

    InitializeObjectAttributes(&ObjectAttributes,pusPath,0,hRoot,NULL);

    Status = NtOpenFile(&hDirectory,
                FILE_LIST_DIRECTORY|FILE_TRAVERSE|SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT
                // FILE_OPEN_REPARSE_POINT bypass reparse point processing for the file. 
                );

    if( Status == STATUS_SUCCESS )
    {
        FILE_ID_BOTH_DIR_INFORMATION *pBuffer = NULL;
        ULONG cbBuffer = sizeof(FILE_ID_BOTH_DIR_INFORMATION) + (sizeof(WCHAR) * WIN32_MAX_PATH);

        pBuffer = (FILE_ID_BOTH_DIR_INFORMATION*)AllocMemory( cbBuffer );

        if( pBuffer != NULL )
        {
            Status = NtQueryDirectoryFile(hDirectory,
                                NULL,NULL,NULL,
                                &IoStatus,
                                pBuffer,cbBuffer,
                                FileIdBothDirectoryInformation,
                                FALSE,
                                NULL,
                                TRUE
                                );

            if( Status == STATUS_SUCCESS )
            {
                pBuffer = (FILE_ID_BOTH_DIR_INFORMATION*)ReAllocateHeap(pBuffer,IoStatus.Information); // shrink buffer
                *DirectoryEntry = pBuffer;
            }
            else
            {
                *DirectoryEntry = NULL;
            }
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }

        NtClose(hDirectory);
    }

    return Status;
}

//---------------------------------------------------------------------------
//
//  TraverseDirectory()
//
//---------------------------------------------------------------------------
NTSTATUS
TraverseDirectory(
    UNICODE_STRING& DirectoryFullPath,
    UNICODE_STRING& FileName,
    BOOLEAN bRecursive,
    ULONG Flags,
    FINDFILECALLBACK pfnCallback,
    ULONG_PTR CallbackContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    CTraverseDirectoryParam *pDTP = new CTraverseDirectoryParam;
    if( pDTP == NULL )
        return STATUS_NO_MEMORY;

    pDTP->pfnCallback = pfnCallback;
    pDTP->CallbackContext = CallbackContext;
    pDTP->bRecursive = bRecursive;
    pDTP->FileName = FileName;
    pDTP->Flags = Flags;

    if( pDTP->InitTraverseRoot(DirectoryFullPath.Buffer,DirectoryFullPath.Length) == STATUS_SUCCESS )
    {
        FILE_ID_BOTH_DIR_INFORMATION *pd;

        if( NT_SUCCESS(QueryDirectoryEntryInformation(NULL,&DirectoryFullPath,&pd)) )
        {
            Status = _TraverseDirectoryImpl(NULL,&DirectoryFullPath,pd,pDTP);

            FreeMemory(pd);
        }
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

    delete pDTP;
    
    return Status;
}
