#include "stdafx.h"
#include "FrameMemoryManager.h"
#include "GameMain.h"

//////////////////////////////////////////////////////////////////////////

FrameMemoryManager gFrameMemoryManager;

//////////////////////////////////////////////////////////////////////////

FrameMemoryManager::arena_buffer_resource::arena_buffer_resource(void* buffer, const size_t bufferSize) noexcept
    : mBufferPtr(buffer)
    , mCursorPtr(buffer)
    , mSpaceAvailable(bufferSize)
    , mBufferSize(bufferSize)
{
}

void FrameMemoryManager::arena_buffer_resource::restart() noexcept
{
    mCursorPtr = mBufferPtr;
    mSpaceAvailable = mBufferSize;

    cxx_assert(mAllocationsCounter == mDeallocationsCounter);

    mAllocationsCounter = 0;
    mDeallocationsCounter = 0;
}

void* FrameMemoryManager::arena_buffer_resource::do_allocate(const size_t dataBytes, const size_t dataAlign)
{
    if (!_STD align(dataAlign, dataBytes, mCursorPtr, mSpaceAvailable))
    {
        gConsole.LogMessage(eLogLevel_Error, "Fatal error: frame memory budget exceeded");

        gGame.Terminate();
        return nullptr;
    }

    void* resultPtr = mCursorPtr;
    mCursorPtr = reinterpret_cast<char *>(mCursorPtr) + dataBytes;
    mSpaceAvailable -= dataBytes;

    ++mAllocationsCounter;
    return (resultPtr);
}

void FrameMemoryManager::arena_buffer_resource::do_deallocate(void * ptr, size_t dataBytes, size_t dataAlign)
{
    ++mDeallocationsCounter;
}

//////////////////////////////////////////////////////////////////////////

bool FrameMemoryManager::Initialize()
{
    mBuffer = std::make_unique<std::byte[]>(FrameBufferCapacity);

    mDefaultMemoryResource = std::pmr::get_default_resource();

    mArenaBufferResource.emplace(mBuffer.get(), FrameBufferCapacity);

    std::pmr::set_default_resource(&*mArenaBufferResource);
    return true;
}

void FrameMemoryManager::Shutdown()
{
    std::pmr::set_default_resource(mDefaultMemoryResource);

    mDefaultMemoryResource = nullptr;

    mArenaBufferResource.reset();
    mBuffer.reset();
}

void FrameMemoryManager::ResetFrameMemory()
{
    mArenaBufferResource->restart(); 
}

