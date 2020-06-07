#pragma once

// console variable base class, not intended for direct usage
class CVarBase
{
public:
    // readonly
    std::string mName;
    std::string mDescription;
    int mFlags;

public:
    // get some cvar flags
    inline bool IsConst() const { return (mFlags & ConsoleVar_Const) == ConsoleVar_Const; }
    inline bool IsCheat() const { return (mFlags & ConsoleVar_Cheat) == ConsoleVar_Cheat; }

    // set function which be invoked each time cvar value changes
    inline void SetValueChangedCallback(CvarChagedCallback callback)
    {
        mValueChangedCb = callback;
    }

    // try set cvar value from console
    virtual bool SetValueFromString(const std::string& consoleInput) = 0;

    // dump cvar value for console
    virtual void GetValueString(std::string& outputString) = 0;

protected:
    CVarBase(const std::string& cvarName, const std::string& cvarDescription, int flags)
        : mName(cvarName)
        , mDescription(cvarDescription)
        , mFlags(flags)
    {
    }
    virtual ~CVarBase()
    {
    }
    // invoke callback on cvar value changed
    inline void OnCvarValueChanged()
    {
        if (mValueChangedCb)
        {
            mValueChangedCb(this);
        }
    }
protected:
    CvarChagedCallback mValueChangedCb;
};

//////////////////////////////////////////////////////////////////////////

// generic console variable implementation
template<typename TCvarValue, typename TCvarValueHandler>
class ConsoleVariable final: public CVarBase
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
    bool SetValueFromString(const std::string& consoleInput) override
    {
        TCvarValue temporaryValue;
        if (TCvarValueHandler::TryLoadValue(temporaryValue, consoleInput))
        {
            if (TCvarValueHandler::ValidateValue(temporaryValue))
            {
                if (mValue == temporaryValue)
                    return true;

                mValue = std::move(temporaryValue);

                OnCvarValueChanged();
                return true;
            }
        }
        return false;
    }

    // dump cvar value for console
    void GetValueString(std::string& outputString) override
    {
        outputString.clear();
        TCvarValueHandler::SerializeValue(mValue, outputString);
    }

    // set value directly from code
    inline bool SetValue(const TCvarValue& cvarValue)
    {
        if (TCvarValueHandler::ValidateValue(cvarValue))
        {
            if (mValue == cvarValue) // same value
                return true;

            mValue = cvarValue;

            OnCvarValueChanged();
            return true;
        }
        return false;
    }

    // test whether cvar value was changed
    inline bool IsValueChanged() const { return mValue != mDefaultValue; }
};

#include "ConsoleVariable.inl"