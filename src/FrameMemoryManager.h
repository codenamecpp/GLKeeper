#pragma once

//////////////////////////////////////////////////////////////////////////

class FrameMemoryManager
{
private:

    //////////////////////////////////////////////////////////////////////////

    static const unsigned int FrameBufferCapacity = 8 * 1024 * 1024; // 8 MB

    //////////////////////////////////////////////////////////////////////////

    class arena_buffer_resource : public std::pmr::memory_resource
    {
    public:
        arena_buffer_resource() = default;
        arena_buffer_resource(void* buffer, const size_t bufferSize) noexcept;
        ~arena_buffer_resource() noexcept override
        {
        }
        arena_buffer_resource(const arena_buffer_resource&) = delete;
        arena_buffer_resource& operator=(const arena_buffer_resource&) = delete;
        void restart() noexcept;
    private:
        void * do_allocate(const size_t dataBytes, const size_t dataAlign) override;
        void do_deallocate(void * ptr, size_t dataBytes, size_t dataAlign) override;
        bool do_is_equal(const memory_resource& rhs) const noexcept override
        {
            return (this == &rhs);
        }
    private:
        void * mBufferPtr = nullptr;
        void * mCursorPtr = nullptr;
        size_t mSpaceAvailable = 0;
        size_t mBufferSize = 0;
        size_t mAllocationsCounter = 0;
        size_t mDeallocationsCounter = 0;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    bool Initialize();
    void Shutdown();

    void ResetFrameMemory();

private:
    std::unique_ptr<std::byte[]> mBuffer;
    std::optional<arena_buffer_resource> mArenaBufferResource;
    std::pmr::memory_resource* mDefaultMemoryResource = nullptr;
};

//////////////////////////////////////////////////////////////////////////

extern FrameMemoryManager gFrameMemoryManager;

//////////////////////////////////////////////////////////////////////////