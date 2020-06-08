#pragma once

// forwards
class CVarBase;

template<typename TValue>
class ConsoleVariable;

template<typename TValue>
class ConsoleValueHandler;

// list of predefined cvar types
using CvarBoolean   = ConsoleVariable<bool>;
using CvarFloat     = ConsoleVariable<float>;
using CvarInteger   = ConsoleVariable<int>;
using CvarString    = ConsoleVariable<std::string>;
using CvarVector2   = ConsoleVariable<glm::vec2>;
using CvarVector3   = ConsoleVariable<glm::vec3>;
using CvarVector4   = ConsoleVariable<glm::vec4>;
using CvarPoint     = ConsoleVariable<Point>;
using CvarRectangle = ConsoleVariable<Rectangle>;

// console variable flags
enum ConsoleVarFlags
{
    ConsoleVar_System   = (1 << 0),
    ConsoleVar_Renderer = (1 << 1),
    ConsoleVar_Sound    = (1 << 2),
    ConsoleVar_Gui      = (1 << 3),
    ConsoleVar_Game     = (1 << 4),
    ConsoleVar_Scene    = (1 << 5),
    ConsoleVar_Cheat    = (1 << 6),
    ConsoleVar_Const    = (1 << 7), // could be changed only directly from code
    ConsoleVar_Save     = (1 << 8),
    ConsoleVar_Debug    = (1 << 9),
};

decl_enum_bitwise_operators(ConsoleVarFlags)


// array of string arguments passed to console function
struct ConsoleFuncArgs
{
public:
    template<typename TValue>
    inline bool ParseArgument(int argumentIndex, TValue& output) const
    {
        if (argumentIndex < (int) mArgumentsList.size())
        {
            using ValueHandler = ConsoleValueHandler<TValue>;
            return ValueHandler::TryLoadValue(output, mArgumentsList[argumentIndex]);
        }
        return false;
    }
    template<>
    inline bool ParseArgument<std::string>(int argumentIndex, std::string& output) const
    {
        if (argumentIndex < (int) mArgumentsList.size())
        {
            output.assign(mArgumentsList[argumentIndex]);
            return true;
        }
        return false;
    }
public:
    std::vector<std::string> mArgumentsList;
};

// console variable callback
using CvarChagedCallback = std::function<void(CVarBase* cvar)>;

// console function callback
using ConsoleFuncExecuteCallback = std::function<void(const ConsoleFuncArgs& args)>;
