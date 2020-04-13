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

    // does a varargs printf into a temp buffer, not thread safe
    const char* va(const char *format_string, ...);

    // simple tokenizer for strings
    struct string_tokenizer
    {
    public:
        string_tokenizer(const std::string& source_string)
            : mSourceStringStream(source_string)
        {
        }
        bool get_next(std::string& output_string, char delimiter = '/')
        {
            output_string.clear();
            if (std::getline(mSourceStringStream, output_string, delimiter))
                return true;

            return false;
        }
    private:
        std::stringstream mSourceStringStream;
    };

    // test whether input symbol is blank character
    inline bool is_blank_char(char inchar)
    {
        return inchar == ' ' || inchar == '\n' || inchar == '\t' || inchar == '\r';
    }

    // erase white spaces on left and right of the input string
    void trim_left(std::string& input_string);
    void trim_right(std::string& input_string);
    void trim(std::string& input_string);

    // convert target string to upper or lower case
    void str_to_lower(std::string& input_string);
    void str_to_upper(std::string& input_string);

} // namespace cxx