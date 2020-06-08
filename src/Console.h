#pragma once

// console back-end
// it is used for debug logging, commands execute and set game variables in runtime
class Console: public cxx::noncopyable
{
    friend class ToolsUIConsoleWindow;

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

    // manage console variables
    bool RegisterVariable(CVarBase* consoleVariable);
    bool UnregisterVariable(CVarBase* consoleVariable);
    void UnregisterAllVariablesWithFlags(int flags);

    // manage console functions
    void RegisterFunction(const std::string& funcName, const std::string& funcDesc, ConsoleFuncExecuteCallback callback);
    void UnregisterFunction(const std::string& funcName);
    bool IsFunctionRegistered(const std::string& funcName) const;

    // parse and execute commands
    // @param commands: Commands string
    void ExecuteCommands(const char* commands);

    // find registered console variable by name
    CVarBase* GetVariableByName(const std::string& cvarNamee) const;

private:
    struct FunctionStruct;
    FunctionStruct* GetFunction(const std::string& funcName);

    void RegisterStandardFunctions();

private:
    struct LineStruct
    {
        eLogMessage mMessageCategory;
        std::string mMessageString;
    };

    std::deque<LineStruct> mLogLines;
    int mMaxLogLines = 0; // no limits default

    // registered console variables
    std::vector<CVarBase*> mConsoleVariables;

    struct FunctionStruct
    {
    public:
        FunctionStruct(const std::string& funcName, const std::string& funcDesc, ConsoleFuncExecuteCallback executeCallback)
            : mName(funcName)
            , mDescription(funcDesc)
            , mExecuteCallback(executeCallback)
        {
        }
    public:
        std::string mName;
        std::string mDescription;
        ConsoleFuncExecuteCallback mExecuteCallback;
    };

    // registered console functions
    std::vector<FunctionStruct> mConsoleFunctions;
};

extern Console gConsole;