
// stdafx.cpp : source file that includes just the standard includes
// STEPViewer.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

extern CString FromUTF8(const char* utf8)
{
    // Calculate the required buffer size for the UTF-16 string
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (sizeNeeded <= 0) {
        return L""; // Return empty on failure
    }

    std::wstring utf16Str(sizeNeeded, L'\0'); // Create a wstring of the required size
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &utf16Str[0], sizeNeeded);

    return utf16Str.c_str(); // Return the converted UTF-16 string
}

extern std::string ToUTF8(const wchar_t* utf16)
{
    // Calculate the required buffer size for the UTF-8 string
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0) {
        return std::string(); // Return empty on failure
    }

    std::string utf8Str(sizeNeeded, '\0'); // Create a string of the required size
    WideCharToMultiByte(CP_UTF8, 0, utf16, -1, &utf8Str[0], sizeNeeded, NULL, NULL);

    return utf8Str; // Return the converted UTF-8 string
}
