#include "pch.h"
#include "Console.h"
#include "ToolsUIConsoleWindow.h"
#include "ToolsUIManager.h"
#include "ConsoleVariable.h"
#include "System.h"

#define SEND_LOG_TO_STDOUT

Console gConsole;

bool Console::Initialize()
{
    RegisterStandardFunctions();

    gToolsUIManager.AttachWindow(&gConsoleWindow);
    return true;
}

void Console::Deinit()
{
    gToolsUIManager.DetachWindow(&gConsoleWindow);

    Clear();

    mConsoleVariables.clear();
    mConsoleFunctions.clear();
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
        // function
        else if (FunctionStruct* func = GetFunction(commandName))
        {
            LogMessage(eLogMessage_Info, "%s (%s)", commandName.c_str(), commandParams.c_str());

            if (func->mExecuteCallback)
            {
                ConsoleFuncArgs functionArgs;
                if (!commandParams.empty())
                {
                    std::string stringBuffer;
                    for (cxx::string_tokenizer tokenizer(commandParams);;)
                    {
                        if (tokenizer.get_next(stringBuffer, ','))
                        {
                            functionArgs.mArgumentsList.push_back(stringBuffer);
                            continue;
                        }
                        break;
                    }
                }
                func->mExecuteCallback(functionArgs);
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

Console::FunctionStruct* Console::GetFunction(const std::string& funcName)
{
    for (FunctionStruct& currStruct: mConsoleFunctions)
    {
        if (currStruct.mName == funcName)
            return &currStruct;
    }
    return nullptr;
}

void Console::RegisterStandardFunctions()
{
    // clear
    RegisterFunction("clear", "Clear console", [](const ConsoleFuncArgs& args)
        {
            gConsole.Clear();
        });

    // quit
    RegisterFunction("quit", "Exit application", [](const ConsoleFuncArgs& args)
        {
            gSystem.QuitRequest();
        });

    // desc
    RegisterFunction("desc", "Show variable or function info", [](const ConsoleFuncArgs& args)
        {
            std::string identifier;
            if (!args.ParseArgument(0, identifier))
            {
                gConsole.LogMessage(eLogMessage_Warning, "Variable or Function name expected");
                return;
            }

            if (CVarBase* cvar = gConsole.GetVariableByName(identifier))
            {
                gConsole.LogMessage(eLogMessage_Debug, "%s", cvar->mDescription.c_str());
                return;
            }

            if (Console::FunctionStruct* func = gConsole.GetFunction(identifier))
            {
                gConsole.LogMessage(eLogMessage_Debug, "%s", func->mDescription.c_str());
                return;
            }
        });
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

void Console::RegisterFunction(const std::string& funcName, const std::string& funcDesc, ConsoleFuncExecuteCallback callback)
{
    UnregisterFunction(funcName);

    mConsoleFunctions.emplace_back(funcName, funcDesc, callback);
}

void Console::UnregisterFunction(const std::string& funcName)
{
    cxx::erase_elements_if(mConsoleFunctions, [&funcName](const FunctionStruct& funcStruct)
        {
            return funcName == funcStruct.mName;
        });
}

bool Console::IsFunctionRegistered(const std::string& funcName) const
{
    return cxx::contains_if(mConsoleFunctions, [&funcName](const FunctionStruct& funcStruct)
        {
            return funcName == funcStruct.mName;
        });
}
