#pragma once

#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

EXTERN_C
VOID
NTAPI
RtlSetLastWin32Error(
    ULONG ErrorCode
    );

EXTERN_C
ULONG
NTAPI
RtlGetLastWin32Error(
    VOID
    );

//////////////////////////////////////////////////////////////////////////////

//
// Heap Runtime Routine
//

EXTERN_C
ULONG
NTAPI
RtlGetProcessHeaps(
    ULONG MaxNumberOfHeaps,
    PVOID *HeapArray
    );

EXTERN_C
PVOID
NTAPI
RtlReAllocateHeap(
    HANDLE hHeap,
    ULONG dwFlags,
    PVOID lpMem,
    SIZE_T dwBytes
    );

//////////////////////////////////////////////////////////////////////////////

//
// Path Runtime Routine
//

#define DOS_MAX_COMPONENT_LENGTH 255
#define DOS_MAX_PATH_LENGTH (DOS_MAX_COMPONENT_LENGTH + 5)
#define WIN32_MAX_PATH DOS_MAX_PATH_LENGTH

#define ANSI_NULL ((CHAR)0)     
#define UNICODE_NULL ((WCHAR)0) 
#define UNICODE_STRING_MAX_BYTES ((USHORT) 65534) 
#define UNICODE_STRING_MAX_CHARS (32767) 

#define PATH_BUFFER_BYTES   (UNICODE_STRING_MAX_BYTES + sizeof(WCHAR))
#define PATH_BUFFER_LENGTH  (UNICODE_STRING_MAX_CHARS)

#define IS_RELATIVE_DIR_NAME_WITH_UNICODE_SIZE(path,size) \
            ((path[0] == L'.' && size == sizeof(WCHAR)) || \
            (path[0] == L'.' && path[1] == L'.' && (size == (sizeof(WCHAR)*2))))

#define WCHAR_LENGTH(u) ((u) / sizeof(WCHAR))
#define WCHAR_BYTES(w) ((w) * sizeof(WCHAR))
#define WCHAR_CHARS(u) WCHAR_LENGTH(u)

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR;

EXTERN_C
NTSYSAPI
ULONG
NTAPI
RtlGetCurrentDirectory_U(
    IN ULONG nBufferLength,
    OUT PWSTR lpBuffer
    );

EXTERN_C
BOOLEAN
NTAPI
RtlDosPathNameToNtPathName_U(
    IN PCWSTR DosPathName,
    OUT PUNICODE_STRING NtPathName,
    OUT PCWSTR *NtFileNamePart,
    OUT CURDIR *DirectoryInfo
    );

EXTERN_C
ULONG
NTAPI
RtlGetFullPathName_U(
    PCWSTR lpFileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    );

EXTERN_C
BOOLEAN
NTAPI
RtlDoesFileExists_U(
    PCWSTR FileName
    );

EXTERN_C
NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosOrNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

