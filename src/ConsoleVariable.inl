#pragma once

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<bool>
{
public:
    static bool ValidateValue(bool value)
    {
        return value == true || value == false;
    }
    static void SerializeValue(bool value, std::string& outputData)
    {
        if (value)
        {
            outputData.assign("true");
        }
        else
        {
            outputData.assign("false");
        }
    }
    static bool TryLoadValue(bool& outputValue, const std::string& inputData)
    {
        if (inputData == "true" || inputData == "1")
        {
            outputValue = true;
            return true;
        }
        if (inputData == "false" || inputData == "0")
        {
            outputValue = false;
            return true;
        }
        return false;
    }
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<float>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<int>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<std::string>
{
public:
    static bool ValidateValue(const std::string& value)
    {
        return true;
    }
    static void SerializeValue(const std::string& value, std::string& outputData)
    {
        outputData.assign(value);
    }
    static bool TryLoadValue(std::string& outputValue, const std::string& inputData)
    {
        outputValue.assign(inputData);
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<glm::vec2>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<glm::vec3>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<glm::vec4>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<Point>
{
public:
};

//////////////////////////////////////////////////////////////////////////

template<>
class ConsoleValueHandler<Rectangle>
{
public:
};
