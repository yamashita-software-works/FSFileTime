#pragma once

#include "ntnativeapi.h"
#include "ntnativehelp.h"
#include "win32helper.h"

#include "fileitem.h"

typedef struct _COMMAND_RUN_PATH
{
    UNICODE_STRING FullPath;
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeDirPart;
    BOOLEAN EnumDirectoryFiles;
} COMMAND_RUN_PATH;

class CCommandRunPath : public COMMAND_RUN_PATH
{
public:
    CCommandRunPath()
    {
        RtlZeroMemory(&FullPath,sizeof(FullPath));
        RtlZeroMemory(&FileName,sizeof(FileName));
        RtlZeroMemory(&RelativeDirPart,sizeof(RelativeDirPart));
        EnumDirectoryFiles = false;
    }
    ~CCommandRunPath()
    {
        RtlReleaseRelativeName(&RelativeDirPart);
        RtlFreeUnicodeString(&FullPath);
        RtlFreeUnicodeString(&FileName);
    }
};

typedef enum
{
    Query,
    Set,
    Copy,
    TimeToBin,
    BinToTime,
    Help,
} COMMANDSACTION;

typedef struct _COMMAND_RUN_PARAM
{
    COMMANDSACTION action;

    CFileList FileList;

    BOOLEAN UTCMode;
    BOOLEAN Recursive;
    BOOLEAN UpdateDirectory;
    BOOLEAN UpdateFile;

    LARGE_INTEGER CurrentSystemTime;
    LARGE_INTEGER CurrentLocalTime;
    TIME_FIELDS CurrentSystemTimeFields;
    TIME_FIELDS CurrentLocalFields;

    struct {
        // QUERY command parameter (and SET command result display)
        PWSTR pszPrintType;  // default: "WCAH"
        PWSTR pszDateFormat; // default: NULL (system dependency)
        PWSTR pszTimeFormat; // default: "HH:mm:ss"

        BOOLEAN LineMode;
        BOOLEAN HexDumpMode;
        BOOLEAN ShowMilliseconds;
    };

    struct {
        // SET command parameter
        TIME_FIELDS tfLastWrite;
        TIME_FIELDS tfCreation;
        TIME_FIELDS tfLastAccess;
        TIME_FIELDS tfAttrChange;
        LARGE_INTEGER liLastWrite;
        LARGE_INTEGER liCreation;
        LARGE_INTEGER liLastAccess;
        LARGE_INTEGER liAttrChange;
        BOOLEAN DosTimeWrite;
        BOOLEAN SuppressPrompt;
        BOOLEAN TestRunMode;
        BOOLEAN ShowResult;
    };

    struct {
        // TIMETOBIN command parameter
        int TimeToBinMode; // 0:nt/windows, 1:dos, 2:unix
        TIME_FIELDS tf;
    };

    struct {
        // BINTOTIME command parameter
        int BinToTimeMode; // 0:nt/windows, 1:dos, 2:unix
        LARGE_INTEGER liValue;
    };

} COMMAND_RUN_PARAM;

__forceinline BOOLEAN isUnSetTimeFieldsDate(TIME_FIELDS& tf)
{
    return ( tf.Year == -1 && tf.Month == -1 && tf.Day == -1 );
}

__forceinline BOOLEAN isUnSetTimeFieldsTime(TIME_FIELDS& tf)
{
    return ( tf.Hour == -1 && tf.Minute == -1 && tf.Second == -1 ); 
}

__forceinline BOOLEAN isUnSetTimeFields(TIME_FIELDS& tf)
{
    return ( isUnSetTimeFieldsDate(tf) &&
             isUnSetTimeFieldsTime(tf) ); 
}

__forceinline BOOLEAN isUnsetLargeInteger(LARGE_INTEGER& li)
{
    return ( li.QuadPart == -1 );
}

__forceinline BOOLEAN checkTimeFieldsAndLargeInteger(TIME_FIELDS& tf,LARGE_INTEGER& li)
{
    if( !isUnSetTimeFields(tf) && !isUnsetLargeInteger(li) )
        return false;
    return true;
}

