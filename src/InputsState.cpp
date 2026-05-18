#include "stdafx.h"
#include "InputsState.h"

//////////////////////////////////////////////////////////////////////////

InputsState gInputs;

//////////////////////////////////////////////////////////////////////////

InputsState::InputsState()
{
    Cleanup();
}

void InputsState::Cleanup()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
    mCursorPositionX = 0;
    mCursorPositionY = 0;
}
