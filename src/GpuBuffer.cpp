#include "pch.h"
#include "GpuBuffer.h"
#include "GraphicsDeviceContext.h"
#include "OpenGL_Defs.h"

//////////////////////////////////////////////////////////////////////////

class GpuBuffer::ScopedBufferBinder
{
public:
    ScopedBufferBinder(GpuBuffer* gpuBuffer)
        : mPreviousBuffer(gpuBuffer->mGraphicsContext.mCurrentBuffers[gpuBuffer->mContent])
        , mBuffer(gpuBuffer)
    {
        debug_assert(mBuffer);
        if (mBuffer != mPreviousBuffer)
        {
            GLenum bufferTargetGL = EnumToGL(mBuffer->mContent);

            ::glBindBuffer(bufferTargetGL, mBuffer->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedBufferBinder()
    {
        if (mBuffer != mPreviousBuffer)
        {
            GLenum bufferTargetGL = EnumToGL(mBuffer->mContent);

            ::glBindBuffer(bufferTargetGL, mPreviousBuffer ? mPreviousBuffer->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GpuBuffer* mPreviousBuffer;
    GpuBuffer* mBuffer;
};

//////////////////////////////////////////////////////////////////////////

GpuBuffer::GpuBuffer(GraphicsDeviceContext& graphicsContext, eBufferContent bufferContent)
    : mGraphicsContext(graphicsContext)
    , mResourceHandle()
    , mContent(bufferContent)
    , mUsageHint()
    , mBufferLength()
    , mBufferCapacity()
{
    ::glGenBuffers(1, &mResourceHandle);
    glCheckError();
}

GpuBuffer::~GpuBuffer()
{
    SetUnbound();

    ::glDeleteBuffers(1, &mResourceHandle);
    glCheckError();
}

void GpuBuffer::SetUnbound()
{    
    if (mGraphicsContext.mCurrentBuffers[mContent] == this)
    {
        mGraphicsContext.mCurrentBuffers[mContent] = nullptr;
    }
}

bool GpuBuffer::Setup(eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    unsigned int paddedContentLength = (bufferLength + 15U) & (~15U);

    mBufferLength = bufferLength;
    mBufferCapacity = paddedContentLength;
    mUsageHint = bufferUsage;
    debug_assert(mUsageHint < eBufferUsage_COUNT);

    ScopedBufferBinder scopedBind {this};
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLenum bufferUsageGL = EnumToGL(mUsageHint);
    ::glBufferData(bufferTargetGL, mBufferCapacity, nullptr, bufferUsageGL);
    glCheckError();

    if (dataBuffer)
    {
        void* pMappedData = ::glMapBufferRange(bufferTargetGL, 0, mBufferCapacity, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        glCheckError();
        if (pMappedData == nullptr)
        {
            debug_assert(false);
        }
        else
        {
            ::memcpy(pMappedData, dataBuffer, bufferLength);
        }

        GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
        glCheckError();
        debug_assert(unmapResult == GL_TRUE);
    }
    return true;
}

bool GpuBuffer::Resize(unsigned int newLength)
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return false;
    }

    if (newLength <= mBufferCapacity)
    {
        mBufferLength = newLength;
        return true;
    }

    unsigned int newBufferCapacity = (newLength + 15U) & (~15U); // padded

                                                                 // allocate new buffer and transfer data
    bool wasBound = IsBufferBound();

    GLenum bufferTargetGL = EnumToGL(mContent);
    GLenum bufferUsageGL = EnumToGL(mUsageHint);

    // generate new buffer object and do setup params
    GLuint newVBO;
    ::glGenBuffers(1, &newVBO);
    glCheckError();
    ::glBindBuffer(bufferTargetGL, newVBO);
    glCheckError();
    ::glBufferData(bufferTargetGL, newBufferCapacity, nullptr, bufferUsageGL);
    glCheckError();

    // bind source and destination buffers and do copy data
    ::glBindBuffer(GL_COPY_READ_BUFFER, mResourceHandle);
    glCheckError();
    ::glCopyBufferSubData(GL_COPY_READ_BUFFER, bufferTargetGL, 0, 0, mBufferCapacity);
    glCheckError();

    // unbind source and destination buffers
    ::glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glCheckError();

    // destroy old buffer
    ::glDeleteBuffers(1, &mResourceHandle);
    glCheckError();

    // set new params
    mBufferCapacity = newBufferCapacity;
    mBufferLength = newLength;
    mResourceHandle = newVBO;

    // restore state
    if (!wasBound)
    {
        ::glBindBuffer(bufferTargetGL, 0);
        glCheckError();
    }

    return true;
}

bool GpuBuffer::SubData(unsigned int dataOffset, unsigned int dataLength, const void* dataSource)
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return false;
    }

    debug_assert(dataLength && dataSource);
    debug_assert(dataOffset + dataLength < mBufferCapacity);

    ScopedBufferBinder scopedBind {this};
    GLenum bufferTargetGL = EnumToGL(mContent);
    ::glBufferSubData(bufferTargetGL, dataOffset, dataLength, dataSource);
    glCheckError();

    return true;
}

void* GpuBuffer::Lock(BufferAccessBits accessBits)
{
    return Lock(accessBits, 0, mBufferCapacity);
}

void* GpuBuffer::Lock(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataLength)
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GLbitfield accessBitsGL = ((accessBits & BufferAccess_Read) > 0 ? GL_MAP_READ_BIT : 0) |
        ((accessBits & BufferAccess_Write) > 0 ? GL_MAP_WRITE_BIT : 0) |
        ((accessBits & BufferAccess_Unsynchronized) > 0 ? GL_MAP_UNSYNCHRONIZED_BIT : 0) |
        ((accessBits & BufferAccess_InvalidateRange) > 0 ? GL_MAP_INVALIDATE_RANGE_BIT : 0) |
        ((accessBits & BufferAccess_InvalidateBuffer) > 0 ? GL_MAP_INVALIDATE_BUFFER_BIT : 0);

    debug_assert(accessBitsGL > 0);
    if (accessBitsGL == 0)
        return nullptr;

    if (dataLength == 0 || ((bufferOffset + dataLength) > mBufferCapacity))
    {
        debug_assert(false);
        return nullptr;
    }

    ScopedBufferBinder scopedBind {this};
    GLenum bufferTargetGL = EnumToGL(mContent);
    void* pMappedData = ::glMapBufferRange(bufferTargetGL, bufferOffset, dataLength, accessBitsGL);
    glCheckError();
    return pMappedData;
}

bool GpuBuffer::Unlock()
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return false;
    }

    ScopedBufferBinder scopedBind {this};
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
    glCheckError();
    return unmapResult == GL_TRUE;
}

void GpuBuffer::Invalidate()
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return;
    }

    ScopedBufferBinder scopedBind {this};
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLenum bufferUsageGL = EnumToGL(mUsageHint);
    ::glBufferData(bufferTargetGL, mBufferCapacity, nullptr, bufferUsageGL);
    glCheckError();
}

bool GpuBuffer::IsBufferBound() const
{
    if (IsBufferInited())
    {
        return this == mGraphicsContext.mCurrentBuffers[mContent];
    }
    return false;
}

bool GpuBuffer::IsBufferInited() const
{
    return mBufferCapacity > 0;
}
