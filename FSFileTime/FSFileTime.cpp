//***************************************************************************
//*                                                                         *
//*  FSFileTime.cpp                                                         *
//*                                                                         *
//*  Create: 2020-10-06                                                     *
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

//----------------------------------------------------------------------------
//
//  AddMillisecondsString()
//
//----------------------------------------------------------------------------
HRESULT AddMillisecondsString(LARGE_INTEGER& DateTime,LPTSTR pszText,int cchTextMax)
{
    WCHAR szMilliseconds[8];
    TIME_FIELDS tf;
    HRESULT hr;

    RtlTimeToTimeFields(&DateTime,&tf);

    hr = StringCchPrintf(szMilliseconds,ARRAYSIZE(szMilliseconds),L".%03u",tf.Milliseconds);
    if( hr == S_OK )
    {
        hr = StringCchCat(pszText,cchTextMax,szMilliseconds);
    }

    return hr;
}

//----------------------------------------------------------------------------
//
//  PrintFileDateTime()
//
//----------------------------------------------------------------------------
VOID PrintFileDateTime(CCommandRunParam *pcm,UNICODE_STRING *FileName,FILE_BASIC_INFORMATION *FileInfo)
{
    PCWSTR pszDF = pcm->GetDateFormat();
    PCWSTR pszTF = pcm->GetTimeFormat();
    ULONG fDateFormatFlags = 0;
    ULONG fTimeFormatFlags = 0;
    BOOLEAN bUTF = pcm->UTCMode;

    WCHAR szCreationDate[64+1];
    WCHAR szLastWriteDate[64+1];
    WCHAR szLastAccessDate[64+1];
    WCHAR szChangeDate[64+1];

    WCHAR szCreationTime[64+1];
    WCHAR szLastWriteTime[64+1];
    WCHAR szLastAccessTime[64+1];
    WCHAR szChangeTime[64+1];

    RtlZeroMemory(szCreationDate,sizeof(szCreationDate));
    RtlZeroMemory(szLastWriteDate,sizeof(szLastWriteDate));
    RtlZeroMemory(szLastAccessDate,sizeof(szLastAccessDate));
    RtlZeroMemory(szChangeDate,sizeof(szChangeDate));
    RtlZeroMemory(szCreationTime,sizeof(szCreationTime));
    RtlZeroMemory(szLastWriteTime,sizeof(szLastWriteTime));
    RtlZeroMemory(szLastAccessTime,sizeof(szLastAccessTime));
    RtlZeroMemory(szChangeTime,sizeof(szChangeTime));

    if( pcm->HexDumpMode )
    {
        // LastWrite
        StringCchPrintf(szLastWriteDate,ARRAYSIZE(szLastWriteDate),L"0x%016I64X",FileInfo->LastWriteTime.QuadPart);

        // Creation
        StringCchPrintf(szCreationDate,ARRAYSIZE(szCreationDate),L"0x%016I64X",FileInfo->CreationTime.QuadPart);

        // LastAccess
        StringCchPrintf(szLastAccessDate,ARRAYSIZE(szLastAccessDate),L"0x%016I64X",FileInfo->LastAccessTime.QuadPart);

        // Attribute Change
        StringCchPrintf(szChangeDate,ARRAYSIZE(szChangeDate),L"0x%016I64X",FileInfo->ChangeTime.QuadPart);
    }
    else
    {
        // LastWrite
        WinGetDateString(FileInfo->LastWriteTime.QuadPart,szLastWriteDate,ARRAYSIZE(szLastWriteDate),pszDF,bUTF,fDateFormatFlags);
        WinGetTimeString(FileInfo->LastWriteTime.QuadPart,szLastWriteTime,ARRAYSIZE(szLastWriteTime),pszTF,bUTF,fTimeFormatFlags);
        
        // Creation
        WinGetDateString(FileInfo->CreationTime.QuadPart,szCreationDate,ARRAYSIZE(szCreationDate),pszDF,bUTF,fDateFormatFlags);
        WinGetTimeString(FileInfo->CreationTime.QuadPart,szCreationTime,ARRAYSIZE(szCreationTime),pszTF,bUTF,fTimeFormatFlags);

        // LastAccess
        WinGetDateString(FileInfo->LastAccessTime.QuadPart,szLastAccessDate,ARRAYSIZE(szLastAccessDate),pszDF,bUTF,fDateFormatFlags);
        WinGetTimeString(FileInfo->LastAccessTime.QuadPart,szLastAccessTime,ARRAYSIZE(szLastAccessTime),pszTF,bUTF,fTimeFormatFlags);

        // Attribute Change
        WinGetDateString(FileInfo->ChangeTime.QuadPart,szChangeDate,ARRAYSIZE(szChangeDate),pszDF,bUTF,fDateFormatFlags);
        WinGetTimeString(FileInfo->ChangeTime.QuadPart,szChangeTime,ARRAYSIZE(szChangeTime),pszTF,bUTF,fTimeFormatFlags);

        if( pcm->ShowMilliseconds )
        {
            AddMillisecondsString(FileInfo->LastWriteTime,szLastWriteTime,ARRAYSIZE(szLastWriteTime));
            AddMillisecondsString(FileInfo->CreationTime,szCreationTime,ARRAYSIZE(szCreationTime));
            AddMillisecondsString(FileInfo->LastAccessTime,szLastAccessTime,ARRAYSIZE(szLastAccessTime));
            AddMillisecondsString(FileInfo->ChangeTime,szChangeTime,ARRAYSIZE(szChangeTime));
        }
    }

    WCHAR *pD[4]={0},*pT[4]={0},*pH[4]={0};

    WCHAR *pf = pcm->pszPrintType;
    int dt = 0;
    while( *pf )
    {
        switch( *pf )
        {
            case L'W':
                pD[dt] = szLastWriteDate;
                pT[dt] = szLastWriteTime;
                pH[dt] = L"Last Wrire";
                break;
            case L'C':
                pD[dt] = szCreationDate;
                pT[dt] = szCreationTime;
                pH[dt] = L"Creation";
                break;
            case L'A':
                pD[dt] = szLastAccessDate;
                pT[dt] = szLastAccessTime;
                pH[dt] = L"Last Access";
                break;
            case L'H':
                pD[dt] = szChangeDate;
                pT[dt] = szChangeTime;
                pH[dt] = L"Attrib Change";
                break;
            default:
                // error
                break;
        }
        dt++;
        pf++;
    }

    int i;
    if( pcm->LineMode )
    {
        int time_width = 9;  // todo:
        int date_width = 10; // todo:

        for(i = 0; i < dt; i++ )
        {
            if( pcm->HexDumpMode )
                wprintf(L"%s ",pD[i]);
            else
                wprintf(L"%s %s ",pD[i],pT[i]);
        }
        wprintf(L"%wZ\n",FileName);
    }
    else
    {
        wprintf(L"%wZ\n",FileName);

        for(i = 0; i < dt; i++ )
        {
            if( pcm->HexDumpMode )
                wprintf(L"%15s : %s\n",pH[i],pD[i]);
            else
                wprintf(L"%15s : %s %s\n",pH[i],pD[i],pT[i]);
        }

        wprintf(L"\n");
    }
}

