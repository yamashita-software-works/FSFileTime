#pragma once
//
// ntddk/ntifs helper function library
//
#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif
PVOID NTAPI AllocMemory(SIZE_T cb);
PVOID NTAPI ReAllocateHeap(PVOID pv,SIZE_T cb);
#define ReallocMemory(pv,cb) ReAllocateHeap(pv,cb)
VOID  NTAPI FreeMemory(PVOID ptr);
NTSTATUS NTAPI AllocateUnicodeString(UNICODE_STRING *pus,PCWSTR psz);
NTSTATUS NTAPI DuplicateUnicodeString(UNICODE_STRING *pusDup,UNICODE_STRING *pusSrc);
NTSTATUS NTAPI AllocateUnicodeStringCbBuffer(UNICODE_STRING *pus,ULONG cb);
NTSTATUS NTAPI AllocateUnicodeStringCchBuffer(UNICODE_STRING *pus,ULONG cch);
PWSTR NTAPI AllocateSzFromUnicodeString(UNICODE_STRING *pus);
WCHAR* NTAPI AllocStringBuffer(SIZE_T cch);
WCHAR* NTAPI AllocStringBufferCb(SIZE_T cb);
PWSTR NTAPI AllocStringLengthCb(PCWSTR psz,SIZE_T cb);
PWSTR NTAPI DuplicateString(PCWSTR psz);
BOOLEAN IsNtDevicePath(PCWSTR pszPath);
BOOLEAN HasPrefix(PCWSTR pszPrefix,PCWSTR pszPath);
BOOLEAN HasPrefix_U(PCWSTR pszPrefix,UNICODE_STRING *String);
BOOLEAN HasWildCardChar_U(UNICODE_STRING *String);
NTSTATUS GetFileNamePart_U(UNICODE_STRING *FilePath,UNICODE_STRING *FileName);
NTSTATUS SplitPathFileName_U(UNICODE_STRING *Path,UNICODE_STRING *FileName);
BOOLEAN SplitRootRelativePath(PCWSTR pszFullPath,UNICODE_STRING *RootDirectory,UNICODE_STRING *RootRelativePath);
BOOLEAN IsRelativePath(PCWSTR pszPath);
BOOLEAN PathFileExists_U(UNICODE_STRING *pusPath,ULONG *FileAttributes);
BOOLEAN PathFileExists_UEx(HANDLE hParentDir,UNICODE_STRING *pusPath,ULONG *FileAttributes);
BOOLEAN PathFileExists_W(PCWSTR pszPath,ULONG *FileAttributes);
BOOLEAN IsDirectory(PCWSTR pszPath);
BOOLEAN IsDirectory_U(UNICODE_STRING *pusPath);
BOOLEAN IsRootDirectory_U(UNICODE_STRING *pusFullyPath);
BOOLEAN IsLastCharacterBackslash(PCWSTR pszPath);
BOOLEAN IsLastCharacterBackslash_U(UNICODE_STRING *pusPath);
BOOLEAN GetRootDirectory_U(UNICODE_STRING *pusFullyQualifiedPath);
BOOLEAN GetVolumeName_U(UNICODE_STRING *pusFullyQualifiedPath);
NTSTATUS FindRootDirectory_U(UNICODE_STRING *pusFullyQualifiedPath,PWSTR *pRootDirectory);
VOID RemoveBackslash(PWSTR pszPath);
VOID RemoveBackslash_U(UNICODE_STRING *pusPath);
VOID RemoveFileSpec(PWSTR pszPath);
PWSTR CombinePath(PCWSTR pszPath,PCWSTR pszFileName);
PWSTR CombinePath_U(PCWSTR pszPath,UNICODE_STRING *pusFileName);
NTSTATUS CombineUnicodeStringPath(UNICODE_STRING *CombinedPath,UNICODE_STRING *Path,UNICODE_STRING *FileName);
PWSTR DosPathNameToNtPathName(PCWSTR pszDosPath);

#ifdef _NTIFS_
NTSTATUS GetFileDateTime_U( UNICODE_STRING *FilePath, FILE_BASIC_INFORMATION *pbi );
#endif

VOID NTAPI _SetLastStatusDos(NTSTATUS ntStatus);
VOID NTAPI _SetLastWin32Error(ULONG Win32ErrorCode);
VOID NTAPI _SetLastNtStatus(NTSTATUS ntStatus);

BOOLEAN _UStrMatchI(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatchI_UStr(const WCHAR *ptn,const WCHAR *str,const WCHAR *end);
BOOLEAN _UStrMatch_U(const WCHAR *ptn,const UNICODE_STRING *pus);
BOOLEAN _UStrMatchI_U(const WCHAR *ptn,const UNICODE_STRING *pus);

HRESULT GetNtPath(PCWSTR DosPathName,PWSTR *NtPath,PCWSTR *NtFileNamePart);
HRESULT GetNtPath_U(PCWSTR DosPathName,UNICODE_STRING *NtPath,PCWSTR *NtFileNamePart);

#ifdef __cplusplus
}
#endif

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

typedef
BOOLEAN
(CALLBACK *ENUMFILESCALLBACK)(
    HANDLE hDirectory,
    PCWSTR DirectoryName,
    PVOID pFileInfo,
    ULONG_PTR CallbackContext
    );

EXTERN_C
NTSTATUS
NTAPI
EnumFiles(
    HANDLE hRoot,
    PCWSTR pszDirectoryPath,
    PCWSTR pszFileName,
    ENUMFILESCALLBACK pfnCallback,
    ULONG_PTR CallbackContext
    );

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
    );

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
    );

#ifdef _NTIFS_
#define OpenFile OpenFile_W
#endif

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
    );

#ifndef BOOL
#define BOOL INT
#endif

typedef unsigned long DWORD;

#ifndef _WINBASE_
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    PVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
#endif

EXTERN_C
LONG
NTAPI
CreateDirectory_W(
    HANDLE hRoot,
    LPCWSTR NewDirectory,
    SECURITY_ATTRIBUTES *SecurityAttributes
    );

#ifdef _NTIFS_
#define CreateDirectory CreateDirectory_W
#endif

EXTERN_C
NTSTATUS
NTAPI
MakeSureDirectoryPathExistsW(
    PCWSTR pszFullPath
    );

EXTERN_C
NTSTATUS
NTAPI
StringFromGUID(
    __in  const GUID *Guid,
    __out LPWSTR lpszGuid,
    __in  int cchMax
    );

#define LPWSTR_GLOBALROOTPREFIX  L"\\??\\"

// A string minimum length of 6 characters need.
#define PathIsGlobalRootPrefixDosDrive(p) \
		(\
		p != NULL && \
		p[0] == L'\\' && \
		p[1] == L'?' && \
		p[2] == L'?' && \
		p[3] == L'\\' && \
		((L'A' <= p[4] && p[4] <= L'Z') || (L'a' <= p[4] && p[4] <= L'z')) && \
		p[5] == L':')

// A string minimum length of 4 characters need.
#define PathIsGlobalRootPrefix(p) \
		(\
		p != NULL && \
		p[0] == L'\\' && \
		p[1] == L'?' && \
		p[2] == L'?' && \
		p[3] == L'\\' )

#define PathIsGlobalRootPrefix_U(pus) \
		((pus)->Length >= 8 && PathIsGlobalRootPrefix((pus)->Buffer))
