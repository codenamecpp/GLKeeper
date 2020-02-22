#pragma once

template<typename TEnum>
struct enum_string_elem
{
    TEnum mEnumValue;

    // string value should be statically allocated
    const char* mEnumString;
};

template<typename TEnum>
struct enum_strings 
{
    static const enum_string_elem<TEnum> mEnumValueStrings[];
};

//////////////////////////////////////////////////////////////////////////

#define decl_enum_strings(enum_type)\
    template<>\
    struct enum_strings<enum_type>\
    {\
        static const std::vector<enum_string_elem<enum_type>> mEnumValueStrings; \
    }

#define impl_enum_strings(enum_type)\
    const std::vector<enum_string_elem<enum_type>> enum_strings<enum_type>::mEnumValueStrings =

//////////////////////////////////////////////////////////////////////////

namespace cxx
{

    template<typename TEnum>
    inline const char* enum_to_string(TEnum enum_value)
    {
        for (const enum_string_elem<TEnum>& curr: enum_strings<TEnum>::mEnumValueStrings)
        {
            if (enum_value == curr.mEnumValue)
                return curr.mEnumString;
        }
        debug_assert(false);
        return "";
    }

    template<typename TEnum>
    inline bool parse_enum(const char* string_value, TEnum& enum_value)
    {
        for (const enum_string_elem<TEnum>& curr: enum_strings<TEnum>::mEnumValueStrings)
        {
            if (strcmp(curr.mEnumString, string_value) == 0)
            {
                enum_value = curr.mEnumValue;
                return true;
            }
        }
        debug_assert(false);
        return false;
    }

    template<typename TEnum>
    inline bool parse_enum_int(int int_value, TEnum& enum_value)
    {
        for (const enum_string_elem<TEnum>& curr: enum_strings<TEnum>::mEnumValueStrings)
        {
            if (curr.mEnumValue == int_value)
            {
                enum_value = curr.mEnumValue;
                return true;
            }
        }
        debug_assert(false);
        return false;
    }

} // namespace cxx