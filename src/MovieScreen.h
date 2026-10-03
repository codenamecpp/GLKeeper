#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "DK2MovieDecoder.h"

//////////////////////////////////////////////////////////////////////////

class MovieScreen: public UiView
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    enum ePlaybackState
    {
        ePlaybackState_Stopped,
        ePlaybackState_Playing,
        ePlaybackState_Paused,
    };

    //////////////////////////////////////////////////////////////////////////

public:
    MovieScreen();

    bool ShowMovie(const std::string& movieName);
    void PauseMovie(bool isPaused);
    void ExitMovie();
    bool IsPlaying() const;
    bool IsPaused() const;

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void UpdateFrame(float deltaTime) override;
    void RenderFrame(UiRenderContext& renderContext) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(KeyInputEvent& inputEvent) override;

private:
    void UpdateFrameTexture();
    void ClearState();

private:
    DK2MovieDecoder mMovieDecoder;

    ePlaybackState mPlaybackState {};

    std::shared_ptr<GpuTexture2D> mTexture;
    std::vector<uint8_t> mRgbData;
    Point2D mTextureDimensions {};
    Point2D mMvFrameDimensions {};
    float mPlaybackTime {};
    float mFrameDuration {};
};

//////////////////////////////////////////////////////////////////////////