#pragma once

PVOID AllocMemory(SIZE_T cb);
PVOID ReAllocateHeap(PVOID pv,SIZE_T cb);
VOID FreeMemory(PVOID ptr);
NTSTATUS AllocateUnicodeString(UNICODE_STRING *pus,PCWSTR psz);
PWSTR AllocateSzFromUnicodeString(UNICODE_STRING *pus);
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
VOID RemoveBackslash(PWSTR pszPath);
PWSTR CombinePath_U(PCWSTR pszPath,UNICODE_STRING *pusFileName);

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
    BOOLEAN bRecursive,
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