//----------------------------------------------------------------------------
//
//  DisplayFileDateTime()
//
//----------------------------------------------------------------------------
VOID DisplayFileDateTime( CCommandRunParam *pcm, UNICODE_STRING *FileNameWithFullPath )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes,FileNameWithFullPath,0,NULL,NULL);

    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatus = {0};

    Status = NtOpenFile(&Handle,FILE_READ_ATTRIBUTES,&ObjectAttributes,&IoStatus,FILE_SHARE_READ|FILE_SHARE_WRITE,0);

    if( Status == STATUS_SUCCESS )
    {
        RtlZeroMemory(&IoStatus,sizeof(IoStatus));

        FILE_BASIC_INFORMATION fbi = {0};

        if( (Status = NtQueryInformationFile(Handle,&IoStatus,&fbi,sizeof(fbi),FileBasicInformation)) == STATUS_SUCCESS )
        {
            PrintFileDateTime(pcm,FileNameWithFullPath,&fbi);
        }
        else
        {
            PrintError(Status);
        }

        NtClose(Handle);
    }
    else
    {
        PrintError(Status);
    }
}

//----------------------------------------------------------------------------
//
//  CopyTimeFields()
//
//----------------------------------------------------------------------------
VOID CopyTimeFields( TIME_FIELDS& tfDst, TIME_FIELDS& tfSrc )
{
    if( tfSrc.Year != -1 )
        tfDst.Year = tfSrc.Year;

    if( tfSrc.Month != -1 )
        tfDst.Month = tfSrc.Month;

    if( tfSrc.Day != -1 )
        tfDst.Day = tfSrc.Day;

    if( tfSrc.Hour != -1 )
        tfDst.Hour = tfSrc.Hour;

    if( tfSrc.Minute != -1 )
        tfDst.Minute = tfSrc.Minute;

    if( tfSrc.Second != -1 )
        tfDst.Second = tfSrc.Second;

    if( tfSrc.Milliseconds != -1 )
        tfDst.Milliseconds = tfSrc.Milliseconds;
}

//----------------------------------------------------------------------------
//
//  UpdateCopyDateTime()
//
//----------------------------------------------------------------------------
LARGE_INTEGER UpdateCopyDateTime( CCommandRunParam *pcm, LARGE_INTEGER& liFileDateTime, TIME_FIELDS& tfUserSpecified )
{
    TIME_FIELDS tfTemp;
    LARGE_INTEGER liDst;
    LARGE_INTEGER liFile;
    
    liFile = liFileDateTime;

    // current file date/time copy to liDst
    if( pcm->UTCMode )
        liDst = liFile;
    else
        RtlSystemTimeToLocalTime(&liFile,&liDst);

    // expand LARGE_INTEGER to TIME_FIELDS 
    RtlTimeToTimeFields(&liDst,&tfTemp);

    // copy user specified date/time structure members.
    CopyTimeFields(tfTemp,tfUserSpecified);

    // revert TIME_FIELDS to LARGE_INTEGER.
    RtlTimeFieldsToTime(&tfTemp,&liDst);

    // return value to liFile.
    if( pcm->UTCMode )
        liFile = liDst;
    else
        RtlLocalTimeToSystemTime(&liDst,&liFile);

    if( pcm->DosTimeWrite )
    {
        USHORT FatDate,FatTime;
        if( WinFileTimeToDosDateTime(&liFile,&FatDate,&FatTime) )
        {
            WinDosDateTimeToFileTime(FatDate,FatTime,&liFile);
        }
    }

    return liFile;
}

//----------------------------------------------------------------------------
//
//  UpdateCopyDateTime()
//
//----------------------------------------------------------------------------
LARGE_INTEGER UpdateCopySystemTime( CCommandRunParam *pcm, LARGE_INTEGER& liUserSpecified )
{
    LARGE_INTEGER liFile;
    
    liFile = liUserSpecified;

    if( pcm->DosTimeWrite )
    {
        USHORT FatDate,FatTime;
        if( WinFileTimeToDosDateTime(&liFile,&FatDate,&FatTime) )
        {
            WinDosDateTimeToFileTime(FatDate,FatTime,&liFile);
        }
    }

    return liFile;
}

//----------------------------------------------------------------------------
//
//  SetFileDateTime()
//
//----------------------------------------------------------------------------
VOID SetFileDateTime( CCommandRunParam *pcm, FILE_BASIC_INFORMATION *pfbi )
{
    if( isUnsetLargeInteger(pcm->liLastWrite) )
    {
        pfbi->LastWriteTime = UpdateCopyDateTime(pcm,pfbi->LastWriteTime,pcm->tfLastWrite);
    }
    else
    {
        pfbi->LastWriteTime = UpdateCopySystemTime(pcm,pcm->liLastWrite);
    }

    if( isUnsetLargeInteger(pcm->liCreation) )
    {
        pfbi->CreationTime = UpdateCopyDateTime(pcm,pfbi->CreationTime,pcm->tfCreation);
    }
    else
    {
        pfbi->CreationTime = UpdateCopySystemTime(pcm,pcm->liCreation);
    }

    if( isUnsetLargeInteger(pcm->liLastAccess) )
    {
        pfbi->LastAccessTime = UpdateCopyDateTime(pcm,pfbi->LastAccessTime,pcm->tfLastAccess);
    }
    else
    {
        pfbi->LastAccessTime = UpdateCopySystemTime(pcm,pcm->liLastAccess);
    }

    if( isUnsetLargeInteger(pcm->liAttrChange) )
    {
        pfbi->ChangeTime = UpdateCopyDateTime(pcm,pfbi->ChangeTime,pcm->tfAttrChange);
    }
    else
    {
        pfbi->ChangeTime = UpdateCopySystemTime(pcm,pcm->liAttrChange);
    }
}

//----------------------------------------------------------------------------
//
//  UpdateFileDateTime()
//
//----------------------------------------------------------------------------
VOID UpdateFileDateTime( CCommandRunParam *pcm, UNICODE_STRING *FileNameWithFullPath, FILE_BASIC_INFORMATION *pBasicInfo )
{
    if( !pcm->SuppressPrompt )
    {
        printf("Update '%wZ' (y/N) :",FileNameWithFullPath);
        int ch = _getch();
        printf("\n");
        if( ch == 0x3 || ch == 'Q' || ch == 'q' ) // ctrl+C or Quit
            exit(0);
        if( ch != 'y' && ch != 'Y' )
            return;
    }
    else
    {
        printf("%wZ ",FileNameWithFullPath);
    }

    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes,FileNameWithFullPath,0,NULL,NULL);

    HANDLE Handle;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus = {0};

    Status = NtOpenFile(&Handle,FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES,
                    &ObjectAttributes,&IoStatus,FILE_SHARE_READ|FILE_SHARE_WRITE,0);

    if( Status == STATUS_SUCCESS )
    {
        FILE_BASIC_INFORMATION fbi = {0};

        if( pBasicInfo != NULL )
        {
            fbi = *pBasicInfo;
        }
        else
        {
            NtQueryInformationFile(Handle,&IoStatus,&fbi,sizeof(fbi),FileBasicInformation);
            SetFileDateTime(pcm,&fbi);
        }

        if( !pcm->TestRunMode )
            Status = NtSetInformationFile(Handle,&IoStatus,&fbi,sizeof(fbi),FileBasicInformation);
        else
            Status = STATUS_SUCCESS;

        if( Status == STATUS_SUCCESS )
        {
            if( pcm->ShowResult )
                PrintFileDateTime(pcm,FileNameWithFullPath,&fbi);
            else
                printf("succeeded.\n");
        }
        else
        {
            printf("failed (0x%08X)\n",Status);
        }

        NtClose(Handle);
    }
    else
    {
        PrintError( Status );
    }
}

