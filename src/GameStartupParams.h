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
        mNoIntro = {};
        mDevScreen = {};
    }

public:
    std::string mLoadLevelName;

    bool mNoSound = false;
    bool mNoIntro = false;
    bool mDevScreen = false;
};

//////////////////////////////////////////////////////////////////////////