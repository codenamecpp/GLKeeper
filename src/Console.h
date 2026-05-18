#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ListenersContainer.h"

//////////////////////////////////////////////////////////////////////////

class Console;

//////////////////////////////////////////////////////////////////////////

// Defines system console listener interface
class IConsoleListener
{
public:
    virtual ~IConsoleListener() {}
    // @param sender: Console instance
    virtual void OnConsoleMessagesAdded(Console* sender) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Represents console system that handles debug commands
class Console
{
public:
    //////////////////////////////////////////////////////////////////////////
    // single message record on console
    struct Record
    {
        eLogLevel mLogLevel;
        std::wstring mMessage;
    };
    //////////////////////////////////////////////////////////////////////////
public:
    // one-time initialization/deinitialization
    bool Initialize();
    void Shutdown();

    // Add or Remove console listeners
    void Subscribe(IConsoleListener* listener);
    void Unsubscribe(IConsoleListener* listener);

    // Write text message in console
    void LogMessage(eLogLevel logLevel, const wchar_t* format, ...);
    void LogMessage(eLogLevel logLevel, const char* format, ...);

    // Clear all console text messages
    void Flush();

private:
    // insert new console message
    Record& AddMessageRecord(eLogLevel logLevel);
    void PostAddMessageRecords();

public:
    std::deque<Record> mRecords;
    ListenersContainer<IConsoleListener> mConsoleListeners;
};

//////////////////////////////////////////////////////////////////////////

extern Console gConsole;

//////////////////////////////////////////////////////////////////////////