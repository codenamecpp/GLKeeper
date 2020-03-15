#pragma once

namespace cxx
{
    // ignore case string comparsion
    inline bool iequals(const std::string& lhs, const std::string& rhs)
    {
        if (lhs.length() != rhs.length())
            return false;

        const int iresult = cxx_stricmp(lhs.c_str(), rhs.c_str());
        return iresult == 0;
    }

    // ignore case string comparators
    struct icase_eq
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            return iequals(astring, bstring);
        }
    };

    struct icase_less
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            const int iresult = cxx_stricmp(astring.c_str(), bstring.c_str());
            return iresult < 0;
        }
    };

    struct icase_hashfunc
    {
        enum 
        { 
            FNV_OFFSET_BASIS = 2166136261U,
            FNV_PRIME = 16777619U
        };

        inline unsigned int operator() (const std::string& input_string) const
        {
            unsigned int result_value = FNV_OFFSET_BASIS;
            for (char c: input_string) 
            {
                result_value ^= tolower(c);
                result_value *= FNV_PRIME;
            }
            return result_value;
        }
    };

    // convert target string to lower case
    // @param string: Target string
    inline void str_to_lower(std::string& input_string)
    {
        ::std::transform(input_string.begin(), input_string.end(), input_string.begin(), ::tolower);
    }

    // convert target string to upper case
    // @param string: Target string
    inline void str_to_upper(std::string& input_string)
    {
        ::std::transform(input_string.begin(), input_string.end(), input_string.begin(), ::toupper);
    }

    inline bool is_blank_char(char inchar)
    {
        return inchar == ' ' || inchar == '\n' || inchar == '\t' || inchar == '\r';
    }

    // erase white spaces on left and right of the input string
    inline void trim_left(std::string& input_string)
    {
        input_string.erase(input_string.begin(), std::find_if(input_string.begin(), input_string.end(), 
            [](char ch) 
        {
            return !is_blank_char(ch);
        }));
    }

    inline void trim_right(std::string& input_string)
    {
        input_string.erase(std::find_if(input_string.rbegin(), input_string.rend(), 
            [](char ch) 
        {
            return !is_blank_char(ch);
        }).base(), input_string.end());
    }

    inline void trim(std::string& input_string)
    {
        trim_right(input_string);
        trim_left(input_string);
    }

} // namespace cxx