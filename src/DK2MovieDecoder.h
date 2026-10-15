#pragma once

//////////////////////////////////////////////////////////////////////////

class DK2MovieDecoder
{
public:
   
    //////////////////////////////////////////////////////////////////////////

    enum eCodec { eCodec_Unknown, eCodec_Tqi };
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

public:
    bool OpenFile(const std::string& path);
    void Close();
    void Rewind();

    // outFrame data is packed RGB24, width * height * 3
    bool DecodeNextFrame(ByteArray& outFrame);

    inline Point2D GetFrameDimensions() const 
    { 
        return mFrameDims; 
    }
    inline eErrorCode GetErrorCode() const { return mErrorCode; }
    inline eCodec GetCodec() const { return mCodec; }

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
    std::vector<uint8_t> mFileData;
    uint32_t mFilePosition = 0;

    eCodec mCodec {};
    eErrorCode mErrorCode {};

    Point2D mFrameDims {};
    Point2D mFrameDimsCoded {};

    int mFrameIndex = 0;
};

//////////////////////////////////////////////////////////////////////////