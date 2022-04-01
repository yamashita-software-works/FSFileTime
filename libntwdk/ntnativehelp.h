#pragma once

PVOID AllocMemory(SIZE_T cb);
PVOID ReAllocateHeap(PVOID pv,SIZE_T cb);
VOID FreeMemory(PVOID ptr);
NTSTATUS AllocateUnicodeString(UNICODE_STRING *pus,PCWSTR psz);
NTSTATUS DuplicateUnicodeString(UNICODE_STRING *pusDup,UNICODE_STRING *pusSrc);
PWSTR AllocateSzFromUnicodeString(UNICODE_STRING *pus);
WCHAR *AllocStringBuffer(ULONG cch);
BOOLEAN IsNtDevicePath(PCWSTR pszPath);
BOOLEAN HasPrefix(PCWSTR pszPrefix,PCWSTR pszPath);
BOOLEAN HasPrefix_U(PCWSTR pszPrefix,UNICODE_STRING *String);
BOOLEAN HasWildCardChar_U(UNICODE_STRING *String);
NTSTATUS GetFileNamePart_U(UNICODE_STRING *FilePath,UNICODE_STRING *FileName);
NTSTATUS SplitPathFileName_U(UNICODE_STRING *Path,UNICODE_STRING *FileName);
BOOLEAN IsRelativePath(PCWSTR pszPath);
BOOLEAN NtPathFileExists_U(UNICODE_STRING *pusPath,ULONG *FileAttributes);
BOOLEAN IsDirectory(PCWSTR pszPath);
BOOLEAN IsDirectory_U(UNICODE_STRING *pusPath);
BOOLEAN IsRootDirectory_U(UNICODE_STRING *pusFullyPath);
BOOLEAN IsLastCharacterBackslash(PWSTR pszPath);
BOOLEAN IsLastCharacterBackslash_U(UNICODE_STRING *pusPath);
VOID RemoveBackslash(PWSTR pszPath);
VOID RemoveBackslash_U(UNICODE_STRING *pusPath);
PWSTR CombinePath(PCWSTR pszPath,PCWSTR pszFileName);
PWSTR CombinePath_U(PCWSTR pszPath,UNICODE_STRING *pusFileName);
NTSTATUS CombineUnicodeStringPath(UNICODE_STRING *CombinedPath,UNICODE_STRING *Path,UNICODE_STRING *FileName);

BOOLEAN _UStrMatchI(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatchI_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch_U(const WCHAR *ptn,const UNICODE_STRING *pus);
BOOLEAN _UStrMatchI_U(const WCHAR *ptn,const UNICODE_STRING *pus);

NTSTATUS GetFileDateTime( UNICODE_STRING *FilePath, FILE_BASIC_INFORMATION *pbi );

//
// Enum Files with Traverse Directory
//
typedef
NTSTATUS
(CALLBACK *FINDFILECALLBACK)(
    ULONG CallbackReason,
    PCWSTR Path,
    PCWSTR RelativePath,
    UNICODE_STRING *FileName,
    NTSTATUS Status,
    ULONG FileInfoType,  // Reserved always zero
    PVOID FileInfo,      // FILE_ID_BOTH_DIR_INFORMATION
    ULONG_PTR CallbackContext
    );

// Callback reason
#define FFCBR_FINDFILE         0
#define FFCBR_DIRECTORYSTART   1
#define FFCBR_DIRECTORYEND     2
#define FFCBR_ERROR            3

NTSTATUS
TraverseDirectory(
    UNICODE_STRING& DirectoryFullPath,
    UNICODE_STRING& FileName,
    BOOLEAN bRecursive,
    ULONG Flags,
    FINDFILECALLBACK pfnCallback,
    ULONG_PTR CallbackContext
    );

//
// Enum Files (without recursive subdirectory traverse)
//
typedef
BOOLEAN
(CALLBACK *ENUMFILESCALLBACK)(
    HANDLE hDirectory,
    PCWSTR DirectoryName,
    FILE_ID_BOTH_DIR_INFORMATION *pFileInfo,
    ULONG_PTR CallbackContext
    );

NTSTATUS
EnumFiles(
    HANDLE hRoot,
    PCWSTR pszDirectoryPath,
    PCWSTR pszFileName,
    ENUMFILESCALLBACK pfnCallback,
    ULONG_PTR CallbackContext
    );
