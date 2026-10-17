#include "stdafx.h"
#include "MovieScreen.h"
#include "GpuTexture2D.h"
#include "UiRenderContext.h"
#include "AudioManager.h"

//////////////////////////////////////////////////////////////////////////

#define MOVIE_PFS               25.0f
#define MOVIE_FRAME_DURATION    (1.0f / MOVIE_PFS)

//////////////////////////////////////////////////////////////////////////

MovieScreen::MovieScreen()
    : UiView(eUiViewLayer_Overlay)
{
}

bool MovieScreen::ShowMovie(const std::string& movieName)
{
    if (!Activate())
    {
        cxx_assert(false);
        return false;
    }

    ClearState();

    std::string filePath;
    if (!gFiles.LocateMovie(movieName, filePath))
        return false;

    if (!mMovieDecoder.OpenFile(filePath) || 
        !mMovieDecoder.DecodeNextFrame(mRgbData))
    {
        cxx_assert(false);
        return false;
    }

    if (mTexture == nullptr)
    {
        mTexture = gRenderDevice.CreateTexture2D();
        cxx_assert(mTexture);
    }

    mMvFrameDimensions = mMovieDecoder.GetFrameDimensions();
    mTextureDimensions = {
        cxx::get_next_pot(mMvFrameDimensions.x),
        cxx::get_next_pot(mMvFrameDimensions.y)
    };

    if (mTexture)
    {
        if (!mTexture->Create(mTextureDimensions, ePixelFormat_RGB8, nullptr))
        {
            cxx_assert(false);
        }
    }

    // audio
    if (gAudio.IsAudioOnline())
    {
        DK2MovieDecoder::AudioData audioData;
        DK2MovieDecoder::AudioInfo audioInfo;
        if (mMovieDecoder.DecodeAudio(1, audioInfo, audioData))
        {
            mAudioHandle = gAudio.PlayOneShotPcm(eAudioChannel_Master, audioInfo.mSampleRate, audioInfo.mChannels, std::move(audioData));
            cxx_assert(mAudioHandle != 0);
        }
    }
    mPlaybackTime = 0.0f;
    mFrameDuration = MOVIE_FRAME_DURATION;
    mPlaybackState = ePlaybackState_Playing;
    return true;
}

void MovieScreen::PauseMovie(bool isPaused)
{
    // todo
}

void MovieScreen::ExitMovie()
{
    Deactivate();

    ClearState();
}

bool MovieScreen::IsPlaying() const
{
    return (mPlaybackState == ePlaybackState_Playing) || (mPlaybackState == ePlaybackState_Paused);
}

bool MovieScreen::IsPaused() const
{
    return (mPlaybackState == ePlaybackState_Paused);
}

bool MovieScreen::LoadContent()
{
    return true;
}

void MovieScreen::Cleanup()
{
    UiView::Cleanup();

    ClearState();
}

void MovieScreen::UpdateFrame(float deltaTime)
{
    UiView::UpdateFrame(deltaTime);

    if (mPlaybackState == ePlaybackState_Playing)
    {
        mPlaybackTime += deltaTime;

        // sync with audio
        if (mAudioHandle)
        {
            double audioTime {};
            if (gAudio.GetPlaybackTime(mAudioHandle, audioTime))
            {
                const double threshold = 0.1;
                if ((mPlaybackTime < (audioTime - threshold)) ||
                    (mPlaybackTime > (audioTime + threshold)))
                {
                    mPlaybackTime = static_cast<float>(audioTime);
                }
            }
        }

        int currentFrame = static_cast<int>(std::round(mPlaybackTime / mFrameDuration));
        bool frameChanged = false;
        while (mMovieDecoder.GetFrameIndex() < currentFrame)
        {
            if (!mMovieDecoder.DecodeNextFrame(mRgbData))
            {
                mPlaybackState = ePlaybackState_Stopped;
                break;
            }
            frameChanged = true;
        }

        if (frameChanged)
        {
            UpdateFrameTexture();
        }
    }
}

void MovieScreen::RenderFrame(UiRenderContext& renderContext)
{
    UiView::RenderFrame(renderContext);

    const Rect2D& screenRect = renderContext.GetScreenRect();

    renderContext.FillRect(screenRect, COLOR_BLACK);

    // enforce 4:3 aspect ratio
    static const float targetRatio = 4.0f / 3.0f;
    static const float targetRatioInv = 1.0f / targetRatio;

    if (mTexture)
    {
        Rect2D dstRect;

        const int expectWidth = static_cast<int>(screenRect.h * targetRatio + 0.5f);
        if (expectWidth > screenRect.w)
        {
            const int expectHeight = static_cast<int>(screenRect.w * targetRatioInv + 0.5f);
            dstRect.w = screenRect.w;
            dstRect.h = expectHeight;
        }
        else
        {
            dstRect.w = expectWidth;
            dstRect.h = screenRect.h;
        }

        dstRect.x = (screenRect.w >> 1) - (dstRect.w >> 1);
        dstRect.y = (screenRect.h >> 1) - (dstRect.h >> 1);

        const Rect2D srcRect {0, 0, mMvFrameDimensions.x, mMvFrameDimensions.y};
        renderContext.DrawTexture(mTexture.get(), COLOR_WHITE, dstRect, srcRect);
    }
}

void MovieScreen::InputEvent(MouseButtonInputEvent& inputEvent)
{
    UiView::InputEvent(inputEvent);

    if (inputEvent.mPressed)
    {
        mPlaybackState = ePlaybackState_Stopped;
        inputEvent.SetConsumed(true);
    }
}

void MovieScreen::InputEvent(KeyInputEvent& inputEvent)
{
    UiView::InputEvent(inputEvent);

    if (inputEvent.mPressed)
    {
        mPlaybackState = ePlaybackState_Stopped;
        inputEvent.SetConsumed(true);
    }
}

void MovieScreen::UpdateFrameTexture()
{
    if (mTexture)
    {
        mTexture->Upload({0, 0}, mMvFrameDimensions, mRgbData.data(), 0);
    }
}

void MovieScreen::ClearState()
{
    if (mAudioHandle)
    {
        gAudio.StopPlayback(mAudioHandle);
        mAudioHandle = {};
    }
    mMovieDecoder.Close();
    mTexture.reset();
    mRgbData.clear();
    mTextureDimensions = {};
    mMvFrameDimensions = {};
    mPlaybackTime = {};
    mFrameDuration = {};
    mPlaybackState = {};
}
