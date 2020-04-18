#include "pch.h"
#include "unique_string.h"

namespace cxx
{

// unique string table singleton
std::map<std::string, unique_string::string_holder> unique_string::sUniqueStringsTable;

// null unique string singleton
unique_string::string_holder unique_string::sNullUniqueString;

void unique_string::set_data_internal(const char* string_content)
{
    // check whether string isn't null
    if (string_content == nullptr || *string_content == 0)
    {
        reset_data_internal();
        return;
    }

    std::string string_content_temp(string_content);
    string_holder& holder = sUniqueStringsTable[string_content_temp];
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

    string_holder& holder = sUniqueStringsTable[string_content];
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

    if (mStringHolder != &sNullUniqueString)
    {
        ++mStringHolder->mReferenceCounter;
    }
}

void unique_string::reset_data_internal()
{
    if (mStringHolder == &sNullUniqueString)
        return;

    debug_assert(mStringHolder->mReferenceCounter > 0);
    if (--mStringHolder->mReferenceCounter == 0)
    {
        auto map_iterator = sUniqueStringsTable.find(mStringHolder->mString);
        if (map_iterator != sUniqueStringsTable.end())
        {
            sUniqueStringsTable.erase(map_iterator);
        }
        else
        {
            debug_assert(false);
        }
    }
    mStringHolder = &sNullUniqueString;
}

} // namespace cxx