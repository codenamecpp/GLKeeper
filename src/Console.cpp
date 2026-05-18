#include "stdafx.h"
#include "Console.h"

//////////////////////////////////////////////////////////////////////////

enum
{
    SYSTEM_CONSOLE_MAX_RECORDS = 2048
};

#define VA_SCOPE_OPEN(firstArg, vaName) \
    { \
        va_list vaName {}; \
        va_start(vaName, firstArg); \

#define VA_SCOPE_CLOSE(vaName) \
        va_end(vaName); \
    }

//////////////////////////////////////////////////////////////////////////

Console gConsole;

//////////////////////////////////////////////////////////////////////////

bool Console::Initialize()
{
    mConsoleListeners.RemoveListeners();

    return true;
}

void Console::Shutdown()
{
    mRecords.clear();
    mConsoleListeners.RemoveListeners();
}

void Console::Subscribe(IConsoleListener* listener)
{
    cxx_assert(listener);
    if (listener)
    {
        mConsoleListeners.AddListener(listener);
    }
}

void Console::Unsubscribe(IConsoleListener* listener)
{
    cxx_assert(listener);
    if (listener)
    {
        mConsoleListeners.RemoveListener(listener);
    }
}

void Console::LogMessage(eLogLevel logLevel, const wchar_t* format, ...)
{
    static wchar_t MessageBuffer[1024];

    VA_SCOPE_OPEN(format, vaList)
    std::vswprintf(MessageBuffer, sizeof(MessageBuffer), format, vaList);
    VA_SCOPE_CLOSE(vaList)

    wprintf(L"%s\n", MessageBuffer);

    Record& rec = AddMessageRecord(logLevel);
    rec.mMessage.assign(MessageBuffer, MessageBuffer + wcslen(MessageBuffer));
    PostAddMessageRecords();
}

void Console::LogMessage(eLogLevel logLevel, const char* format, ...)
{
    static char MessageBuffer[1024];

    VA_SCOPE_OPEN(format, vaList)
    std::vsnprintf(MessageBuffer, sizeof(MessageBuffer), format, vaList);
    VA_SCOPE_CLOSE(vaList)

    printf("%s\n", MessageBuffer);

    Record& rec = AddMessageRecord(logLevel);
    rec.mMessage.assign(MessageBuffer, MessageBuffer + strlen(MessageBuffer));
    PostAddMessageRecords();
}

void Console::Flush()
{
    mRecords.clear();
}

Console::Record& Console::AddMessageRecord(eLogLevel logLevel)
{
    while (mRecords.size() >= SYSTEM_CONSOLE_MAX_RECORDS)
    {
        mRecords.pop_front();
    }

    Record& rec = mRecords.emplace_back();
    rec.mLogLevel = logLevel;
    return rec;
}

void Console::PostAddMessageRecords()
{
    // notify listeners

    mConsoleListeners.IterateListeners([this](IConsoleListener* listener)
        {
            listener->OnConsoleMessagesAdded(this);
        });
}