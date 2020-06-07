#pragma once

//////////////////////////////////////////////////////////////////////////

class CvarBooleanValueHandler
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

class CvarFloatValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarIntegerValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarStringValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarVector2ValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarVector3ValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarVector4ValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarPointValueHandler
{
public:
};

//////////////////////////////////////////////////////////////////////////

class CvarRectangleValueHandler
{
public:
};
