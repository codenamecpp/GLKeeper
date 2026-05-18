#include "stdafx.h"
#include "GpuBuffer.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

GpuBuffer* GpuBuffer::sCurrentBuffers[eBufferTarget_COUNT] = {};

//////////////////////////////////////////////////////////////////////////

GpuBuffer::ScopedBinder::ScopedBinder(GpuBuffer* gpuBuffer)
    : mBuffer(gpuBuffer)
{
    if (mBuffer)
    {
        mPreviousBuffer = mBuffer->BindBuffer();
    }
}

GpuBuffer::ScopedBinder::~ScopedBinder()
{
    if (mPreviousBuffer != mBuffer)
    {
        if (mPreviousBuffer)
        {
            mPreviousBuffer->BindBuffer();
        }
        else
        {
            mBuffer->UnBindBuffer();
        }
    }
}

//////////////////////////////////////////////////////////////////////////

GpuBuffer::GpuBuffer(eBufferTarget bufferTarget)
    : mResourceHandle()
    , mTarget(bufferTarget)
    , mUsageHint()
    , mBufferLength()
    , mBufferCapacity()
{
    ::glGenBuffers(1, &mResourceHandle);
    glCheckErrors();
}

GpuBuffer::~GpuBuffer()
{
    UnBindBuffer();

    ::glDeleteBuffers(1, &mResourceHandle);
    glCheckErrors();
}

bool GpuBuffer::Create(eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    mBufferLength = bufferLength;
    mBufferCapacity = (bufferLength + 15U) & (~15U);
    mUsageHint = bufferUsage;

    ScopedBinder scopedBind (this);

    GLenum bufferTargetGL = ToGLEnum(mTarget);
    GLenum bufferUsageGL = ToGLEnum(mUsageHint);
    ::glBufferData(bufferTargetGL, mBufferCapacity, nullptr, bufferUsageGL);
    glCheckErrors();
    if (dataBuffer)
    {
        void* pMappedData = ::glMapBufferRange(bufferTargetGL, 0, mBufferCapacity, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        glCheckErrors();
        if (pMappedData == nullptr)
        {
            mBufferLength = 0;
            mBufferCapacity = 0;
            return false;
        }

        ::memcpy(pMappedData, dataBuffer, bufferLength);

        GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
        glCheckErrors();
        return unmapResult == GL_TRUE;
    }
    return true;
}

void* GpuBuffer::Lock(BufferAccessBits contentAccess)
{
    return Lock(contentAccess, 0, mBufferCapacity);
}

void* GpuBuffer::Lock(BufferAccessBits contentAccess, unsigned int bufferOffset, unsigned int mapLength)
{
    if (mBufferCapacity == 0)
    {
        cxx_assert(false);
        return false;
    }

    GLbitfield accessBitsGL = ((contentAccess & BufferAccess_Read) > 0 ? GL_MAP_READ_BIT : 0) |
        ((contentAccess & BufferAccess_Write) > 0 ? GL_MAP_WRITE_BIT : 0) |
        ((contentAccess & BufferAccess_Unsynchronized) > 0 ? GL_MAP_UNSYNCHRONIZED_BIT : 0) |
        ((contentAccess & BufferAccess_InvalidateRange) > 0 ? GL_MAP_INVALIDATE_RANGE_BIT : 0) |
        ((contentAccess & BufferAccess_InvalidateBuffer) > 0 ? GL_MAP_INVALIDATE_BUFFER_BIT : 0);

    cxx_assert(accessBitsGL > 0);
    if (accessBitsGL == 0)
        return nullptr;

    if (mapLength == 0 || bufferOffset + mapLength > mBufferCapacity)
    {
        cxx_assert(false);
        return nullptr;
    }

    void* pMappedData = nullptr;

    ScopedBinder scopedBind (this);
    GLenum bufferTargetGL = ToGLEnum(mTarget);
    pMappedData = ::glMapBufferRange(bufferTargetGL, bufferOffset, mapLength, accessBitsGL);
    glCheckErrors();
    
    return pMappedData;
}

bool GpuBuffer::Unlock()
{
    ScopedBinder scopedBind (this);
    GLenum bufferTargetGL = ToGLEnum(mTarget);

    GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
    glCheckErrors();
    return unmapResult == GL_TRUE;
}

bool GpuBuffer::SubData(unsigned int bufferOffset, unsigned int dataSize, const void* sourceContent)
{
    if (mBufferCapacity == 0)
    {
        cxx_assert(false);
        return false;
    }
    // out of range check
    if ((bufferOffset + dataSize) > mBufferLength)
    {
        cxx_assert(false);
        return false;
    }
    ScopedBinder scopedBind (this);
    GLenum bufferTargetGL = ToGLEnum(mTarget);
    ::glBufferSubData(bufferTargetGL, bufferOffset, dataSize, sourceContent);
    glCheckErrors();
    return true;
}

bool GpuBuffer::IsBufferBound() const
{
    return sCurrentBuffers[mTarget] == this;
}

GpuBuffer* GpuBuffer::BindBuffer()
{
    GpuBuffer* previousBuffer = sCurrentBuffers[mTarget];
    if (previousBuffer != this)
    {
        GLenum bufferTargetGL = ToGLEnum(mTarget);
        ::glBindBuffer(bufferTargetGL, mResourceHandle);
        glCheckErrors();

        sCurrentBuffers[mTarget] = this;
    }
    return previousBuffer;
}

void GpuBuffer::UnBindBuffer()
{
    if (sCurrentBuffers[mTarget] == this)
    {
        GLenum bufferTargetGL = ToGLEnum(mTarget);
        ::glBindBuffer(bufferTargetGL, 0);
        glCheckErrors();

        sCurrentBuffers[mTarget] = nullptr;
    }
}
