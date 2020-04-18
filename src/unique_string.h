#pragma once

namespace cxx
{
    // stores unique pointer to immutable string
    class unique_string
    {
    public:
        // construct null unique string
        unique_string() = default;

        // construct unique string copy
        unique_string(const unique_string& string_content)
        {
            set_data_internal(string_content.mStringHolder);
        }

        // construct unique string using c content
        explicit unique_string(const char* string_content)
        {
            set_data_internal(string_content);
        }

        // construct unique string using std string content
        explicit unique_string(const std::string& string_content)
        {
            set_data_internal(string_content);
        }

        ~unique_string()
        {
            reset_data_internal();
        }

        // set unique string data using c string content
        inline void assign(const char* string_content)
        {
            set_data_internal(string_content);
        }

        // set unique string using std string content
        inline void assign(const std::string& string_content)
        {
            set_data_internal(string_content);
        }

        // set null unique string
        inline void clear()
        {
            reset_data_internal();
        }

        // set unique string copy
        inline unique_string& operator = (const unique_string& string_content)
        {
            set_data_internal(string_content);
            return *this;
        }
        // set unique string data using c string content
        inline unique_string& operator = (const char* string_content)
        {
            set_data_internal(string_content);
            return *this;
        }
        // set unique string using std string content
        inline unique_string& operator = (const std::string& string_content)
        {
            set_data_internal(string_content);
            return *this;
        }

        inline bool operator == (const unique_string& other_string) const { return mStringHolder == other_string.mStringHolder; }
        inline bool operator != (const unique_string& other_string) const { return mStringHolder != other_string.mStringHolder; }
        // compare unique string content with std string
        inline bool operator == (const std::string& other_string) const
        {
            return mStringHolder->mString == other_string;
        }
        inline bool operator != (const std::string& other_string) const { return !(*this == other_string); }
        // compare unique string content with c string
        inline bool operator == (const char* other_string) const
        {
            return mStringHolder->mString == other_string;
        }
        inline bool operator != (const char* other_string) const { return !(*this == other_string); }
        // getters
        inline operator const char* () const { return mStringHolder->mString.c_str(); }
        inline int length() const
        {
            return mStringHolder->mString.length();
        }
        inline bool empty() const
        {
            return mStringHolder == &sNullUniqueString;
        }
        inline const char* c_str() const { return mStringHolder->mString.c_str(); }

    private:
        struct string_holder;
        static std::map<std::string, string_holder> sUniqueStringsTable;
        static string_holder sNullUniqueString;

        struct string_holder
        {
        public:
            std::string mString; // unique data
            unsigned int mReferenceCounter = 0;
        };

    private:
        // set unique string data for c string content
        void set_data_internal(const char* string_content);

        // set unique string data for std string content
        void set_data_internal(const std::string& string_content);

        // set unique string data
        void set_data_internal(string_holder* content_holder);

        // free unique string data
        void reset_data_internal();

    private:
        string_holder* mStringHolder = &sNullUniqueString;
    };

} // namespace cxx