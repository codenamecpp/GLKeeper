#pragma once

// forwards
class ModelAsset;
class Texture2D;
class Texture2DAnimation;

enum eAnimationStatus
{
    eAnimationStatus_Stop,
    eAnimationStatus_PlayForward,
    eAnimationStatus_PlayBackward,
};

enum eAnimationLoop
{
    eAnimationLoop_None,
    eAnimationLoop_PingPong,
    eAnimationLoop_FromStart,
};