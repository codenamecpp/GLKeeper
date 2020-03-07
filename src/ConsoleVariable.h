#pragma once

#include "ConsoleDefs.h"

// console variable base class, not intended for direct usage
class CVarBase
{
public:
    // readonly
    std::string mName;
    std::string mDescription;
    int mFlags;

public:
    CVarBase( const std::string& cvarName, const std::string& cvarDescription, int flags)
        : mName(cvarName)
        , mDescription(cvarDescription)
        , mFlags(flags)
    {
    }
    virtual ~CVarBase()
    {
    }

    // get some cvar flags
    inline bool IsConst() const { return (mFlags & ConsoleVar_Const) == ConsoleVar_Const; }
    inline bool IsCheat() const { return (mFlags & ConsoleVar_Cheat) == ConsoleVar_Cheat; }

    // try set cver value from console
    virtual bool SetValueFromConsole(const std::string& consoleInput) = 0;

protected:
    // internal stuff
    static unsigned int mTypeIDCounter;
};

//////////////////////////////////////////////////////////////////////////

// generic console variable implementation
template<typename TCvarValue, typename TCvarValueHandler>
class ConsoleVariable: public CVarBase
{
public:
    // readonly
    TCvarValue mValue;
    TCvarValue mDefaultValue;

public:
    ConsoleVariable(const std::string& cvarName, TCvarValue&& cvarValue, const std::string& cvarDescription, int flags)
        : CVarBase(cvarName, cvarDescription, flags)
        , mValue(cvarValue)
        , mDefaultValue(cvarValue)
    {
        TCvarValueHandler::ValidateValue(cvarValue);
    }

    // set value from console
    bool SetValueFromConsole(const std::string& consoleInput) override
    {
        TCvarValue temporaryValue;
        if (TCvarValueHandler::TryLoadValue(temporaryValue, consoleInput))
        {
            if (TCvarValueHandler::ValidateValue(temporaryValue))
            {
                if (mValue == temporaryValue)
                    return true;

                mValue = std::move(temporaryValue);
                return true;
            }
        }
        return false;
    }

    // set value directly from code
    inline bool SetValue(TCvarValue&& cvarValue)
    {
        if (TCvarValueHandler::ValidateValue(cvarValue))
        {
            if (mValue == cvarValue) // same value
                return true;

            mValue = std::move(cvarValue);
            return true;
        }
        return false;
    }

    // test whether cvar value was changed
    inline bool IsValueChanged() const { return mValue != mDefaultValue; }

private:
    // internal
    static unsigned int mUniqueTypeID;
};

#include "ConsoleVariable.inl"