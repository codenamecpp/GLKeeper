#pragma once

//////////////////////////////////////////////////////////////////////////

using snd_group_id = unsigned int; // 1 based
using snd_clip_idx = unsigned int; // 0 based

//////////////////////////////////////////////////////////////////////////

using AudioHandle = unsigned int;

//////////////////////////////////////////////////////////////////////////

enum eAudioChannel
{
    eAudioChannel_Master,
    eAudioChannel_Music,
    eAudioChannel_Speech,
    eAudioChannel_Sfx,
};

//////////////////////////////////////////////////////////////////////////

// generic sound groups

enum : unsigned int
{
    SoundGroupId_Null = 0,

    // ambience
    SoundGroupId_Ambience_Track = 341,
    // frontend
    SoundGroupId_Frontend_MenuClick = 778,
    // frontend ext
    SoundGroupId_FrontendExt_BigButtonClick = 781,
    // gui button icon
    SoundGroupId_GuiButtonIcon_ButtonClick = 260,
    // gui sell
    SoundGroupId_GuiSell_ButtonClick = 260,

};

//////////////////////////////////////////////////////////////////////////