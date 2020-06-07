#include "pch.h"
#include "Console.h"
#include "ToolsUIConsoleWindow.h"
#include "ToolsUIManager.h"
#include "ConsoleVariable.h"

#define SEND_LOG_TO_STDOUT

Console gConsole;

bool Console::Initialize()
{
    gToolsUIManager.AttachWindow(&gConsoleWindow);
    return true;
}

void Console::Deinit()
{
    gToolsUIManager.DetachWindow(&gConsoleWindow);

    Clear();

    mConsoleVariables.clear();
}

void Console::LogMessage(eLogMessage cat, const char* format, ...)
{
    static char consoleMessageBuffer[2048];

    {
        va_list vaList { };
        va_start(vaList, format);
            vsnprintf(consoleMessageBuffer, sizeof(consoleMessageBuffer), format, vaList);
        va_end(vaList);
    }

#ifdef SEND_LOG_TO_STDOUT 
    printf("%s\n", consoleMessageBuffer);
#endif

    LineStruct consoleLine;
        consoleLine.mMessageCategory = cat;
        consoleLine.mMessageString = consoleMessageBuffer;
    mLogLines.push_back(std::move(consoleLine));
}

void Console::ExecuteCommands(const char* commands)
{
    cxx::string_tokenizer tokenizer(commands);
    for (;;)
    {
        std::string commandName;
        if (!tokenizer.get_next(commandName, ' '))
            break;

        std::string commandParams;
        if (tokenizer.get_next(commandParams, ';'))
        {
            cxx::trim(commandParams);
        }

        cxx::trim(commandName);

        // cvar
        if (CVarBase* cvar = GetVariableByName(commandName))
        {
            if (commandParams.empty())
            {
                // just print current value and info
                std::string cvarValue;
                cvar->GetValueString(cvarValue);
                LogMessage(eLogMessage_Info, "'%s' is '%s' (%s)", cvar->mName.c_str(), cvarValue.c_str(), cvar->mDescription.c_str());
            }
            else
            {
                // set new value
                if (cvar->SetValueFromString(commandParams))
                {
                    LogMessage(eLogMessage_Info, "New value is '%s'", commandParams.c_str());
                    
                }
                else
                {
                    LogMessage(eLogMessage_Warning, "Invalid value '%s'", commandParams.c_str());
                }
            }
        }
        else
        {
            LogMessage(eLogMessage_Warning, "Unknown command %s", commandName.c_str());
        }
    }
}

CVarBase* Console::GetVariableByName(const std::string& cvarNamee) const
{
    for (CVarBase* currCvar: mConsoleVariables)
    {
        if (currCvar->mName == cvarNamee)
            return currCvar;
    }
    return nullptr;
}

void Console::Clear()
{
    mLogLines.clear();
}

void Console::SetLogMessagesLimit(int messagesCount)
{
    if (messagesCount > 0 && messagesCount != mMaxLogLines)
    {
        int currentLinesCount = (int) mLogLines.size();
        if (currentLinesCount > messagesCount)
        {
            mLogLines.erase(mLogLines.begin(), mLogLines.begin() + (currentLinesCount - messagesCount));
        }
    }

    mMaxLogLines = messagesCount;
}

bool Console::RegisterVariable(CVarBase* consoleVariable)
{
    if (consoleVariable == nullptr)
    {
        debug_assert(consoleVariable);
        return false;
    }

    UnregisterVariable(consoleVariable);

    mConsoleVariables.push_back(consoleVariable);
    return true;
}

bool Console::UnregisterVariable(CVarBase* consoleVariable)
{
    auto remove_itarator = std::find(mConsoleVariables.begin(), mConsoleVariables.end(), consoleVariable);
    if (remove_itarator != mConsoleVariables.end())
    {
        mConsoleVariables.erase(remove_itarator);
        return true;
    }
    return false;
}

void Console::UnregisterAllVariablesWithFlags(int flags)
{
    auto remove_iterator = std::remove_if(mConsoleVariables.begin(), mConsoleVariables.end(), 
        [flags](const CVarBase* currCvar)
        {
            return (currCvar->mFlags & flags) > 0;
        });
    
    if (remove_iterator != mConsoleVariables.end())
    {
        mConsoleVariables.erase(remove_iterator, mConsoleVariables.end());
    }
}