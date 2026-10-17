#pragma once

//////////////////////////////////////////////////////////////////////////

class DK2MovieDecoder
{
public:
   
    //////////////////////////////////////////////////////////////////////////

    enum eErrorCode
    {
        eErrorCode_None,
        eErrorCode_FileError,
        eErrorCode_EOF,
        eErrorCode_FrameTooSmall,
        eErrorCode_InvalidFrameDimensions,
        eErrorCode_DecodeError,
    };

    //////////////////////////////////////////////////////////////////////////

    struct AudioInfo
    {
    public:
        int mSampleRate = 0;    // Hz
        int mChannels = 0;      // 1 or 2
        int mFramesCount = 0;   // samples count / channels
        float mDuration = 0.0f; // seconds
    };

    // pcm samples are signed 16-bit, interleaved (L R L R ... for stereo)
    using AudioData = std::vector<int16_t>;

    //////////////////////////////////////////////////////////////////////////

public:
    bool OpenFile(const std::string& path);
    void Close();
    void Rewind();

    // outFrame data is packed RGB24, width * height * 3
    bool DecodeNextFrame(ByteArray& outFrame);

    // reads and decodes the whole audio track (track is 1-based)
    bool DecodeAudio(int track, AudioInfo& audioInfo, AudioData& audioData);

    inline Point2D GetFrameDimensions() const 
    { 
        return mFrameDims; 
    }
    inline eErrorCode GetErrorCode() const { return mErrorCode; }

    inline int GetFrameIndex() const { return mFrameIndex; }
private:

    //////////////////////////////////////////////////////////////////////////
    struct DemuxResult
    {
        const uint8_t* mDataPtr {};
        uint32_t mLength {};
        uint32_t mTag {};
    };
    //////////////////////////////////////////////////////////////////////////

    bool DemuxNext(DemuxResult& outResult);
    bool DecodeTqi(const uint8_t* payload, uint32_t size, ByteArray& out);

private:
    std::ifstream mFileStream;
    std::vector<uint8_t> mDataBuffer;

    eErrorCode mErrorCode {};

    Point2D mFrameDims {};
    Point2D mFrameDimsCoded {};

    int mFrameIndex = 0;
};

//////////////////////////////////////////////////////////////////////////