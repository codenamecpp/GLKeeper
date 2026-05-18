#pragma once

// Internals
struct cJSON;

//////////////////////////////////////////////////////////////////////////
// Json Element
//////////////////////////////////////////////////////////////////////////

class JsonElement
{
public:
    // ctor
    JsonElement(cJSON* jsonElementImplementation = nullptr) 
        : mJsonElement(jsonElementImplementation)
    {}

    // Find child element by path, use '.' symbol to separator
    // @param stringPath: String path
    JsonElement FindElement(const std::string& stringPath) const;

    // Get next / previous sibling json element
    JsonElement NextSibling() const;
    JsonElement PrevSibling() const;

    // Get first child element of object or array element
    JsonElement FirstChild() const;

    // Get array element by index
    // @param elementIndex: Array element index
    JsonElement GetArrayElement(int elementIndex) const;

    // Get name of json element
    const char* GetElementName() const;

    // Determine type of json element value
    bool IsString() const;
    bool IsNumber() const;
    bool IsBoolean() const;
    bool IsArray() const;
    bool IsObject() const;

    // Get json element value
    const char* GetValueString() const;
    bool GetValueBoolean() const;
    int GetValueInteger() const;
    float GetValueFloat() const;
    double GetValueDouble() const;

    // Get number of elements in array
    int GetArrayElementsCount() const;

    // Operators
    inline bool operator == (const JsonElement& otherElement) const { return mJsonElement && otherElement.mJsonElement == mJsonElement; }
    inline bool operator != (const JsonElement& otherElement) const { return !mJsonElement || otherElement.mJsonElement != mJsonElement; }
    inline operator bool () const { return mJsonElement != nullptr; }

private:
    cJSON* mJsonElement;
};

//////////////////////////////////////////////////////////////////////////
// Json document
//////////////////////////////////////////////////////////////////////////

class JsonDocument : public cxx::noncopyable
{
public:
    // ctor
    // @param jsonFileConent: Json content string
    JsonDocument(const std::string& jsonFileConent);
    JsonDocument();
    ~JsonDocument();

    // Get root json object of document
    inline JsonElement GetRootElement() const { return mJsonRootObject; }

    // Parse json document from string content
    // @param jsonFile: Json content string
    bool ParseDocument(const std::string& jsonFileConent);
    void CloseDocument();

private:
    cJSON* mJsonRootObject;
};

//////////////////////////////////////////////////////////////////////////

bool JsonReadValue(const JsonElement& jsonElement, Rect2D& output);
bool JsonReadValue(const JsonElement& jsonElement, Point2D& output);
bool JsonReadValue(const JsonElement& jsonElement, Color32& output);

bool JsonReadValue(const JsonElement& jsonElement, glm::vec2& output);

// Read arrays
// @param jsonElement: Json element reference
// @param numbersArray: Output array
// @param stringArray: Output array
bool JsonReadArray(const JsonElement& jsonElement, std::vector<int>& numbersArray);
bool JsonReadArray(const JsonElement& jsonElement, std::vector<float>& numberArray);
bool JsonReadArray(const JsonElement& jsonElement, std::vector<bool>& booleansArray);
bool JsonReadArray(const JsonElement& jsonElement, std::vector<std::string>& stringArray);

// Read values
// @param jsonElement: Root element
// @param elementName: Element name or path
// @param output: Output
bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, int& output);
bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, float& output);
bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, bool& output);
bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, std::string& output);

// Query value of specified type
template<typename TOutputValue>
inline bool JsonQuery(const JsonElement& jsonElement, const std::string& elementName, TOutputValue& output)
{
    if (JsonElement element = jsonElement.FindElement(elementName))
    {
        return JsonReadValue(element, output);
    }
    return false;
}