__forceinline BOOLEAN checkValidDate(int year,int month,int day)
{
    // NOTE: TODO:
    // - We not check the year range.
    // - We not check the leap year.
    static char md[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if( 1 <= month && month <= 12 )
    {
        if( 1 <= day && day <= md[month] )
        {
            return TRUE;
        }
    }
    return FALSE;
}

__forceinline BOOLEAN checkValidTime(int hour,int minute,int second,int msecond=-1)
{
    if( msecond != -1 && !(0 <= msecond && msecond < 1000) )
        return FALSE;
    return ( 0 <= hour && hour <= 23 ) &&
           ( 0 <= minute && minute <= 59 ) &&
           ( 0 <= second && second <= 59 );
}

struct CCommandRunParam : public COMMAND_RUN_PARAM
{
    CCommandRunParam()
    {
        RtlZeroMemory(this,sizeof(COMMAND_RUN_PARAM));
    }

    ~CCommandRunParam()
    {
    }

    //
    // Print format setter
    //
    void SetPrintType(PCWSTR psz)
    {
        if( pszPrintType != NULL )
            free(pszPrintType);
        pszPrintType = _wcsdup(psz);
    }

    PCWSTR GetPrintType() const
    {
         return pszPrintType;
    }

    void SetDateFormat(PCWSTR psz)
    {
        if( pszDateFormat != NULL )
            free(pszDateFormat);
        pszDateFormat = _wcsdup(psz);
    }

    PCWSTR GetDateFormat() const
    {
         return pszDateFormat;
    }

    void SetTimeFormat(PCWSTR psz)
    {
        if( pszTimeFormat != NULL )
            free(pszTimeFormat);
        pszTimeFormat = _wcsdup(psz);
    }

    PCWSTR GetTimeFormat() const
    {
         return pszTimeFormat;
    }

    //
    // Last Write Date/Time
    //
    BOOLEAN SetLastWriteDate(PCWSTR psz)
    {
        return getDate(psz,tfLastWrite);
    }

    BOOLEAN SetLastWriteTime(PWSTR psz)
    {
        return getTime(psz,tfLastWrite);
    }

    BOOLEAN SetLastWriteBin(PWSTR psz)
    {
        return getBinData(psz,liLastWrite);
    }

    //
    // Creation Date/Time
    //
    BOOLEAN SetCreationDate(PCWSTR psz)
    {
        return getDate(psz,tfCreation);
    }

    BOOLEAN SetCreationTime(PWSTR psz)
    {
        return getTime(psz,tfCreation);
    }

    BOOLEAN SetCreationBin(PWSTR psz)
    {
        return getBinData(psz,liCreation);
    }

    //
    // Last Access Date/Time
    //
    BOOLEAN SetLastAccessDate(PCWSTR psz)
    {
        return getDate(psz,tfLastAccess);
    }

    BOOLEAN SetLastAccessTime(PWSTR psz)
    {
        return getTime(psz,tfLastAccess);
    }

    BOOLEAN SetLastAccessBin(PWSTR psz)
    {
        return getBinData(psz,liLastAccess);
    }

    //
    // Attribute Change Date/Time
    //
    BOOLEAN SetAttrChangeDate(PCWSTR psz)
    {
        return getDate(psz,tfAttrChange);
    }

    BOOLEAN SetAttrChangeTime(PWSTR psz)
    {
        return getTime(psz,tfAttrChange);
    }

    BOOLEAN SetAttrChangeBin(PWSTR psz)
    {
        return getBinData(psz,liAttrChange);
    }

    //
    // miscellaneous
    //
    VOID InitCurrentTime()
    {
        NtQuerySystemTime(&CurrentSystemTime);
        RtlSystemTimeToLocalTime(&CurrentSystemTime,&CurrentLocalTime);

        RtlTimeToTimeFields(&CurrentSystemTime,&CurrentSystemTimeFields);
        RtlTimeToTimeFields(&CurrentLocalTime,&CurrentLocalFields);
    }

    int IsMacroText(PCWSTR psz)
    {
        if( _wcsicmp(psz,L"@now") == 0 )
            return 1;
        return 0; // no macro pattern
    }

private:
    BOOLEAN getDate(PCWSTR psz,TIME_FIELDS& tf)
    {
        if( psz && !isUnSetTimeFieldsDate(tf) )
            return FALSE; // already value setted.

        int r,y,m,d;
        y = m = d = -1;

        if( psz != NULL )
        {
            int ptn = IsMacroText(psz);
            if( ptn == 1 )
            {
                getCurDate(&y,&m,&d);
            }
            else
            {
                r = swscanf_s(psz,L"%d-%d-%d",&y,&m,&d);
                if( r != 3 || (y == -1 || m == -1 || d == -1) )
                    return false;

                if( !checkValidDate(y,m,d) )
                    return false;
            }
        }

        tf.Year = (CSHORT)y;
        tf.Month = (CSHORT)m;
        tf.Day = (CSHORT)d;
        return true;
    }

    BOOLEAN getTime(PCWSTR psz,TIME_FIELDS& tf)
    {
        if( psz && !isUnSetTimeFieldsTime(tf) )
            return FALSE; // already value setted.

        int r,h,m,s,ms;
        h = m = s = ms = -1;

        if( psz != NULL )
        {
            int ptn = IsMacroText(psz);
            if( ptn == 1 )
            {
                getCurTime(&h,&m,&s,&ms);
            }
            else
            {
                if( wcschr(psz,L'.') != NULL )
                {
                    r = swscanf_s(psz,L"%d:%d:%d.%d",&h,&m,&s,&ms);
                    if( r != 4 || (h == -1 || m == -1 || s == -1 || ms != -1) )
                        return false;
                }
                else
                {
                    ms = 0;
                    r = swscanf_s(psz,L"%d:%d:%d",&h,&m,&s);
                    if( r != 3 || (h == -1 || m == -1 || s == -1) )
                        return false;
                }
                if( !checkValidTime(h,m,s,ms) )
                    return false;
            }
        }

        tf.Hour = (CSHORT)h;
        tf.Minute = (CSHORT)m;
        tf.Second = (CSHORT)s;
        tf.Milliseconds = (CSHORT)ms;

        return true;
    }

    BOOLEAN getBinData(PCWSTR psz,LARGE_INTEGER& li)
    {
        if( psz && !isUnsetLargeInteger(li) )
            return FALSE;  // already value setted.

        if( psz != NULL )
        {
            int ptn = IsMacroText(psz);
            if( ptn == 1 )
            {
                li.QuadPart = CurrentSystemTime.QuadPart;
            }
            else
            {
                int radix = 10;
                if( psz[0] == L'0' && (psz[1] == L'x' || psz[1] == L'X') )
                    radix = 16;
                li.QuadPart = _wcstoi64(psz,NULL,radix);
            }
        }
        else
        {
            li.QuadPart = -1;
        }

        return true;
    }

    VOID getCurDate(int *py,int *pm,int *pd)
    {
        *py = CurrentLocalFields.Year;
        *pm = CurrentLocalFields.Month;
        *pd = CurrentLocalFields.Day;
    }

    VOID getCurTime(int *ph,int *pm,int *ps,int *pms)
    {
        *ph = CurrentLocalFields.Hour;
        *pm = CurrentLocalFields.Minute;
        *ps = CurrentLocalFields.Second;
        *pms = CurrentLocalFields.Milliseconds;
    }
};

extern int PrintError( LONG ErrorCode, PCWSTR pszParam = NULL );
extern void PrintHelp();
extern HRESULT CommandCopy(CCommandRunParam& cmd);
extern BOOLEAN DeterminePathType(CCommandRunParam *pcm,FILEITEM& fi,COMMAND_RUN_PATH *prunpath);
extern VOID UpdateFileDateTime(CCommandRunParam *pcm, UNICODE_STRING *FileNameWithFullPath,FILE_BASIC_INFORMATION *pBasicInfo);
extern VOID SetFileDateTime(CCommandRunParam *pcm, FILE_BASIC_INFORMATION *pfbi);
