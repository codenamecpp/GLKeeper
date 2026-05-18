#include "stdafx.h"
#include "strings.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace cxx
{

//////////////////////////////////////////////////////////////////////////

// va
static char* va_get_string_buffer()
{
    static int scope_index = 0;
    static char string_buffers[4][2048]; // in case called by nested functions

    char* current_buffer = string_buffers[scope_index];
    scope_index = (scope_index + 1) & 3;
    return current_buffer;
}

static wchar_t* va_get_wstring_buffer()
{
    static int scope_index = 0;
    static wchar_t string_buffers[4][2048]; // in case called by nested functions

    wchar_t* current_buffer = string_buffers[scope_index];
    scope_index = (scope_index + 1) & 3;
    return current_buffer;
}

//////////////////////////////////////////////////////////////////////////

void trim_left(std::string& input_string)
{
    input_string.erase(input_string.begin(), std::find_if(input_string.begin(), input_string.end(), 
        [](char ch) 
    {
        return !is_space(ch);
    }));
}

void trim_right(std::string& input_string)
{
    input_string.erase(std::find_if(input_string.rbegin(), input_string.rend(), 
        [](char ch) 
    {
        return !is_space(ch);
    }).base(), input_string.end());
}

void trim(std::string& input_string)
{
    trim_right(input_string);
    trim_left(input_string);
}

//////////////////////////////////////////////////////////////////////////

const char* va(const char *format_string, ...)
{
    va_list argptr;

    char* current_buffer = va_get_string_buffer();

    va_start(argptr, format_string);
    vsprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    return current_buffer;
}

const wchar_t* va(const wchar_t* format_string, ...)
{
    va_list argptr;

    wchar_t* current_buffer = va_get_wstring_buffer();

    va_start(argptr, format_string);
    vswprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    return current_buffer;
}

//////////////////////////////////////////////////////////////////////////

int str_printf(std::string& stringBuffer, const char* format_string, ...)
{
    va_list argptr;

    char* current_buffer = va_get_string_buffer();

    va_start(argptr, format_string);
    int str_length = vsprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    stringBuffer.clear();
    stringBuffer.assign(current_buffer);

    return str_length;
}

int str_wprintf(std::wstring& stringBuffer, const wchar_t* format_string, ...)
{
    va_list argptr;

    wchar_t* current_buffer = va_get_wstring_buffer();

    va_start(argptr, format_string);
    int str_length = vswprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    stringBuffer.clear();
    stringBuffer.assign(current_buffer);

    return str_length;
}

//////////////////////////////////////////////////////////////////////////

bool string_to_wide_string(const std::string_view& srcString, std::wstring& resultString)
{
    if (srcString.empty())
    {
        resultString.clear();
        return true;
    }

    size_t wideCharsCount = ::MultiByteToWideChar(CP_UTF8, 0, srcString.data(), srcString.size(), nullptr, 0);
    if (wideCharsCount == 0)
    {
        return false;
    }

    resultString.resize(wideCharsCount, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, srcString.data(), srcString.size(), resultString.data(), wideCharsCount);
    return true;
}

//////////////////////////////////////////////////////////////////////////

} // namespace cxx