#include "pch.h"
#include "Console.h"

#define SEND_LOG_TO_STDOUT

Console gConsole;

bool Console::Initialize()
{
    return true;
}

void Console::Deinit()
{
    Clear();
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

    LogLineData consoleLine;
        consoleLine.mMessageCategory = cat;
        consoleLine.mMessageString = consoleMessageBuffer;
    mLogLines.push_back(std::move(consoleLine));
}

void Console::ExecuteCommands(const char* commands)
{
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
