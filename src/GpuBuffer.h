#pragma once

#include "GraphicsDefs.h"

// defines hardware vertices/indices buffer object
class GpuBuffer: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    // readonly
    eBufferContent mContent;
    eBufferUsage mUsageHint;
    unsigned int mBufferLength; // user requested length, bytes
    unsigned int mBufferCapacity; // actually allocated length, bytes

public:
    // @param bufferContent: Content type stored in buffer, cannot be changed 
    GpuBuffer(GraphicsDeviceContext& graphicsContext, eBufferContent bufferContent);
    ~GpuBuffer();

    // will drop buffer data and allocate new chunk of gpu memory
    // @param bufferUsage: Usage hint of buffer
    // @param theLength: Data length
    // @param dataBuffer: Initial data, optional
    // @returns false if out of memory
    bool Setup(eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer);

    // upload source data to buffer replacing old content
    // @param dataOffset: Offset within buffer to write in bytes
    // @param dataLength: Size of data to write in bytes
    // @param dataSource: Source data
    bool SubData(unsigned int dataOffset, unsigned int dataLength, const void* dataSource);

    // resize buffer and save all uploaded data, works only for growth and does nothing if new size less then current capacity
    // @param newLength: New buffer size
    bool Resize(unsigned int newLength);

    // map hardware buffer content to process memory
    // @param accessBits: Desired data access policy
    // @param bufferOffset: Offset from start, bytes
    // @param dataSize: Size of data, bytes
    // @return Pointer to buffer data or null on fail
    void* Lock(BufferAccessBits accessBits);
    void* Lock(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataSize);

    template<typename TElement>
    inline TElement* LockData(BufferAccessBits accessBits)
    {
        return static_cast<TElement*>(Lock(accessBits));
    }
    template<typename TElement>
    inline TElement* LockData(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataSize)
    {
        return static_cast<TElement*>(Lock(accessBits, bufferOffset, dataSize));
    }

    // unmap buffer object data source
    // @return false on fail, indicates that buffer should be reload
    bool Unlock();

    // will drop buffer data and allocate new chunk of gpu memory of same size
    // This is used in 'orphaning' technique 
    void Invalidate();

    // test whether index or vertex attributes data is currently bound - depends on content
    bool IsBufferBound() const;

    // test whether buffer is created
    bool IsBufferInited() const;

private:
    class ScopeBinder;

    void SetUnbound();

private:
    GpuBufferHandle mResourceHandle;
    GraphicsDeviceContext& mGraphicsContext;
};