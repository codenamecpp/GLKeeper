#include "stdafx.h"
#include "JsonDocument.h"
#include "cJSON.h"

//////////////////////////////////////////////////////////////////////////

inline bool eq_range_ignorecase(const char *sz, const char *szRangeFrom, const char* szRangeEnd)
{
    if (!sz || !szRangeFrom || !szRangeEnd)
        return false;

    while (*sz)
    {
        if (szRangeFrom == szRangeEnd)
            return false;

        int a = ::tolower(*sz);
        int b = ::tolower(*szRangeFrom);
        if (a != b)
            return false;

        ++sz; ++szRangeFrom;
    }
    return szRangeFrom == szRangeEnd;
}

//////////////////////////////////////////////////////////////////////////

static cJSON* cJSON_GetChildByName(cJSON* firstChild, const char* objectName, const char* objectNameEnd)
{
    for (;firstChild; firstChild = firstChild->next)
    {
        if (eq_range_ignorecase(firstChild->string, objectName, objectNameEnd))
            return firstChild;
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////

bool JsonElement::IsString() const
{
    return mJsonElement && mJsonElement->type == cJSON_String;
}

bool JsonElement::IsNumber() const
{
    return mJsonElement && mJsonElement->type == cJSON_Number;
}

bool JsonElement::IsBoolean() const
{
    return mJsonElement && (mJsonElement->type == cJSON_True || mJsonElement->type == cJSON_False);
}

bool JsonElement::IsObject() const
{
    return mJsonElement && mJsonElement->type == cJSON_Object;
}

bool JsonElement::IsArray() const
{
    return mJsonElement && mJsonElement->type == cJSON_Array;
}

JsonElement JsonElement::GetArrayElement(int elementIndex) const
{
    if (mJsonElement)
        return {cJSON_GetArrayItem(mJsonElement, elementIndex)};

    return nullptr;
}

JsonElement JsonElement::NextSibling() const
{
    bool hasSibling = mJsonElement && mJsonElement->next;
    if (hasSibling)
        return {mJsonElement->next};

    return nullptr;
}

JsonElement JsonElement::PrevSibling() const
{
    bool hasSibling = mJsonElement && mJsonElement->prev;
    if (hasSibling)
        return {mJsonElement->prev};

    return nullptr;
}

JsonElement JsonElement::FirstChild() const
{
    bool hasChild = mJsonElement && mJsonElement->child;
    if (hasChild)
        return {mJsonElement->child};

    return nullptr;
}

const char* JsonElement::GetElementName() const
{
    bool hasName = mJsonElement && mJsonElement->string;
    if (hasName)
        return mJsonElement->string;

    return "";
}

const char* JsonElement::GetValueString() const
{
    cxx_assert(IsString());
    if (mJsonElement->valuestring)
        return mJsonElement->valuestring;

    return "";
}

int JsonElement::GetValueInteger() const
{
    cxx_assert(IsNumber());
    return mJsonElement->valueint;
}

float JsonElement::GetValueFloat() const
{
    cxx_assert(IsNumber());
    return static_cast<float>(mJsonElement->valuedouble);
}

double JsonElement::GetValueDouble() const
{
    cxx_assert(IsNumber());
    return mJsonElement->valuedouble;
}

bool JsonElement::GetValueBoolean() const
{
    cxx_assert(IsBoolean());
    return mJsonElement->valueint != 0;
}

int JsonElement::GetArrayElementsCount() const
{
    cxx_assert(IsArray());
    return cJSON_GetArraySize(mJsonElement);
}

JsonElement JsonElement::FindElement(const std::string& stringPath) const
{
    bool hasChild = IsObject();
    if (!hasChild)
        return false;

    cJSON* cjson = mJsonElement;
    for (std::string::size_type istart = 0, ilength = stringPath.size(); cjson;)
    {
        std::string::size_type iseparator = stringPath.find('.', istart);
        if (iseparator == std::string::npos)
            iseparator = ilength;

        cjson = cJSON_GetChildByName(cjson->child, 
            stringPath.c_str() + istart, 
            stringPath.c_str() + iseparator);

        istart = iseparator + 1;
        if (iseparator == ilength)
            break;
    }
    return { cjson };
}

//////////////////////////////////////////////////////////////////////////

JsonDocument::JsonDocument(const std::string& jsonFileConent)
    : mJsonRootObject()
{
    ParseDocument(jsonFileConent);
}

JsonDocument::JsonDocument()
    : mJsonRootObject()
{}

JsonDocument::~JsonDocument()
{
    CloseDocument();
}

void JsonDocument::CloseDocument()
{
    if (mJsonRootObject)
    {
        cJSON_Delete(mJsonRootObject);
        mJsonRootObject = nullptr;
    }
}

bool JsonDocument::ParseDocument(const std::string& jsonFileConent)
{
    if (mJsonRootObject)
    {
        cJSON_Delete(mJsonRootObject);
    }
    mJsonRootObject = cJSON_Parse(jsonFileConent.c_str());
    return mJsonRootObject != nullptr;
}

//////////////////////////////////////////////////////////////////////////

bool JsonReadArray(const JsonElement& jsonElement, std::vector<bool>& booleansArray)
{
    bool isArray = jsonElement.IsArray();
    if (isArray)
    {
        booleansArray.clear();
        booleansArray.reserve(jsonElement.GetArrayElementsCount());

        // read numbers
        for (JsonElement subElement = jsonElement.FirstChild(); subElement; subElement = subElement.NextSibling())
        {
            const bool boolean_value = subElement.GetValueBoolean();
            booleansArray.push_back(boolean_value);
        }
    }
    return isArray;
}

bool JsonReadArray(const JsonElement& jsonElement, std::vector<int>& numbersArray)
{
    bool isArray = jsonElement.IsArray();
    if (isArray)
    {
        numbersArray.clear();
        numbersArray.reserve(jsonElement.GetArrayElementsCount());

        // read numbers
        for (JsonElement subElement = jsonElement.FirstChild(); subElement; subElement = subElement.NextSibling())
        {
            const int integer_value = subElement.GetValueInteger();
            numbersArray.push_back(integer_value);
        }
    }
    return isArray;
}

bool JsonReadArray(const JsonElement& jsonElement, std::vector<float>& numbersArray)
{
    bool isArray = jsonElement.IsArray();
    if (isArray)
    {
        numbersArray.clear();
        numbersArray.reserve(jsonElement.GetArrayElementsCount());

        // read numbers
        for (JsonElement subElement = jsonElement.FirstChild(); subElement; subElement = subElement.NextSibling())
        {
            const float float_value = subElement.GetValueFloat();
            numbersArray.push_back(float_value);
        }
    }
    return isArray;
}

bool JsonReadArray(const JsonElement& jsonElement, std::vector<std::string>& stringArray)
{
    bool isArray = jsonElement.IsArray();
    if (isArray)
    {
        // prepare buffer
        stringArray.clear();
        stringArray.reserve(jsonElement.GetArrayElementsCount());

        // read numbers
        for (JsonElement subElement = jsonElement.FirstChild(); subElement; subElement = subElement.NextSibling())
        {
            stringArray.emplace_back(subElement.GetValueString());
        }
    }
    return isArray;
}

bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, int& output)
{
    if (JsonElement element = jsonElement.FindElement(elementName))
    {
        bool isInteger = element.IsNumber();
        cxx_assert(isInteger);
        if (isInteger)
        {
            output = element.GetValueInteger();
        }
        return isInteger;
    }
    return false;
}

bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, float& output)
{
    if (JsonElement element = jsonElement.FindElement(elementName))
    {
        if (element.IsNumber())
        {
            output = element.GetValueFloat();
            return true;
        }
    }
    return false;
}

bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, bool& output)
{
    if (JsonElement element = jsonElement.FindElement(elementName))
    {
        if (element.IsBoolean())
        {
            output = element.GetValueBoolean();
            return true;
        }
    }
    return false;
}

bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, std::string& output)
{
    if (JsonElement element = jsonElement.FindElement(elementName))
    {
        if (element.IsString())
        {
            output = element.GetValueString();
            return true;
        }       
    }
    return false;
}

bool JsonReadValue(const JsonElement& element, Color32& output)
{
    bool isArray = element.IsArray();
    cxx_assert(isArray);
    if (!isArray)
        return false;

    int rgba[4] {0, 0, 0, 255}; // default is black

    // max 4 elements rgba
    const int NumComponents = element.GetArrayElementsCount();
    const int MaxComponents = 4;
    cxx_assert(NumComponents <= MaxComponents);
    for (int iComponent = 0; iComponent < NumComponents && iComponent < MaxComponents; ++iComponent)
    {
        JsonElement component = element.GetArrayElement(iComponent);
        cxx_assert(component.IsNumber());
        if (component.IsNumber())
        {
            rgba[iComponent] = component.GetValueInteger();
        }
    }
    output.SetComponents(rgba[0], rgba[1], rgba[2], rgba[3]);
    return isArray;
}


bool JsonReadValue(const JsonElement& element, Rect2D& output)
{
    bool isArray = element.IsArray();
    cxx_assert(isArray);
    if (!isArray)
        return false;

    // must be exactly 4 elements
    cxx_assert(element.GetArrayElementsCount() == 4);
    if (element.GetArrayElementsCount() != 4)
        return false;

    JsonElement element_x = element.GetArrayElement(0);
    JsonElement element_y = element.GetArrayElement(1);
    JsonElement element_w = element.GetArrayElement(2);
    JsonElement element_h = element.GetArrayElement(3);

    bool isNumbers = element_x.IsNumber() && element_y.IsNumber() && element_w.IsNumber() && element_h.IsNumber();
    cxx_assert(isNumbers);
    if (isNumbers)
    {
        output.x = element_x.GetValueInteger();
        output.y = element_y.GetValueInteger();
        output.w = element_w.GetValueInteger();
        output.h = element_h.GetValueInteger();
    }
    return isNumbers;
}

bool JsonReadValue(const JsonElement& element, Point2D& output)
{
    bool isArray = element.IsArray();
    cxx_assert(isArray);
    if (!isArray)
        return false;

    // must be exactly 2 elements
    cxx_assert(element.GetArrayElementsCount() == 2);
    if (element.GetArrayElementsCount() != 2)
        return false;

    JsonElement element_x = element.GetArrayElement(0);
    JsonElement element_y = element.GetArrayElement(1);

    bool isNumbers = element_x.IsNumber() && element_y.IsNumber();
    cxx_assert(isNumbers);
    if (isNumbers)
    {
        output.x = element_x.GetValueInteger();
        output.y = element_y.GetValueInteger();
    }
    return isNumbers;
}

bool JsonReadValue(const JsonElement& element, glm::vec2& output)
{
    bool isArray = element.IsArray();
    cxx_assert(isArray);
    if (!isArray)
        return false;

    // must be exactly 2 elements
    cxx_assert(element.GetArrayElementsCount() == 2);
    if (element.GetArrayElementsCount() != 2)
        return false;

    JsonElement element_x = element.GetArrayElement(0);
    JsonElement element_y = element.GetArrayElement(1);

    bool isNumbers = element_x.IsNumber() && element_y.IsNumber();
    cxx_assert(isNumbers);
    if (isNumbers)
    {
        output.x = element_x.GetValueFloat();
        output.y = element_y.GetValueFloat();
    }
    return isNumbers;
}
