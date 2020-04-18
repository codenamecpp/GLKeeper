#include "pch.h"
#include "unique_string.h"

namespace cxx
{

void unique_string::set_data_internal(const char* string_content)
{
    // check whether string isn't null
    if (string_content == nullptr || *string_content == 0)
    {
        reset_data_internal();
        return;
    }
    std::string string_content_temp(string_content);

    unique_string_table& strings_table = get_strings_table();
    string_holder& holder = strings_table[string_content_temp];
    if (holder.mReferenceCounter == 0) 
    {
        // consume temporary std string
        holder.mString.swap(string_content_temp);
    }
    set_data_internal(&holder);
}

void unique_string::set_data_internal(const std::string& string_content)
{
    // check whether string isn't null
    if (string_content.empty())
    {
        reset_data_internal();
        return;
    }

    unique_string_table& strings_table = get_strings_table();
    string_holder& holder = strings_table[string_content];
    if (holder.mReferenceCounter == 0) 
    {
        holder.mString = string_content;
    }
    set_data_internal(&holder); 
}

void unique_string::set_data_internal(string_holder* content_holder)
{
    if (mStringHolder == content_holder) // same, do nothing
        return;

    reset_data_internal();
    mStringHolder = content_holder;

    if (mStringHolder != get_null_string_data())
    {
        ++mStringHolder->mReferenceCounter;
    }
}

void unique_string::reset_data_internal()
{
    string_holder* null_string_data = get_null_string_data();
    if (mStringHolder == null_string_data)
        return;

    debug_assert(mStringHolder->mReferenceCounter > 0);
    if (--mStringHolder->mReferenceCounter == 0)
    {
        unique_string_table& strings_table = get_strings_table();
        auto map_iterator = strings_table.find(mStringHolder->mString);
        if (map_iterator != strings_table.end())
        {
            strings_table.erase(map_iterator);
        }
        else
        {
            debug_assert(false);
        }
    }

    mStringHolder = null_string_data;
}

} // namespace cxx