#include "pch.h"
#include "json_document.h"
#include "cJSON.h"

namespace cxx
{

json_document_node::json_document_node(cJSON* jsonElementImplementation)
    : mJsonElement(jsonElementImplementation)
{
}

json_document_node::json_document_node()
{
}

json_document_node::~json_document_node()
{
}


json_document_node json_document_node::operator[](const char* name) const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = cJSON_GetObjectItem(mJsonElement, name);
    return json_document_node { element };
}

json_document_node json_document_node::next_sibling() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->next;
    return json_document_node { element };
}

json_document_node json_document_node::prev_sibling() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->prev;
    return json_document_node { element };
}

json_document_node json_document_node::first_child() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->child;
    return json_document_node { element };
}

json_document_node json_document_node::operator[](int elementIndex) const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    if (mJsonElement->type == cJSON_Object)
    {
        cJSON* currentChild = mJsonElement->child;
        while (currentChild && elementIndex > 0) 
        {
            --elementIndex; 
            currentChild = currentChild->next; 
        }
        return json_document_node {currentChild};
    }

    if (mJsonElement->type == cJSON_Array)
    {
        cJSON* element = cJSON_GetArrayItem(mJsonElement, elementIndex);
        return json_document_node { element };
    }
    debug_assert(false);
    return json_document_node {};
}

int json_document_node::get_elements_count() const
{
    if (mJsonElement == nullptr)
        return 0;

    if (mJsonElement->type == cJSON_Object)
    {
        cJSON* currentChild = mJsonElement->child;
        int counter = 0;
        while(currentChild)
        {
            ++counter;
            currentChild = currentChild->next; 
        }
        return counter;
    }

    if (mJsonElement->type == cJSON_Array)
    {
        return cJSON_GetArraySize(mJsonElement);
    }
    return 0;
}

bool json_document_node::is_child_exists(const char* name) const
{
    if (mJsonElement == nullptr)
        return false;

    return cJSON_HasObjectItem(mJsonElement, name) > 0;
}

const char* json_document_node::get_element_name() const
{
    if (mJsonElement == nullptr || mJsonElement->string == nullptr)
        return "";

    return mJsonElement->string;
}

bool json_document_node::is_string_element() const
{
    return mJsonElement && mJsonElement->type == cJSON_String;
}

bool json_document_node::is_number_element() const
{
    return mJsonElement && mJsonElement->type == cJSON_Number;
}

bool json_document_node::is_boolean_element() const
{
    return mJsonElement && (mJsonElement->type == cJSON_True || mJsonElement->type == cJSON_False);
}

bool json_document_node::is_array_element() const
{
    return mJsonElement && mJsonElement->type == cJSON_Array;
}

bool json_document_node::is_object_element() const
{
    return mJsonElement && mJsonElement->type == cJSON_Object;
}

const char* json_document_node::get_value_string() const
{
    debug_assert(is_string_element());

    if (mJsonElement == nullptr || mJsonElement->valuestring == nullptr)
        return "";

    return mJsonElement->valuestring;
}

bool json_document_node::get_value_boolean() const
{
    debug_assert(is_boolean_element());
    return mJsonElement && mJsonElement->valueint != 0;
}

int json_document_node::get_value_integer() const
{
    debug_assert(is_number_element());
    if (mJsonElement == nullptr)
        return 0;

    return mJsonElement->valueint;
}

float json_document_node::get_value_float() const
{
    debug_assert(is_number_element());
    if (mJsonElement == nullptr)
        return 0.0f;

    return static_cast<float>(mJsonElement->valuedouble);
}

//////////////////////////////////////////////////////////////////////////

json_document::json_document(const char* content)
    : mJsonElement()
{
    parse_document(content);
}

json_document::json_document()
    : mJsonElement()
{
}

json_document::~json_document()
{
    close_document();
}

bool json_document::parse_document(const char* content)
{
    close_document();

    mJsonElement = cJSON_Parse(content);
    return mJsonElement != nullptr;
}

void json_document::close_document()
{
    if (mJsonElement)
    {
        cJSON_Delete(mJsonElement);
        mJsonElement = nullptr;
    }
}

json_document_node json_document::get_root_node() const
{
    return json_document_node { mJsonElement };
}

bool json_document::is_loaded() const
{
    return mJsonElement != nullptr;
}

} // namespace cxx