//----------------------------------------------------------------------------
//
//  ActionFileDateTime()
//
//----------------------------------------------------------------------------
VOID ActionFileDateTime( CCommandRunParam *pcm, UNICODE_STRING *FileNameWithFullPath )
{
    switch( pcm->action )
    {
        case Query:
            DisplayFileDateTime( pcm, FileNameWithFullPath );
            break;
        case Set:
            UpdateFileDateTime( pcm, FileNameWithFullPath, NULL );
            break;
    }
}

//----------------------------------------------------------------------------
//
//  EnumFileCallback()
//
//----------------------------------------------------------------------------
BOOLEAN CALLBACK EnumFileCallback(HANDLE hDirectory,PCWSTR DirectoryName,
                                  PVOID pInfoBuffer,ULONG_PTR Context)
{
	FILE_ID_BOTH_DIR_INFORMATION *pFileInfo = (FILE_ID_BOTH_DIR_INFORMATION *)pInfoBuffer;

    if( IS_RELATIVE_DIR_NAME_WITH_UNICODE_SIZE(pFileInfo->FileName,pFileInfo->FileNameLength) )
        return TRUE;

    CCommandRunParam *pcm = (CCommandRunParam *)Context;

    if( pcm->UpdateDirectory == FALSE && (pFileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        return TRUE;

    if( pcm->UpdateFile == false && ((pFileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) )
        return TRUE;

    UNICODE_STRING usFileName;

    usFileName.Buffer = pFileInfo->FileName;
    usFileName.Length = usFileName.MaximumLength = (USHORT)pFileInfo->FileNameLength;

    PWSTR pszFullPath;
    pszFullPath = CombinePath_U(DirectoryName,&usFileName);

    UNICODE_STRING usFullyQualifiedFilePath;
    RtlInitUnicodeString(&usFullyQualifiedFilePath,pszFullPath);

    ActionFileDateTime( pcm, &usFullyQualifiedFilePath );

    FreeMemory(pszFullPath);

    return TRUE;
}

//----------------------------------------------------------------------------
//
//  EnumDirectoryFiles()
//
//----------------------------------------------------------------------------
VOID EnumDirectoryFiles(CCommandRunParam *pcm, UNICODE_STRING *Path, UNICODE_STRING *FileName)
{
    PWSTR pszPath = AllocateSzFromUnicodeString(Path);
    PWSTR pszName = AllocateSzFromUnicodeString(FileName);

    NTSTATUS Status;
    Status = EnumFiles(NULL,pszPath,pszName,&EnumFileCallback,(ULONG_PTR)pcm);

    if( NT_ERROR(Status) )
    {
        PrintError(Status, pszName);
    }

    FreeMemory(pszPath);
    FreeMemory(pszName);
}

//----------------------------------------------------------------------------
//
//  RecursiveEnumDirectoryFilesCallback()
//
//----------------------------------------------------------------------------
NTSTATUS CALLBACK RecursiveEnumDirectoryFilesCallback(
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
    CCommandRunParam *pcm = (CCommandRunParam *)CallbackContext;

    switch( CallbackReason )
    {
        case FFCBR_FINDFILE: // process file
        {
            if( (pcm->action == Set || pcm->action == Query) && pcm->UpdateFile )
            {
                UNICODE_STRING usDirPath;
                UNICODE_STRING usFullFilePath;

                RtlInitUnicodeString(&usDirPath,Path);
                CombineUnicodeStringPath(&usFullFilePath,&usDirPath,FileName);

                ActionFileDateTime( pcm, &usFullFilePath );

                RtlFreeUnicodeString(&usFullFilePath);
            }
            break;
        }
        case FFCBR_DIRECTORYSTART: // open directory
        {
            if( pcm->action == Query && pcm->UpdateDirectory )
            {
                UNICODE_STRING usDirPath;
                RtlInitUnicodeString(&usDirPath,Path);
                ActionFileDateTime( pcm, &usDirPath );
            }
            break;
        }
        case FFCBR_DIRECTORYEND: // close directory
        {
            if( pcm->action == Set && pcm->UpdateDirectory )
            {
                UNICODE_STRING usDirPath;
                RtlInitUnicodeString(&usDirPath,Path);
                ActionFileDateTime( pcm, &usDirPath );
            }
            break;
        }
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//  RecursiveEnumDirectoryFiles()
//
//----------------------------------------------------------------------------
VOID RecursiveEnumDirectoryFiles(CCommandRunParam *pcm, UNICODE_STRING *Path, UNICODE_STRING *FileName)
{
    TraverseDirectory(*Path,*FileName,TRUE,0,&RecursiveEnumDirectoryFilesCallback,(ULONG_PTR)pcm);
}

//----------------------------------------------------------------------------
//
//  CommandProcessFiles()
//
//----------------------------------------------------------------------------
HRESULT CommandProcessFiles(CCommandRunParam& cmd)
{
    HRESULT hr = E_FAIL;

    CCommandRunPath Path;

    CFileItem **pp = cmd.FileList.FirstFile();

#if 0
    //
    // Prepare processing
    //
    while( pp != NULL )
    {
        DeterminePathType(&cmd,**pp,&Path);
        pp = cmd.FileList.NextFile(pp);
    }
#endif

    //
    // Actual processing
    //
    pp = cmd.FileList.FirstFile();

    while( pp != NULL )
    {
        DeterminePathType(&cmd,**pp,&Path);

        if( IsDirectory_U(&Path.FullPath) && Path.EnumDirectoryFiles == FALSE && cmd.Recursive )
        {
            // If the directory path last character in does not end with '\'
            // and if cmd.Recursive == true, force setting recursive mode.
            // example: "\??\C:\windows", "\??\C:\windows\system32"
            RtlFreeUnicodeString( &Path.FileName );
            RtlCreateUnicodeString(&Path.FileName,L"*"); 
            Path.EnumDirectoryFiles = TRUE;
        }

        BOOLEAN PathExists = PathFileExists_U(&Path.FullPath,NULL);

        if( PathExists )
        {
            if( IsDirectory_U(&Path.FullPath) )
            {
                if( cmd.Recursive && Path.EnumDirectoryFiles )
                    RecursiveEnumDirectoryFiles(&cmd,&Path.FullPath,&Path.FileName);
                else if( Path.EnumDirectoryFiles )
                    EnumDirectoryFiles(&cmd,&Path.FullPath,&Path.FileName);
                else
                    ActionFileDateTime(&cmd,&Path.FullPath);
            }
            else
            {
                ActionFileDateTime(&cmd,&Path.FullPath);
            }
        }
        else
        {
            PrintError(STATUS_NO_SUCH_FILE, Path.FileName.Buffer);
        }

        pp = cmd.FileList.NextFile(pp);
    }

    return hr;
}

//----------------------------------------------------------------------------
//
//  DeterminePathType()
//
//----------------------------------------------------------------------------
BOOLEAN DeterminePathType(CCommandRunParam *,FILEITEM& fi,COMMAND_RUN_PATH *prunpath)
{
    BOOLEAN PathExists = FALSE;
    BOOLEAN Success = FALSE;

    __try
    {
        if( IsNtDevicePath(fi.pszFilename) )
        {
            //
            // NT device name space path
            // In this case absolute path only.
            //

            if( AllocateUnicodeString(&prunpath->FullPath,fi.pszFilename) == STATUS_SUCCESS )
            {
                if( NT_ERROR(GetFileNamePart_U(&prunpath->FullPath,&prunpath->FileName)) )
                {
                    Success = FALSE;
                    __leave;
                }
            }
        }
        else
        {
            //
            // DOS drive path
            //
            PCWSTR FileNamePart = NULL;

            if( IsRelativePath(fi.pszFilename) )
            {
                // If path string is a relative format, to determine as a DOS path.
                //
                RtlDosPathNameToRelativeNtPathName_U(fi.pszFilename,&prunpath->FullPath,(PWSTR*)&FileNamePart,&prunpath->RelativeDirPart);
            }
            else
            {
                RtlDosPathNameToNtPathName_U(fi.pszFilename,&prunpath->FullPath,(PWSTR*)&FileNamePart,NULL);
            }

            if( FileNamePart != NULL )
                RtlCreateUnicodeString(&prunpath->FileName,FileNamePart);
        }

        if( prunpath->FullPath.Buffer == NULL )
        {
            Success = FALSE;
            __leave; // fatal error, abort
        }

        prunpath->EnumDirectoryFiles = FALSE;

        if( IsRootDirectory_U(&prunpath->FullPath) )
        {
            ;
        }
        else if( IsLastCharacterBackslash_U(&prunpath->FullPath) )
        {
            RtlCreateUnicodeString(&prunpath->FileName,L"*"); 
            prunpath->EnumDirectoryFiles = TRUE;
        }
        else
        {
            // If filename part includes wildcard character, split the path 
            // the directory-part and filename-part.
            //
            if( HasWildCardChar_U(&prunpath->FileName) )
            {
                // Remove filename spec
                SplitPathFileName_U(&prunpath->FullPath,NULL);
                prunpath->EnumDirectoryFiles = TRUE;
            }
        }

        Success = TRUE;
    }
    __finally
    {
#ifdef _DEBUG
        PWSTR p1,p2;
        p1 = AllocateSzFromUnicodeString(&prunpath->FullPath);
        p2 = AllocateSzFromUnicodeString(&prunpath->FileName);
        FreeMemory(p1);
        FreeMemory(p2);
#endif
    }

    return Success;
}

//----------------------------------------------------------------------------
//
//  PrintTimeToBin()
//
//----------------------------------------------------------------------------
VOID PrintTimeToBin(CCommandRunParam *pcm)
{
    LARGE_INTEGER li;
    if( !RtlTimeFieldsToTime(&pcm->tf,&li) )
    {
        printf("Invalid date/time\n");
        return ;
    }
    if( !pcm->UTCMode )
        RtlLocalTimeToSystemTime(&li,&li);

    printf("Absolute System Time : ");
    printf("0x%016I64X\n",li.QuadPart);

    ULONG ElapsedSeconds;

    printf("Elapsed Seconds Since 1980 : ");
    if( RtlTimeToSecondsSince1980(&li,&ElapsedSeconds) )
        printf("0x%08X\n",ElapsedSeconds);
    else
        printf("Invalid range\n");

    printf("Elapsed Seconds Since 1970 : ");
    if( RtlTimeToSecondsSince1970(&li,&ElapsedSeconds) )
        printf("0x%08X\n",ElapsedSeconds);
    else
        printf("Invalid range\n");
}

//----------------------------------------------------------------------------
//
//  PrintBinToTime()
//
//----------------------------------------------------------------------------
VOID PrintBinToTime(CCommandRunParam *pcm)
{
    WCHAR szDate[64];
    WCHAR szTime[64];
    LARGE_INTEGER li;

    if( pcm->BinToTimeMode == 1 )
    {
        RtlSecondsSince1980ToTime(pcm->liValue.LowPart,&li);
    }
    else if( pcm->BinToTimeMode == 2 )
    {
        RtlSecondsSince1970ToTime(pcm->liValue.LowPart,&li);
    }
    else
    {
        li = pcm->liValue;
    }

    if( WinGetDateString(li.QuadPart,szDate,ARRAYSIZE(szDate),pcm->GetDateFormat(),pcm->UTCMode,0) == 0 )
        StringCchCopy(szDate,ARRAYSIZE(szDate),L"<Date Error>");
    if( WinGetTimeString(li.QuadPart,szTime,ARRAYSIZE(szTime),pcm->GetTimeFormat(),pcm->UTCMode,0) == 0 )
        StringCchCopy(szDate,ARRAYSIZE(szDate),L"<Time Error>");
    if( pcm->ShowMilliseconds )
    {
        AddMillisecondsString(li,szTime,ARRAYSIZE(szTime));
    }

    printf("%S %S\n",szDate,szTime);
}

//----------------------------------------------------------------------------
//
//  ParsePrintType()
//
//----------------------------------------------------------------------------
BOOLEAN ParsePrintType(CCommandRunParam *pcm,PCWSTR pszOpt)
{
    WCHAR type[5] = {0};
    BOOLEAN op_check[4] = {0,0,0,0};
    int f;

    int ch = 0;
    while( *pszOpt && ch < 4 )
    {
        f = -1;

        switch( *pszOpt )
        {
            case L'w':
            case L'W': // Last Write
                f = 0;
                break;
            case L'c':
            case L'C': // Creation
                f = 1;
                break;
            case L'a':
            case L'A': // Last Access
                f = 2;
                break;
            case L'h':
            case L'H': // Attribute Change
                f = 3;
                break;
            default:
                return FALSE;
        }

        if( f != -1 )
        {
            if( op_check[f] == FALSE )
            {
                type[ch++] = RtlUpcaseUnicodeChar( *pszOpt++ );
                op_check[f] = TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    pcm->SetPrintType( type );
    
    return TRUE;
}

//----------------------------------------------------------------------------
//
//  ParseQueryActionParameter()
//
//----------------------------------------------------------------------------
/*++

  -u   displays time as UTC.

  -b   displays date/time by hex. ignores -fd,-ft switches.

  -l   displays file date/time by a line.

  -m   displays milliseconds. 

  -s   if included directory in parameters, do recursive directory scan. 

  -t:<type> 
       print type switch:
       w  last write
       c  creation
       a  last access
       h  attribute change
       default: "wcah"

  -fd:<date format>
       specifies format for date display. default: system setting.

  -ft:<time format>
       specifies format for time display. default: "HH:mm:ss".

--*/
BOOLEAN ParseQueryActionParameter(int argc, WCHAR *argv[],CCommandRunParam *pcm)
{
    size_t len;
    int i;
    for(i = 2; i < argc; i++)
    {
        len = wcslen(argv[i]);

        if( argv[i][0] == L'/' || argv[i][0] == L'-' )
        {
            switch( argv[i][1] )
            {
                case L'u':
                case L'U':
                    if( argv[i][2] == L'\0' )
                        pcm->UTCMode = TRUE;
                    else
                        return FALSE; // invalid switch
                    break;

                case L'b':
                case L'B':
                    if( argv[i][2] == L'\0' )
                        pcm->HexDumpMode = TRUE;
                    else
                        return FALSE; // invalid switch
                    break;

                case L'l':
                case L'L':
                    if( argv[i][2] == L'\0' )
                        pcm->LineMode = TRUE;
                    else
                        return FALSE; // invalid switch
                    break;

                case L'm':
                case L'M':
                    if( argv[i][2] == L'\0' )
                        pcm->ShowMilliseconds = TRUE;
                    else
                        return FALSE; // invalid switch
                    break;

                case L't':
                case L'T':
                    if( !(argv[i][2] == L':' && argv[i][3] != L'\0' && ParsePrintType( pcm, &argv[i][3] )) )
                    {
                        return FALSE; // invalid type switch
                    }
                    break;

                case L'f':
                case L'F':
                    if( _wcsnicmp(&argv[i][1],L"fd:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        if( pcm->GetDateFormat() == NULL )
                            pcm->SetDateFormat( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"ft:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        if( pcm->GetTimeFormat() == NULL )
                            pcm->SetTimeFormat( &argv[i][4] );
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L's':
                case L'S':
                    if( argv[i][2] == L'\0' )
                        pcm->Recursive = TRUE;
                    else
                        return FALSE; // invalid switch
                    break;

                default:
                    return FALSE; // invalid switch
            }
        }
        else
        {
            // setup target files
            CFileItem *pfi = new CFileItem;

            pfi->SetFile( argv[i] );

            pcm->FileList.Add( pfi );
        }
    }

    if( pcm->GetPrintType() == NULL )
        pcm->SetPrintType( L"WCAH" );

    if( pcm->GetTimeFormat() == NULL )
        pcm->SetTimeFormat( L"HH:mm:ss" );

    pcm->UpdateDirectory = TRUE;
    pcm->UpdateFile = TRUE;

    return TRUE;
}

//----------------------------------------------------------------------------
//
//  ParseSetActionParameter()
//
//----------------------------------------------------------------------------
/*++

  -dw:<date>  set last write date
  -dc:<date>  set creation date
  -da:<date>  set last access date
  -dh:<date>  set attribute change date

  -dz:<date>  set all date

  -tw:<time>  set last write time
  -tc:<time>  set creation time
  -ta:<time>  set last access time
  -th:<time>  set attribute change time

  -tz:<time>  set all time 

  - or - 

  -bw:<bin>   set absolute systemtime last write date/time
  -bc:<bin>   set absolute systemtime creation date/time
  -ba:<bin>   set absolute systemtime last access date/time
  -bh:<bin>   set absolute systemtime attribute change date/time

  -bz:<bin>   set all absolute systemtime

  options {-d? -t?} and {-?b} is mutually exclusive.

  -u|-utc     set time as UTC.
  -dos|-fat   set time as ms-dos time.
  -y          suppress confirm prompt.
  -r          show result.
  -test       test run mode. enum target files but no update to date/time.
  -f[o]       when enumeration (with the wild card) or recursive mode (with the -s option),
              condition matched also directories are update processed.
              with 'o' option, only for udapte directoies not files.

  input format:
     date > "yyyy-mm-dd"
     time > "hh:mm:ss"

       date and time uses localtime if without specifies -u option.

       NOTE:
       The date format varies from country to country, but we use as default 
       the Japanese format to avoid complications. 

       e.g.
       -wd:"2019-03-01" -wt:"12:34:56"

     bin  > decimal or hex value. use prefix "0x" as hex input.

       The specified value is directly set to file's 64bit date time attribute.
       (ignores -u option)

       e.g.
       input decimal
       -wb:8000000000000000000

       input hex
       -cb:0x1E000000000000000

--*/
BOOLEAN ParseSetActionParameter(int argc, WCHAR *argv[],CCommandRunParam *pcm)
{
    BOOLEAN UpdateDirectory = FALSE;
    BOOLEAN UpdateFile = TRUE;

    pcm->SetLastWriteDate(NULL);
    pcm->SetLastWriteTime(NULL);
    pcm->SetLastWriteBin(NULL);

    pcm->SetCreationDate(NULL);
    pcm->SetCreationTime(NULL);
    pcm->SetCreationBin(NULL);

    pcm->SetLastAccessDate(NULL);
    pcm->SetLastAccessTime(NULL);
    pcm->SetLastAccessBin(NULL);

    pcm->SetAttrChangeDate(NULL);
    pcm->SetAttrChangeTime(NULL);
    pcm->SetAttrChangeBin(NULL);

    BOOLEAN bSuccess;
    int i;
    for(i = 2; i < argc; i++)
    {
        bSuccess = TRUE;

        if( argv[i][0] == L'/' || argv[i][0] == L'-' )
        {
            if( _wcsicmp(&argv[i][1],L"dos") == 0 || _wcsicmp(&argv[i][1],L"fat") == 0 )
            {
                pcm->DosTimeWrite = TRUE;
                continue;
            }
            else if( _wcsicmp(&argv[i][1],L"test") == 0 )
            {
                pcm->TestRunMode = TRUE;
                continue;
            }

            switch( argv[i][1] )
            {
                case L'd':
                case L'D':
                    if( _wcsnicmp(&argv[i][1],L"dw:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastWriteDate( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"dc:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetCreationDate( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"da:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastAccessDate( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"dh:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetAttrChangeDate( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"dz:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = (pcm->SetLastWriteDate( &argv[i][4] ) &&
                                pcm->SetCreationDate( &argv[i][4] ) &&
                                pcm->SetLastAccessDate( &argv[i][4] ) &&
                                pcm->SetAttrChangeDate( &argv[i][4] ));
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L't':
                case L'T':
                    if( _wcsnicmp(&argv[i][1],L"tw:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastWriteTime( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"tc:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetCreationTime( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"ta:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastAccessTime( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"th:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetAttrChangeTime( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"tz:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = (pcm->SetLastWriteTime( &argv[i][4] ) &&
                                pcm->SetCreationTime( &argv[i][4] ) &&
                                pcm->SetLastAccessTime( &argv[i][4] ) &&
                                pcm->SetAttrChangeTime( &argv[i][4] ));
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L'b':
                case L'B':
                    if( _wcsnicmp(&argv[i][1],L"bw:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastWriteBin( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"bc:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetCreationBin( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"ba:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetLastAccessBin( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"bh:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = pcm->SetAttrChangeBin( &argv[i][4] );
                    }
                    else if( _wcsnicmp(&argv[i][1],L"bz:",3) == 0 && argv[i][4] != L'\0' )
                    {
                        bSuccess = (pcm->SetLastWriteBin( &argv[i][4] ) &&
                                pcm->SetCreationBin( &argv[i][4] ) &&
                                pcm->SetLastAccessBin( &argv[i][4] ) &&
                                pcm->SetAttrChangeBin( &argv[i][4] ));
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L'u':
                case L'U':
                    if( argv[i][2] == L'\0' )
                    {
                        pcm->UTCMode = TRUE;
                    }
                    else if( _wcsicmp(&argv[i][1],L"utc") == 0 )
                    {
                        pcm->UTCMode = TRUE;
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L'y':
                case L'Y':
                    if( argv[i][2] == L'\0' )
                    {
                        pcm->SuppressPrompt = TRUE;
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L'r':
                case L'R':
                    if( argv[i][2] == L'\0' )
                    {
                        pcm->ShowResult = TRUE;
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L's':
                case L'S':
                    if( argv[i][2] == L'\0' )
                    {
                        pcm->Recursive = TRUE;
                    }
                    else
                    {
                        return FALSE; // invalid format
                    }
                    break;

                case L'f':
                case L'F':
                    UpdateDirectory = TRUE;
                    if( argv[i][2] == L'o' || argv[i][2] == L'O' )
                    {
                        UpdateFile = FALSE;
                    }
                    else if( argv[i][2] != L'\0' )
                    {
                        return FALSE; // invalid format
                    }
                    break;

                default:
                    return FALSE; // invalid switch
            }

            if( !bSuccess ) // something failed.
                return FALSE;  // invalid specification
        }
        else
        {
            // setup target files
            CFileItem *pfi = new CFileItem;

            pfi->SetFile( argv[i] );

            pcm->FileList.Add( pfi );
        }
    }

    BOOLEAN bEmpty = (
        (isUnSetTimeFields(pcm->tfLastWrite) && isUnsetLargeInteger(pcm->liLastWrite)) &&
        (isUnSetTimeFields(pcm->tfCreation) && isUnsetLargeInteger(pcm->liCreation)) &&
        (isUnSetTimeFields(pcm->tfLastAccess) && isUnsetLargeInteger(pcm->liLastAccess)) &&
        (isUnSetTimeFields(pcm->tfAttrChange) && isUnsetLargeInteger(pcm->liAttrChange))
        );

    if( bEmpty )
        return FALSE;

    // check if parameters are overlapping.
    BOOLEAN bResult = (
        checkTimeFieldsAndLargeInteger( pcm->tfLastWrite, pcm->liLastWrite ) &&
        checkTimeFieldsAndLargeInteger( pcm->tfCreation, pcm->liCreation ) &&
        checkTimeFieldsAndLargeInteger( pcm->tfLastAccess, pcm->liLastAccess ) &&
        checkTimeFieldsAndLargeInteger( pcm->tfAttrChange, pcm->liAttrChange )
        );

    pcm->SetPrintType( L"WCAH" ); // default for result displays.

    pcm->UpdateDirectory = UpdateDirectory;
    pcm->UpdateFile = UpdateFile;

    return bResult;
}

//----------------------------------------------------------------------------
//
//  ParseCopyActionParameter()
//
//----------------------------------------------------------------------------
/*++

  -t:<type> 
        copy date/time selction switch:
         w  last write
         c  creation
         a  last access
         h  attribute change

        if this option not specified, same as "w".
  -f[o] when enumeration (with the wild card) or recursive mode (with the -s option),
        condition matched also directories are update processed.
        with 'o' option, only for udapte directoies not files.
  -a    copy all date/time attributes.
        cannot be used with -t option.
  -s    if specified directory at source, dos recursive directory scan.
  -y    suppress confirm prompt.

--*/
BOOLEAN ParseCopyActionParameter(int argc, WCHAR *argv[],CCommandRunParam *pcm)
{
    BOOLEAN bSuccess = FALSE;
    BOOLEAN bCopyAllDateTime = FALSE;
    BOOLEAN UpdateDirectory = FALSE;
    BOOLEAN UpdateFile = TRUE;
    int i;
    for(i = 2; i < argc; i++)
    {
        bSuccess = TRUE;

        if( argv[i][0] == L'/' || argv[i][0] == L'-' )
        {
            switch( argv[i][1] )
            {
                case L't':
                case L'T':
                    if( !(argv[i][2] == L':' && argv[i][3] != L'\0' && ParsePrintType( pcm, &argv[i][3] )) )
                    {
                        return FALSE; // invalid type switch
                    }
                    break;
                case L'a':
                case L'A':
                    if( argv[i][2] == L'\0' )
                        bCopyAllDateTime = TRUE;
                    else
                        return FALSE; // invalid format
                    break;
                case L's':
                case L'S':
                    if( argv[i][2] == L'\0' )
                        pcm->Recursive = TRUE;
                    else
                        return FALSE; // invalid format
                    break;
                case L'y':
                case L'Y':
                    if( argv[i][2] == L'\0' )
                        pcm->SuppressPrompt = TRUE;
                    else
                        return FALSE; // invalid format
                    break;
                case L'f':
                case L'F':
                    UpdateDirectory = TRUE;
                    if( argv[i][2] == L'o' || argv[i][2] == L'O' )
                        UpdateFile = FALSE;
                    else if( argv[i][2] != L'\0' )
                        return FALSE; // invalid format
                    break;
                default:
                    return FALSE; // invalid switch
            }

            if( !bSuccess ) // something failed.
                return FALSE;  // invalid specification
        }
        else
        {
            CFileItem *pfi = new CFileItem;

            pfi->SetFile( argv[i] );

            pcm->FileList.Add( pfi );
        }
    }

    if( pcm->FileList.GetCount() != 2 )
    {
        // not enough parameters. we must need source and destination at least.
        return FALSE;
    }

    if( bCopyAllDateTime == TRUE )
    {
        if( pcm->GetPrintType() != NULL )
            return FALSE; // error: overlapping options -a and -t:.
        pcm->SetPrintType( L"WCAH" ); // all copy.
    }
    else
    {
        if( pcm->GetPrintType() == NULL )
            pcm->SetPrintType( L"W" ); // copy default is LastWrite only.
    }

    pcm->UpdateDirectory = UpdateDirectory;
    pcm->UpdateFile = UpdateFile;

    return bSuccess;
}

//----------------------------------------------------------------------------
//
//  ParseTimeToBinActionParameter()
//
//----------------------------------------------------------------------------
/*++

    fsfiletime timetobin <date> [<time>] [/u] [-fd:<date format>] [-ft:<time format>]

    -u   displays time as UTC.

    -fd:<date format>
         specifies format for date display. default: system setting.

    -ft:<time format>
         specifies format for time display. default: system setting.

--*/
BOOLEAN ParseTimeToBinActionParameter(int argc, WCHAR *argv[],CCommandRunParam *pcm)
{
    int i;

    // argv[0] exe path
    // argv[1] command
    // argv[2]...[argc-1] parameters/options

    // minimum parameters count is 3 (fsfiletime timetobin date-string time-string)
    if( argc < 3 )
    {
        return FALSE;
    }

    // option
    BOOLEAN bResult = FALSE;
    int option = -1;
    int r,year,month,day,hour,minute,second,msecond;

    pcm->tf.Year = pcm->tf.Month = pcm->tf.Day = -1;
    pcm->tf.Hour = pcm->tf.Minute = pcm->tf.Second = pcm->tf.Milliseconds = 0;

    year = month = day = hour = minute = second = msecond = -1;

    __try
    {
        for(i = 2; i < argc; i++)
        {
            if( argv[i][0] == L'/' || argv[i][0] == L'-' )
            {
                switch( argv[i][1] )
                {
                    case L'u':
                    case L'U':
                        if( argv[i][2] == L'\0' )
                        {
                            pcm->UTCMode = TRUE;
                        }
                        else if( _wcsicmp(&argv[i][1],L"utc") == 0 )
                        {
                            pcm->UTCMode = TRUE;
                        }
                        else
                        {
                            __leave; // invalid format
                        }
                        break;
                    default:
                        __leave; // invalid switch
                }
            }
            else
            {
                if( iswdigit(argv[i][0]) )
                {
                    if( wcschr(argv[i],L'-') && year == -1 )
                    {
                        // date format string 
                        r = swscanf_s(argv[i],L"%d-%d-%d",&year,&month,&day);

                        if( r != 3 || (year == -1|| month == -1 || day == -1) )
                        {
                            __leave;
                        }

                        if( !checkValidDate(year,month,day) )
                        {
                            __leave;
                        }

                        pcm->tf.Year = (CSHORT)year;
                        pcm->tf.Month = (CSHORT)month;
                        pcm->tf.Day = (CSHORT)day;
                    }
                    else if( wcschr(argv[i],L':') && hour == -1 )
                    {
                        // time format string
                        if( wcschr(argv[i],L'.') != NULL )
                        {
                            r = swscanf_s(argv[i],L"%d:%d:%d.%d",&hour,&minute,&second,&msecond);
                            if( r != 4 || (hour == -1 || minute == -1 || second == -1 || msecond == -1) )
                            {
                                __leave;
                            }
                        }
                        else
                        {
                            r = swscanf_s(argv[i],L"%d:%d:%d",&hour,&minute,&second);
                            if( r != 3 || (hour == -1 || minute == -1 || second == -1) )
                            {
                                __leave;
                            }
                            msecond = 0;
                        }

                        if( !checkValidTime(hour,minute,second,msecond) )
                        {
                            __leave;
                        }
    
                        pcm->tf.Hour = (CSHORT)hour;
                        pcm->tf.Minute = (CSHORT)minute;
                        pcm->tf.Second = (CSHORT)second;
                        pcm->tf.Milliseconds = (CSHORT)msecond;
                    }
                    else
                    {
                        __leave;
                    }
                }
                else if( pcm->IsMacroText(argv[i]) )
                {
                    if( year == -1 )
                    {
                        pcm->tf.Year = year = pcm->CurrentLocalFields.Year;
                        pcm->tf.Month = month = pcm->CurrentLocalFields.Month;
                        pcm->tf.Day = pcm->CurrentLocalFields.Day;
                    }
                    if( hour == -1 )
                    {
                        pcm->tf.Hour = pcm->CurrentLocalFields.Hour;
                        pcm->tf.Minute = pcm->CurrentLocalFields.Minute;
                        pcm->tf.Second = pcm->CurrentLocalFields.Second;
                        pcm->tf.Milliseconds = pcm->CurrentLocalFields.Milliseconds;
                    }
                }
                else
                {
                    __leave;
                }
            }
        }

        // date is must required 
        if( pcm->tf.Year == -1 )
            __leave;

        if( option == -1 )
            option = 0; // default is windows mode

        pcm->TimeToBinMode = option;

        bResult = TRUE;
    }
    __finally
    {
        ;
    }

    return bResult;
}

//----------------------------------------------------------------------------
//
//  ParseBinToTimeActionParameter()
//
//----------------------------------------------------------------------------
/*++

    fsfiletime bintotime <systemtime> [/dos|/unix]

    <systemtime> specifies system time of UTC. 
                 (same as format of saved to file system)
                 with the prefix "0x" is a hexadecimal number, otherwise a decimal number.

--*/
BOOLEAN ParseBinToTimeActionParameter(int argc, WCHAR *argv[],CCommandRunParam *pcm)
{
    size_t len;
    int i;

    // argv[0] exe path
    // argv[1] command
    // argv[2]...[argc-1] parameters/options

    // minimum parameters count is 3.
    if( argc < 3 )
    {
        return FALSE;
    }

    // option
    BOOLEAN bResult = FALSE;
    LARGE_INTEGER li = { -1, -1 };
    int option = -1;

    __try
    {
        for(i = 2; i < argc; i++)
        {
            len = wcslen(argv[i]);

            if( argv[i][0] == L'/' || argv[i][0] == L'-' )
            {
                switch( argv[i][1] )
                {
                    case L'd':
                    case L'D':
                        if( _wcsicmp(&argv[i][1],L"dos") == 0  && option == -1 )
                        {
                            option = 1; // output is ms-dos time
                        }
                        else
                        {
                            __leave; // invalid format
                        }
                        break;

                    case L'u':
                    case L'U':
                        if( argv[i][2] == L'\0' || (_wcsicmp(&argv[i][1],L"utc") == 0 && option == -1) )
                        {
                            pcm->UTCMode = TRUE; // output time as UTC
                        }
                        else if( _wcsicmp(&argv[i][1],L"unix") == 0 && option == -1 )
                        {
                            option = 2; // output is unix time
                        }
                        else
                        {
                            __leave; // invalid format
                        }
                        break;

                    case L'm':
                    case L'M':
                        pcm->ShowMilliseconds = TRUE;
                        break;

                    case L'f':
                    case L'F':
                        if( _wcsnicmp(&argv[i][1],L"fd:",3) == 0 && argv[i][4] != L'\0' )
                        {
                            if( pcm->GetDateFormat() == NULL )
                                pcm->SetDateFormat( &argv[i][4] );
                        }
                        else if( _wcsnicmp(&argv[i][1],L"ft:",3) == 0 && argv[i][4] != L'\0' )
                        {
                            if( pcm->GetTimeFormat() == NULL )
                                pcm->SetTimeFormat( &argv[i][4] );
                        }
                        else
                        {
                            __leave; // invalid format
                        }
                        break;

                    default:
                        __leave; // invalid switch
                }
            }
            else
            {
                if( li.QuadPart == -1 && iswdigit(argv[i][0]) )
                {
                    PWSTR psz = argv[i];
                    int radix = 10;
                    if( psz[0] == L'0' && (psz[1] == L'x' || psz[1] == L'X') )
                        radix = 16;
                    li.QuadPart = _wcstoi64(psz,NULL,radix);
                }
                else
                {
                    __leave; // invalid switch
                }
            }
        }

        if( option == -1 )
            option = 0; // default is windows mode

        if( li.QuadPart == -1 )
            li.QuadPart = 0; // specifiles the origin

        if( option != 0 && li.HighPart != 0 )
            __leave; // dos,unix time use only LowPart (because must in ULONG)

        pcm->BinToTimeMode = option;
        pcm->liValue = li;

        bResult = TRUE;
    }
    __finally
    {
        ;
    }

    return bResult;
}

//----------------------------------------------------------------------------
//
//  Cleanup()
//
//----------------------------------------------------------------------------
void Cleanup(CCommandRunParam *pcm)
{
    int i,c = pcm->FileList.GetCount();
    for(i = 0; i < c; i++)
    {
        CFileItem *pi = pcm->FileList[i];
        delete pi;
    }
}

//----------------------------------------------------------------------------
//
//  usage()
//
//----------------------------------------------------------------------------
int PrintError( LONG ErrorCode, PCWSTR pszParam )
{
    switch( ErrorCode )
    {
        case 0:
            break;
        case STATUS_NO_SUCH_FILE:
            // The file %hs does not exist.
            if( pszParam )
                printf("The file '%ls' does not exist.\n",pszParam);
            else
                printf("The file does not exist.\n");
            break;
        case STATUS_NOT_A_DIRECTORY:
            if( pszParam )
                printf("The file '%ls' is not directory.\n",pszParam);
            else
                printf("The specifies path is not directory.\n");
            break;
        case STATUS_INVALID_PARAMETER:
            // An invalid combination of parameters was specified.
            printf("The parameter is incorrect.\n");
            break;
        default:
        {
            ULONG dosError;
            if( (ErrorCode & 0xC0000000) == 0xC0000000 )
                dosError = RtlNtStatusToDosError(ErrorCode);
            else
                dosError = ErrorCode;

            PWSTR pMessage = NULL;
            WinGetSystemErrorMessage(dosError,&pMessage);
            if( pMessage )
            {
                PWSTR p = wcspbrk(pMessage,L"\r\n");
                if( p )
                    *p = L'\0';
            }

            printf(
                ((ErrorCode & 0xC0000000) == 0xC0000000) ? "%S (0x%08X)\n" : "%S (%d)\n",
                pMessage != NULL ? pMessage : L"Unknown error" ,
                ErrorCode);

            WinFreeErrorMessage(pMessage);
            break;
        }
    }

    return (int)RtlNtStatusToDosError(ErrorCode);
}

//----------------------------------------------------------------------------
//
//  Usage()
//
//----------------------------------------------------------------------------
void Usage()
{
    printf(
        "usage:\n"
        "   fsfiletime command [parameters]\n"
        "\n"
        "command:\n"
        "   query\n"
        "   set\n"
        "   copy\n"
        "   timetobin\n"
        "   bintotime\n"
        "   help\n"
        "\n"
        "fsfiletime query [options] filename\n"
        "\n"
        "fsfiletime set {date|time|bin} [options] filename\n"
        "\n"
        "fsfiletime copy [options] source destination\n"
        "\n"
        "fsfiletime timetobin [options] date [time]\n"
        "\n"
        "fsfiletime bintotime [options] absolute_systemtime_value\n"
        "\n"
        "fsfiletime help\n"
        );
}

//
// SourcePath's Specification:
//
// \??\c:\windows\file   ... Only target single file as "\??\c:\windows\file" (not directory).
// \??\c:\windows\*      ... Enumerate files "*" under the "\??\c:\windows". 
//                           If specified -s option, include sub-directory.
// \??\c:\windows\*.exe  ... Enumerate files "*.exe" under the "\??\c:\windows". 
//                           If specified -s option, include sub-directory.
// \??\c:\windows\       ... Same as "\??\c:\windows\*". 
// \??\c:\windows        ... The directory "\??\c:\windows" itself is processed.  Not directory contents.
//
// If the end of the path is '\', it is determined that the contents of the directory
// are specified for processing, and FileName = L "*" is set.
// in short 
//      DirPath  = "\??\C:\Temp\"
//      FileName = "*"
// will be.
//

//----------------------------------------------------------------------------
//
//  wmain()
//
//----------------------------------------------------------------------------
int __cdecl wmain(int argc, WCHAR* argv[])
{
    setlocale(LC_ALL,"");

    if( argc < 2 )
    {
        Usage();
        return 0;
    }

    CCommandRunParam cmd;

    cmd.InitCurrentTime();

    PWSTR cmdstr = argv[1];

    if( _wcsicmp(cmdstr,L"query") == 0 )
    {
        if( !ParseQueryActionParameter(argc,argv,&cmd) || cmd.FileList.GetCount() == 0 )
        {
            return PrintError( STATUS_INVALID_PARAMETER );
        }
        cmd.action = Query;
    }
    else if( _wcsicmp(cmdstr,L"set") == 0 )
    {
        cmd.action = Set;
        if( !ParseSetActionParameter(argc,argv,&cmd) || cmd.FileList.GetCount() == 0 )
        {
            return PrintError( STATUS_INVALID_PARAMETER );
        }
    }
    else if( _wcsicmp(cmdstr,L"copy") == 0 )
    {
        cmd.action = Copy;
        if( !ParseCopyActionParameter(argc,argv,&cmd) || cmd.FileList.GetCount() == 0 )
        {
            return PrintError( STATUS_INVALID_PARAMETER );
        }
    }
    else if( _wcsicmp(cmdstr,L"timetobin") == 0 )
    {
        cmd.action = TimeToBin;
        if( !ParseTimeToBinActionParameter(argc,argv,&cmd) )
        {
            return PrintError( STATUS_INVALID_PARAMETER );
        }
    }
    else if( _wcsicmp(cmdstr,L"bintotime") == 0 )
    {
        cmd.action = BinToTime;
        if( !ParseBinToTimeActionParameter(argc,argv,&cmd) )
        {
            return PrintError( STATUS_INVALID_PARAMETER );
        }
    }
    else if( _wcsicmp(cmdstr,L"help") == 0 )
    {
        cmd.action = Help;
    }
    else
    {
        Usage();
        return RtlNtStatusToDosError( STATUS_INVALID_PARAMETER );
    }

    //
    // Process Command
    //

    if( cmd.action == Query || cmd.action == Set )
    {
        CommandProcessFiles(cmd);
    }
    if( cmd.action == Copy )
    {
        CommandCopy(cmd);
    }
    else if( cmd.action == TimeToBin )
    {
        PrintTimeToBin(&cmd);
    }
    else if( cmd.action == BinToTime )
    {
        PrintBinToTime(&cmd);
    }
    else if( cmd.action == Help )
    {
        PrintHelp();
    }

    Cleanup(&cmd);

    return 0;
}
