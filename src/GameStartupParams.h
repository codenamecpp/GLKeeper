#pragma once

//////////////////////////////////////////////////////////////////////////

struct GameStartupParams
{
public:
    GameStartupParams() = default;

    inline void Clear()
    {
        mLoadLevelName.clear();
        mNoSound = {};
        mDevScreen = {};
    }

public:
    std::string mLoadLevelName;

    bool mNoSound = false;
    bool mDevScreen = false;
};

//////////////////////////////////////////////////////////////////////////