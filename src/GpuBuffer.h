#pragma once

#include "GraphicsDefs.h"

// Defines hardware buffer object
class GpuBuffer: public cxx::noncopyable
{
    friend class RenderDevice;

private:

    //////////////////////////////////////////////////////////////////////////
    class ScopedBinder
    {
    public:
        ScopedBinder(GpuBuffer* gpuBuffer);
        ~ScopedBinder();
    private:
        GpuBuffer* mPreviousBuffer = nullptr;
        GpuBuffer* mBuffer = nullptr;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    ~GpuBuffer();

    // Will drop buffer data and allocate new chunk of gpu memory
    // @param bufferUsage: Usage hint of buffer
    // @param theLength: Data length
    // @param dataBuffer: Initial data, optional
    // @returns false if out of memory
    bool Create(eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer);

    // Map hardware buffer content to process memory
    // @param contentAccess: Desired data access policy
    // @param bufferOffset: Offset from start, bytes
    // @param dataSize: Size of data, bytes
    // @return Pointer to buffer data or null on fail
    void* Lock(BufferAccessBits contentAccess);
    void* Lock(BufferAccessBits contentAccess, unsigned int bufferOffset, unsigned int dataSize);

    template<typename TElement>
    inline TElement* LockData(BufferAccessBits contentAccess)
    {
        return static_cast<TElement*>(Lock(contentAccess));
    }
    template<typename TElement>
    inline TElement* LockData(BufferAccessBits contentAccess, unsigned int bufferOffset, unsigned int dataSize)
    {
        return static_cast<TElement*>(Lock(contentAccess, bufferOffset, dataSize));
    }

    // Unmap buffer object data source
    // @return false on fail, indicates that buffer should be reload
    bool Unlock();

    // Upload partial buffer content
    // @param bufferOffset: Offset from start, bytes
    // @param dataSize: Size of data, bytes
    // @param sourceContent: Content to upload
    bool SubData(unsigned int bufferOffset, unsigned int dataSize, const void* sourceContent);

    // Test whether index or vertex attributes data is currently bound - depends on content
    bool IsBufferBound() const;

protected:
    // ctor
    GpuBuffer(eBufferTarget bufferTarget);

private:
    // @return Previous bound buffer or null if none was bound
    GpuBuffer* BindBuffer();
    void UnBindBuffer();

private:
    // shared render device context data
    static GpuBuffer* sCurrentBuffers[eBufferTarget_COUNT];

private:
    GpuResourceHandle mResourceHandle;
    eBufferTarget mTarget;
    eBufferUsage mUsageHint;
    unsigned int mBufferLength; // user requested length in bytes
    unsigned int mBufferCapacity; // actually allocated length int bytes
};

//////////////////////////////////////////////////////////////////////////

class GpuVertexBuffer: public GpuBuffer
{
public:
    GpuVertexBuffer()
        : GpuBuffer(eBufferTarget_Attributes)
        , mVertexDefinition()
    {
    }
    inline void ConfigureVertexDefinitions(const VertexFormat& vertexDefinition)
    {
        mVertexDefinition.ConfigureWith(vertexDefinition);
    }

    const VertexFormat& GetVertexDefinition() const { return mVertexDefinition; }
private:
    VertexFormat mVertexDefinition;
};

//////////////////////////////////////////////////////////////////////////

class GpuIndexBuffer: public GpuBuffer
{
public:
    GpuIndexBuffer()
        : GpuBuffer(eBufferTarget_Indices)
    {
    }
};

//////////////////////////////////////////////////////////////////////////