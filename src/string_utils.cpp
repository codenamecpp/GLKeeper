#include "pch.h"
#include "string_utils.h"

namespace cxx
{

void str_to_lower(std::string& input_string)
{
    ::std::transform(input_string.begin(), input_string.end(), input_string.begin(), ::tolower);
}

void str_to_upper(std::string& input_string)
{
    ::std::transform(input_string.begin(), input_string.end(), input_string.begin(), ::toupper);
}

bool ends_with(const std::string& source_string, const std::string& suffix_string)
{
    if (source_string.length() >= suffix_string.length()) 
    {
        return (0 == source_string.compare(source_string.length() - suffix_string.length(), suffix_string.length(), suffix_string));
    }
    return false;
}

bool ends_with(const std::string& source_string, char suffix_char)
{
    if (source_string.empty())
        return false;

    return suffix_char == source_string.back();
}

void trim_left(std::string& input_string)
{
    input_string.erase(input_string.begin(), std::find_if(input_string.begin(), input_string.end(), 
        [](char ch) 
    {
        return !is_blank_char(ch);
    }));
}

void trim_right(std::string& input_string)
{
    input_string.erase(std::find_if(input_string.rbegin(), input_string.rend(), 
        [](char ch) 
    {
        return !is_blank_char(ch);
    }).base(), input_string.end());
}

void trim(std::string& input_string)
{
    trim_right(input_string);
    trim_left(input_string);
}

const char* va(const char* format_string, ...)
{
    va_list argptr;

    static int scope_index = 0;
    static char string_buffers[4][16384]; // in case called by nested functions

    char *current_buffer = string_buffers[scope_index];
    scope_index = (scope_index + 1) & 3;

    va_start(argptr, format_string);
    vsprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    return current_buffer;
}

} // namespace cxx