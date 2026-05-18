#pragma once

namespace cxx
{
    //////////////////////////////////////////////////////////////////////////

    // does a varargs printf into a temp buffer, not thread safe
    const char* va(const char *format_string, ...);
    const wchar_t* va(const wchar_t* format_string, ...);
    //////////////////////////////////////////////////////////////////////////

    int str_printf(std::string& stringBuffer, const char* format_string, ...);
    int str_wprintf(std::wstring& stringBuffer, const wchar_t* format_string, ...);

    //////////////////////////////////////////////////////////////////////////

    // ignore case string comparator
    struct icase_string_eq
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            if (astring.length() != bstring.length())
                return false;

            const int iresult = _stricmp(astring.c_str(), bstring.c_str());
            return iresult == 0;
        }
    };

    struct icase_string_less
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            const int iresult = _stricmp(astring.c_str(), bstring.c_str());
            return iresult < 0;
        }
    };

    struct icase_string_hashfunc
    {
        enum 
        { 
            FNV_OFFSET_BASIS = 2166136261U,
            FNV_PRIME = 16777619U
        };

        inline unsigned int operator() (const std::string& theString) const
        {
	        unsigned int result_value = FNV_OFFSET_BASIS;
            for (const char& c: theString) 
            {
		        result_value ^= tolower(c);
		        result_value *= FNV_PRIME;
            }
	        return result_value;
        }
    };

    //////////////////////////////////////////////////////////////////////////

    // helpers
    template<typename TString>
    inline int string_length(const TString& srcString) { return static_cast<int>(srcString.length()); }
    inline int string_length(const char* srcString) { return static_cast<int>(strlen(srcString)); }

    template<typename TString>
    inline const char* c_str(const TString& srcString) { return srcString.c_str(); }
    inline const char* c_str(const char* srcString) { return srcString; }

    //////////////////////////////////////////////////////////////////////////

    template<typename TStringLHS, typename TStringRHS>
    inline bool strings_eq(const TStringLHS& lhs, const TStringRHS& rhs)
    {
        return 0 == strcmp(c_str(lhs), c_str(rhs));
    }

    template<typename TSrcString, typename TPrefixString>
    inline bool starts_with_icase(const TSrcString& sourceString, const TPrefixString& prefixString)
    {
        int stringLength = string_length(sourceString);
        int prefixLength = string_length(prefixString);
        return (stringLength >= prefixLength) && (0 == _strnicmp(c_str(sourceString), c_str(prefixString), prefixLength));
    }

    template<typename TSrcString, typename TPrefixString>
    inline bool starts_with(const TSrcString& sourceString, TPrefixString& prefixString)
    {
        int stringLength = string_length(sourceString);
        int prefixLength = string_length(prefixString);
        return (stringLength >= prefixLength) && (0 == strncmp(c_str(sourceString), c_str(prefixString), prefixLength));
    }

    template<typename TSrcString, typename TSuffixString>
    inline bool ends_with_icase(const TSrcString& sourceString, const TSuffixString& suffixString)
    {
        int stringLength = string_length(sourceString);
        int suffixLength = string_length(suffixString);
        return (stringLength >= suffixLength) && (0 == _stricmp(c_str(sourceString) + stringLength - suffixLength, c_str(suffixString)));
    }

    template<typename TSrcString, typename TSuffixString>
    inline bool ends_with(const TSrcString& sourceString, const TSuffixString& suffixString)
    {
        int stringLength = string_length(sourceString);
        int suffixLength = string_length(suffixString);
        return (stringLength >= suffixLength) && (0 == strcmp(c_str(sourceString) + stringLength - suffixLength, c_str(suffixString)));
    }

    //////////////////////////////////////////////////////////////////////////

    // convert target string to lower case
    // @param string: Target string
    inline std::string lower_string(std::string string)
    {
        std::transform(string.begin(), string.end(), string.begin(), tolower);
        return string;
    }

    // convert target string to upper case
    // @param string: Target string
    inline std::string upper_string(std::string string)
    {
        std::transform(string.begin(), string.end(), string.begin(), toupper);
        return string;
    }

    inline bool is_space(char inchar)
    {
        return inchar == ' ' || inchar == '\n' || inchar == '\t' || inchar == '\r';
    }

    // erase white spaces on left and right of the input string
    void trim_left(std::string& input_string);
    void trim_right(std::string& input_string);
    void trim(std::string& input_string);

    //////////////////////////////////////////////////////////////////////////

    // utf-8 to wide

    bool string_to_wide_string(const std::string_view& srcString, std::wstring& resultString);

    //////////////////////////////////////////////////////////////////////////

} // namespace cxx