EXTERN_C
NTSTATUS
NTAPI
RtlGetLengthWithoutTrailingPathSeperators(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

typedef enum _RTL_PATH_TYPE {
    RtlPathTypeUnknown,         // 0
    RtlPathTypeUncAbsolute,     // 1
    RtlPathTypeDriveAbsolute,   // 2
    RtlPathTypeDriveRelative,   // 3
    RtlPathTypeRooted,          // 4
    RtlPathTypeRelative,        // 5
    RtlPathTypeLocalDevice,     // 6
    RtlPathTypeRootLocalDevice  // 7
} RTL_PATH_TYPE;

EXTERN_C
NTSYSAPI
RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(
    PCWSTR DosFileName
    );

typedef struct _RTLP_CURDIR_REF *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U {
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

EXTERN_C
NTSYSAPI
BOOLEAN
NTAPI
RtlDosPathNameToRelativeNtPathName_U(
    PCWSTR DosFileName,
    PUNICODE_STRING NtFileName,
    PWSTR *FilePart,
    PRTL_RELATIVE_NAME_U RelativeName
    );

EXTERN_C
VOID
NTAPI
RtlReleaseRelativeName(
    PRTL_RELATIVE_NAME_U RelativeName
    );

EXTERN_C
NTSTATUS
NTAPI
RtlQueryEnvironmentVariable_U(
     __in_opt PVOID Environment,
     __in PUNICODE_STRING Name,
     __out PUNICODE_STRING Value
     );

struct _RTL_BUFFER;

#if !defined(RTL_BUFFER)

#define RTL_BUFFER RTL_BUFFER

typedef struct _RTL_BUFFER {
    PUCHAR    Buffer;
    PUCHAR    StaticBuffer;
    SIZE_T    Size;
    SIZE_T    StaticSize;
    SIZE_T    ReservedForAllocatedSize; // for future doubling
    PVOID     ReservedForIMalloc; // for future pluggable growth
} RTL_BUFFER, *PRTL_BUFFER;

#endif

#define RTLP_BUFFER_IS_HEAP_ALLOCATED(b) ((b)->Buffer != (b)->StaticBuffer)

struct _RTL_UNICODE_STRING_BUFFER;

typedef struct _RTL_UNICODE_STRING_BUFFER {
    UNICODE_STRING String;
    RTL_BUFFER     ByteBuffer;
    UCHAR          MinimumStaticBufferForTerminalNul[sizeof(WCHAR)];
} RTL_UNICODE_STRING_BUFFER, *PRTL_UNICODE_STRING_BUFFER;

//
// These are OUT Disposition values.
//
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS   (0x00000001)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_UNC         (0x00000002)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_DRIVE       (0x00000003)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_ALREADY_DOS (0x00000004)

NTSYSAPI
NTSTATUS
NTAPI
RtlNtPathNameToDosPathName(
    __in ULONG Flags,
    __inout PRTL_UNICODE_STRING_BUFFER Path,
    __out_opt PULONG Disposition,
    __inout_opt PWSTR* FilePart
    );

#define RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE (0x00000001)
#define RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING (0x00000002)
#define RTL_DUPSTR_ADD_NULL                          RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE
#define RTL_DUPSTR_ALLOC_NULL                        RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING


// VOID
// RtlInitBuffer(
//     OUT PRTL_BUFFER Buffer,
//     IN  PUCHAR      StaticBuffer,
//     IN  SIZE_T      StaticSize
//     );
#define RtlInitBuffer(Buff, StatBuff, StatSize) \
    do {                                        \
        (Buff)->Buffer       = (StatBuff);      \
        (Buff)->Size         = (StatSize);      \
        (Buff)->StaticBuffer = (StatBuff);      \
        (Buff)->StaticSize   = (StatSize);      \
    } while (0)

#define RTL_ENSURE_BUFFER_SIZE_NO_COPY (0x00000001)

// VOID
// RtlFreeBuffer(
//     IN  PRTL_BUFFER Buffer,
//     );
#define RtlFreeBuffer(Buff)                              \
    do {                                                 \
        if ((Buff) != NULL && (Buff)->Buffer != NULL) {  \
            if (RTLP_BUFFER_IS_HEAP_ALLOCATED(Buff)) {   \
                UNICODE_STRING UnicodeString;            \
                UnicodeString.Buffer = (PWSTR)(PVOID)(Buff)->Buffer; \
                RtlFreeUnicodeString(&UnicodeString);    \
            }                                            \
            (Buff)->Buffer = (Buff)->StaticBuffer;       \
            (Buff)->Size = (Buff)->StaticSize;           \
        }                                                \
    } while (0)


//////////////////////////////////////////////////////////////////////////////

//
// Process Runtime Routine
//
typedef struct _NT_PEB_FRAGMENT
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union {
        BOOLEAN BitField;
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN SpareBits : 7;
         };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PVOID Ldr;               // PEB_LDR_DATA*
    PVOID ProcessParameters; // RTL_USER_PROCESS_PARAMETERS*
    PVOID SubSystemData;
    PVOID ProcessHeap;       // Process Heap
} NT_PEB,NT_PEB_FRAGMENT;

#if 0
#define RtlProcessHeap() (NtCurrentPeb()->ProcessHeap)
#endif

EXTERN_C
PVOID
NTAPI
RtlGetCurrentPeb(
    VOID
    );

typedef struct _SECTION_IMAGE_INFORMATION
{
     PVOID TransferAddress;
     ULONG ZeroBits;
     ULONG MaximumStackSize;
     ULONG CommittedStackSize;
     ULONG SubSystemType;
     union
     {
          struct
          {
               USHORT SubSystemMinorVersion;
               USHORT SubSystemMajorVersion;
          };
          ULONG SubSystemVersion;
     };
     ULONG GpValue;
     USHORT ImageCharacteristics;
     USHORT DllCharacteristics;
     USHORT Machine;
     UCHAR ImageContainsCode;
     UCHAR ImageFlags;
     ULONG ComPlusNativeReady: 1;
     ULONG ComPlusILOnly: 1;
     ULONG ImageDynamicallyRelocated: 1;
     ULONG Reserved: 5;
     ULONG LoaderFlags;
     ULONG ImageFileSize;
     ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

typedef struct RTL_DRIVE_LETTER_CURDIR
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PWSTR Environment;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectories[32];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _RTL_USER_PROCESS_INFORMATION
{
    ULONG Size;
    HANDLE ProcessHandle;
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;
    SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserProcess(
    __in PUNICODE_STRING NtImagePathName,
    __in ULONG AttributesDeprecated,
    __in PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    __in_opt PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
    __in_opt PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    __in_opt HANDLE ParentProcess,
    __in BOOLEAN InheritHandles,
    __in_opt HANDLE DebugPort,
    __in_opt HANDLE TokenHandle, // used to be ExceptionPort
    __out PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

EXTERN_C
NTSTATUS
NTAPI
RtlCreateProcessParameters(
    OUT PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL
    );

EXTERN_C
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlNormalizeProcessParams(
    IN OUT PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlDeNormalizeProcessParams(
    __inout PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyProcessParameters(
    __in __post_invalid PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS (NTAPI *PUSER_THREAD_START_ROUTINE)(
    __in PVOID ThreadParameter
    );

EXTERN_C
NTSTATUS
NTAPI
RtlCreateUserThread(
    IN HANDLE Process,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN ULONG ZeroBits OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PHANDLE Thread OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateEnvironment(
    __in BOOLEAN CloneCurrentEnvironment,
    __out PVOID *Environment
    );

//////////////////////////////////////////////////////////////////////////////

//
// NT Native API
//

EXTERN_C
NTSTATUS
NTAPI
NtQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION  FileInformation
    );

EXTERN_C
NTSTATUS
NTAPI
NtWaitForSingleObject(
    IN HANDLE  Handle,
    IN BOOLEAN  Alertable,
    IN PLARGE_INTEGER  Timeout OPTIONAL
    );

EXTERN_C
NTSTATUS
NTAPI
NtSuspendThread (
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );

EXTERN_C
NTSTATUS
NTAPI
NtResumeThread (
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );

EXTERN_C
NTSTATUS
NTAPI
NtSuspendProcess (
    __in HANDLE ProcessHandle
    );

EXTERN_C
NTSTATUS
NTAPI
NtResumeProcess (
    __in HANDLE ProcessHandle
    );

EXTERN_C
NTSTATUS
NTAPI
NtQuerySystemTime(
    __in PLARGE_INTEGER SystemTime
    );

//////////////////////////////////////////////////////////////////////////////

EXTERN_C
NTSTATUS
NTAPI
RtlLocalTimeToSystemTime(
     __in  PLARGE_INTEGER LocalTime,
     __out PLARGE_INTEGER SystemTime
    );

EXTERN_C
NTSTATUS
NTAPI
RtlSystemTimeToLocalTime(
    __in PLARGE_INTEGER SystemTime,
    __out PLARGE_INTEGER LocalTime
   );

//////////////////////////////////////////////////////////////////////////////

//
// Win32 compatible
//

#ifndef CREATE_NEW
#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5
#endif

