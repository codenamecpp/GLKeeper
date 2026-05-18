#pragma once

//////////////////////////////////////////////////////////////////////////

template<typename TEnum>
struct cxx_enum_serialize_props
{
    // string value should be statically allocated
    struct container_entry { TEnum mEnumValue; const char* mEnumString; };
    using container_type = std::vector<container_entry>;
    static const container_type mEnumValueStrings;
};

//////////////////////////////////////////////////////////////////////////

#define enum_serialize_decl(enum_type) 
#define enum_serialize_impl(enum_type) \
    const cxx_enum_serialize_props<enum_type>::container_type \
        cxx_enum_serialize_props<enum_type>::mEnumValueStrings =

//////////////////////////////////////////////////////////////////////////

namespace cxx
{

    template<typename TEnum>
    inline const char* enum_to_string(TEnum enum_value)
    {
        using enum_props_t = cxx_enum_serialize_props<TEnum>;
        for (const auto& roller: enum_props_t::mEnumValueStrings)
        {
            if (enum_value == roller.mEnumValue)
                return roller.mEnumString;
        }
        return "";
    }

    template<typename TEnum>
    inline bool parse_enum(const char* string_value, TEnum& enum_value)
    {
        using enum_props_t = cxx_enum_serialize_props<TEnum>;
        for (const auto& roller: enum_props_t::mEnumValueStrings)
        {
            if (strcmp(roller.mEnumString, string_value) == 0)
            {
                enum_value = roller.mEnumValue;
                return true;
            }
        }
        return false;
    }

    template<typename TEnum>
    inline bool parse_enum_int(int int_value, TEnum& enum_value)
    {
        using enum_props_t = cxx_enum_serialize_props<TEnum>;
        for (const auto& roller: enum_props_t::mEnumValueStrings)
        {
            if (roller.mEnumValue == int_value)
            {
                enum_value = roller.mEnumValue;
                return true;
            }
        }
        return false;
    }

    template<typename TEnum>
    inline void get_enum_strings(std::vector<const char*>& out_values)
    {
        using enum_props_t = cxx_enum_serialize_props<TEnum>;
        out_values.clear();
        out_values.reserve(enum_props_t::mEnumValueStrings.size());
        for (const auto& roller: enum_props_t::mEnumValueStrings)
        {
            out_values.emplace_back(roller.mEnumString);
        }
    }    

} // namespace cxx