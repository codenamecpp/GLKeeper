#pragma once

#include "CommonTypes.h"

// forwards
class CVarBase;

template<typename TCvarValue, typename TCvarValueHandler>
class ConsoleVariable;

class CvarBooleanValueHandler;
using CvarBoolean = ConsoleVariable<bool, CvarBooleanValueHandler>;

class CvarFloatValueHandler;
using CvarFloat = ConsoleVariable<float, CvarFloatValueHandler>;

class CvarIntegerValueHandler;
using CvarInteger = ConsoleVariable<int, CvarIntegerValueHandler>;

class CvarStringValueHandler;
using CvarString = ConsoleVariable<std::string, CvarStringValueHandler>;

class CvarVector2ValueHandler;
using CvarVector2 = ConsoleVariable<glm::vec2, CvarVector2ValueHandler>;

class CvarVector3ValueHandler;
using CvarVector3 = ConsoleVariable<glm::vec3, CvarVector3ValueHandler>;

class CvarVector4ValueHandler;
using CvarVector4 = ConsoleVariable<glm::vec4, CvarVector4ValueHandler>;

class CvarSize2DValueHandler;
using CvarSize2D = ConsoleVariable<Size2D, CvarSize2DValueHandler>;

class CvarRect2DValueHandler;
using CvarRect2D = ConsoleVariable<Rect2D, CvarRect2DValueHandler>;

// console variable flags
enum 
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

