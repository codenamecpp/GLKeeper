#pragma once

#include "CommonTypes.h"

// console back-end
// it is used for debug logging, commands execute and set game variables in runtime
class Console: public cxx::noncopyable
{
public:

    // setup console internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // set limit on log messages
    // @param messagesCount: Max messages in log, 0 disables current limit
    void SetLogMessagesLimit(int messagesCount);

    // send formatted log messages to system console
    // @param cat: Message category
    // @param format: String format
    void LogMessage(eLogMessage cat, const char* format, ...);

    // clear all console log messages
    void Clear();

    // parse and execute commands
    // @param commands: Commands string
    void ExecuteCommands(const char* commands);

public:

    struct LineStruct
    {
        eLogMessage mMessageCategory;
        std::string mMessageString;
    };

    std::deque<LineStruct> mLogLines;
    int mMaxLogLines = 0; // no limits default
};

extern Console gConsole;