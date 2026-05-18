#pragma once

namespace cxx
{

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    class span
    {
    public:
        span() noexcept = default;

        // construct span from vector

        template<typename U>
        span(std::vector<U>&&) = delete;

        template<typename U>
        span(std::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        template<typename U>
        span(const std::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        // construct span from pmr vector

        template<typename U>
        span(std::pmr::vector<U>&& container) = delete;

        template<typename U>
        span(std::pmr::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        template<typename U>
        span(const std::pmr::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        // construct span from static_vector

        template<typename U, int StaticCapacity>
        span(cxx::static_vector<U, StaticCapacity>&&) = delete;

        template<typename U, int StaticCapacity>
        span(cxx::static_vector<U, StaticCapacity>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        template<typename U, int StaticCapacity>
        span(const cxx::static_vector<U, StaticCapacity>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        // construct span from ranges/c array

        span(T* arrayBegin, T* arrayEnd) noexcept
            : mPtr(arrayBegin)
            , mEnd(arrayEnd)
        {
            cxx_assert(mEnd >= mPtr);
        }

        span(T* arrayBegin, size_t arraySize) noexcept
            : mPtr(arrayBegin)
            , mEnd(arrayBegin + arraySize)
        {
            cxx_assert(mEnd >= mPtr);
        }

        template <std::size_t N>
        span(T (&carray)[N])
            : mPtr(carray)
            , mEnd(carray + N) 
        {
            cxx_assert(mEnd >= mPtr);
        }

        inline bool empty() const noexcept { return mPtr == mEnd; }
        inline int size() const noexcept
        {
            return static_cast<int>(mEnd - mPtr);
        }
        inline T* cbegin() const noexcept { return mPtr; }
        inline T* cend() const noexcept { return mEnd; }
        inline T* begin() const noexcept { return mPtr; }
        inline T* end() const noexcept { return mEnd; }
        inline T& operator [] (int index) const
        {
            cxx_assert((index >= 0) && (index < size()));
            return mPtr[index]; 
        }
    private:
        T* mPtr = nullptr; // array begin
        T* mEnd = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    // special case for pointer types
    
    // the span<T*> provides access to the pointed-to objects,
    // but does not allow changing the pointer values themselves (equivalent to T* const* semantics)

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    class span<T*>
    {
    public:
        span() noexcept = default;

        // construct span from vector

        template<typename U>
        span(std::vector<U>&&) = delete;

        template<typename U>
        span(std::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        template<typename U>
        span(const std::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        // construct span from pmr vector

        template<typename U>
        span(std::pmr::vector<U>&& container) = delete;

        template<typename U>
        span(std::pmr::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        template<typename U>
        span(const std::pmr::vector<U>& container) noexcept
            : mPtr(container.data())
            , mEnd(container.data() + container.size())
        {
            cxx_assert(mEnd >= mPtr);
        }

        // construct span from ranges/c array

        span(T* const* arrayBegin, T* const* arrayEnd) noexcept
            : mPtr(arrayBegin)
            , mEnd(arrayEnd)
        {
            cxx_assert(mEnd >= mPtr);
        }

        span(T* const* arrayBegin, size_t arraySize) noexcept
            : mPtr(arrayBegin)
            , mEnd(arrayBegin + arraySize)
        {
            cxx_assert(mEnd >= mPtr);
        }

        inline bool empty() const noexcept { return mPtr == mEnd; }
        inline int size() const noexcept
        {
            return static_cast<int>(mEnd - mPtr);
        }
        inline T* const* cbegin() const noexcept { return mPtr; }
        inline T* const* cend() const noexcept { return mEnd; }
        inline T* const* begin() const noexcept { return mPtr; }
        inline T* const* end() const noexcept { return mEnd; }
        inline T* operator [] (int index) const
        {
            cxx_assert((index >= 0) && (index < size()));
            return mPtr[index]; 
        }
    private:
        T* const* mPtr = nullptr; // array begin
        T* const* mEnd = nullptr;
    };

} // namespace cxx
