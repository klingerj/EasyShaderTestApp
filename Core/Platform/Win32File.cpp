#include "Platform/PlatformGameAPI.h"
#include "Utility/Logging.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Tk
{
namespace Platform
{

GET_ENTIRE_FILE_SIZE(GetEntireFileSize)
{
    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    uint32 fileSize = 0;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSizeInBytes = {};
        if (GetFileSizeEx(fileHandle, &fileSizeInBytes))
        {
            fileSize = SafeTruncateUint64(fileSizeInBytes.QuadPart);
        }
        else
        {
            fileSize = 0;
        }
        CloseHandle(fileHandle);
        return fileSize;
    }
    else
    {
        Tk::Core::Utility::LogMsg("Platform", "Unable to create file handle!", Core::Utility::LogSeverity::eCritical);
        return 0;
    }
}

READ_ENTIRE_FILE(ReadEntireFile)
{
    // User must specify a file size and the dest buffer.
    TINKER_ASSERT(fileSizeInBytes && buffer);

    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        uint32 fileSize = 0;
        if (fileSizeInBytes)
        {
            fileSize = fileSizeInBytes;
        }
        else
        {
            fileSize = Tk::Platform::GetEntireFileSize(filename);
        }

        DWORD numBytesRead = 0;
        ReadFile(fileHandle, buffer, fileSize, &numBytesRead, 0);
        TINKER_ASSERT(numBytesRead == fileSize);
        CloseHandle(fileHandle);
        return 0;
    }
    else
    {
        Tk::Core::Utility::LogMsg("Platform", "Unable to create file handle!", Core::Utility::LogSeverity::eCritical);
        return 1;
    }
}

WRITE_ENTIRE_FILE(WriteEntireFile)
{
    // User must specify a file size and the dest buffer.
    TINKER_ASSERT(fileSizeInBytes && buffer);

    HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD numBytesWritten = 0;
        WriteFile(fileHandle, buffer, fileSizeInBytes, &numBytesWritten, 0);
        TINKER_ASSERT(numBytesWritten == fileSizeInBytes);
        CloseHandle(fileHandle);
        return 0;
    }
    else
    {
        DWORD dw = GetLastError();
        Tk::Core::Utility::LogMsg("Platform", "Unable to create file handle!", Tk::Core::Utility::LogSeverity::eCritical);
        return 1;
    }
}

FIND_FILE_OPEN(FindFileOpen)
{
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(dirWithFileExts, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        mbstowcs_s(NULL, outFilename, outFilenameMax, fd.cFileName, strlen(fd.cFileName));
    }

    FileHandle handle;
    handle.h = (uint64)hFind;
    return handle;
}

FIND_FILE_NEXT(FindFileNext)
{
    WIN32_FIND_DATA fd;
    bool result = FindNextFile((HANDLE)prevFindFileHandle.h, &fd); // nonzero if no error, zero if error
    if (result)
    {
        mbstowcs_s(NULL, outFilename, outFilenameMax, fd.cFileName, strlen(fd.cFileName));
    }
    return (uint32)(!result); // for me, zero is error, nonzero if error
}

FIND_FILE_CLOSE(FindFileClose)
{
    if ((HANDLE)(handle.h) != INVALID_HANDLE_VALUE)
        FindClose((HANDLE)handle.h);
}

}
}
