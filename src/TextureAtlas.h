#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AssetDefs.h"

//////////////////////////////////////////////////////////////////////////

class TextureAtlas final: public cxx::noncopyable
{
    friend class TextureManager;

    //////////////////////////////////////////////////////////////////////////

    class GuillotinePacker
    {
    public:
        void Setup(const Point2D& rectSize, int padding, int alignment);
        void Clear();
        bool AllocateRect(const Point2D& rectSize, Rect2D& outRect);
        inline int GetPadding() const { return mPadding; }
    private:
        void SplitFreeRect(const Rect2D& freeRect, const Rect2D& placed);
    private:
        std::vector<Rect2D> mFreeRects;
        int mAlignment = 0;
        int mPadding = 0;
        int mAllocated = 0;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    TextureAtlas(TextureSourceId textureSourceId);

    bool Setup(const Point2D& textureDimensions, ePixelFormat pixelFormat, int numMipmaps);
    void Purge();
    bool TryAppendTexture(BitmapImage& bitmapImage, TextureRegion& region);
    void BindTexture(eTextureUnit textureUnit);
    void EnsureRenderDataInited();

    inline ePixelFormat GetPixelFormat() const { return mPixelFormat; }
    inline GpuTexture2D* GetRenderResource() const 
    { 
        return mGpuTextureResource.get(); 
    }
    inline TextureSourceId GetTextureSourceId() const { return mTextureSourceId; }

private:
    void UpdateRenderData();

private:
    GuillotinePacker mRectsPacker;

    std::list<std::pair<Rect2D, BitmapImage>> mUpdateRects;

    Point2D mTextureDimensions {};

    std::unique_ptr<GpuTexture2D> mGpuTextureResource;
    ePixelFormat mPixelFormat {};
    TextureSourceId mTextureSourceId {};

    int mMipMapsCount = 0;
};

//////////////////////////////////////////////////////////////////////////