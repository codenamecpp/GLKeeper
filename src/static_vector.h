#pragma once

namespace cxx
{

    template<typename T, int StaticCapacity>
    class static_vector
    {
    public:
        //////////////////////////////////////////////////////////////////////////

        static_assert(StaticCapacity > 0, "Invalid static_vector capacity");

        //////////////////////////////////////////////////////////////////////////

        using value_type = T;
        using size_type = int;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //////////////////////////////////////////////////////////////////////////

    public:
        
        static_vector() noexcept : mSize() {}
        ~static_vector()
        {
            clear();
        }

        static_vector(size_type count, const_reference value)
        {
            cxx_assert((count >= 0) && (count <= capacity()));
            if (count > 0)
            {
                const size_type correctCount = (count > capacity()) ? capacity() : count;
                std::uninitialized_fill(begin(), begin() + correctCount, value);
                mSize = correctCount;
            }
        }

        static_vector(size_type count)
        {
            cxx_assert((count >= 0) && (count <= capacity()));
            if (count > 0)
            {
                const size_type correctCount = (count > capacity()) ? capacity() : count;
                std::uninitialized_default_construct(begin(), begin() + correctCount);
                mSize = correctCount;
            }
        }

        static_vector(std::initializer_list<value_type> initializerList)
        {
            int count = initializerList.end() - initializerList.begin();
            cxx_assert((count >= 0) && (count <= capacity()));

            if (count > 0)
            {
                const size_type correctCount = (count > capacity()) ? capacity() : count;
                std::uninitialized_copy(
                    initializerList.begin(), 
                    initializerList.begin() + correctCount, begin());
                mSize = correctCount;
            }
        }

        static_vector(const static_vector& sourceVector)
        {
            if (sourceVector.mSize > 0)
            {
                std::uninitialized_copy(
                    sourceVector.begin(), 
                    sourceVector.begin() + sourceVector.mSize, begin());
                mSize = sourceVector.mSize;
            }
        }

        static_vector(static_vector&& sourceVector) 
        {
            if (sourceVector.mSize > 0)
            {
                std::uninitialized_copy(
                    std::make_move_iterator(sourceVector.begin()),
                    std::make_move_iterator(sourceVector.begin() + sourceVector.mSize),
                    begin());

                mSize = sourceVector.mSize;

                sourceVector.mSize = 0;
            }
        }

        static_vector& operator = (const static_vector& sourceVector)
        {
            if (&sourceVector == this) return *this;

            clear();
            if (sourceVector.mSize > 0)
            {
                std::uninitialized_copy(
                    sourceVector.begin(), 
                    sourceVector.begin() + sourceVector.mSize, begin());
                mSize = sourceVector.mSize;
            }
            return *this;
        }

        static_vector& operator = (static_vector&& sourceVector)
        {
            if (&sourceVector == this) return *this;

            clear();

            if (sourceVector.mSize > 0)
            {
                std::uninitialized_copy(
                    std::make_move_iterator(sourceVector.begin()),
                    std::make_move_iterator(sourceVector.begin() + sourceVector.mSize), begin());
                mSize = sourceVector.mSize;

                sourceVector.mSize = 0;
            }
            return *this;
        }

        inline void clear() 
        {
            if (mSize > 0)
            {
                if (!std::is_trivially_destructible_v<value_type>)
                {
                    std::destroy(begin(), end());
                }
                mSize = 0;
            }
        }

        inline constexpr size_type capacity() const noexcept { return StaticCapacity; }
        inline size_type size() const noexcept { return mSize; }

        inline bool full() const noexcept { return mSize == capacity(); }
        inline bool empty() const noexcept { return mSize == 0; }

        inline pointer data() noexcept { return reinterpret_cast<pointer>(&mData[0]); }
        inline const_pointer data() const noexcept 
        {
            return reinterpret_cast<const_pointer>(&mData[0]);
        }

        inline iterator begin() noexcept { return data(); }
        inline iterator end() noexcept { return data() + mSize; }

        inline const_iterator begin() const noexcept { return data(); }
        inline const_iterator end() const noexcept { return data() + mSize; }

        inline const_iterator cbegin() const noexcept { return data(); }
        inline const_iterator cend() const noexcept { return data() + mSize; }

        inline reference operator[](size_t index) noexcept 
        {
            cxx_assert((index >= 0) && (index < size()));
            return data(index); 
        }
        inline const_reference operator[](size_t index) const noexcept 
        {
            cxx_assert((index >= 0) && (index < size()));
            return data(index);
        }

        inline reference front() noexcept 
        {
            cxx_assert(!empty());
            return data(0); 
        }
        inline reference back() noexcept 
        { 
            cxx_assert(!empty());
            return data(mSize - 1); 
        }

        inline const_reference front() const noexcept 
        {
            cxx_assert(!empty());
            return data(0); 
        }
        inline const_reference back() const noexcept 
        {
            cxx_assert(!empty());
            return data(mSize - 1); 
        }

        inline iterator erase(const_iterator erasePos) 
        {
            const size_type index = std::distance(cbegin(), erasePos);
            if ((index >= 0) && (index < mSize))
            {
                if constexpr (std::is_trivially_copyable_v<value_type>) 
                {
                    if (index < (mSize - 1))
                    {
                        std::memmove(begin() + index, begin() + index + 1, (mSize - index - 1) * sizeof(value_type));
                    }
                }
                else
                {
                    std::move(begin() + index + 1, end(), begin() + index);
                    std::destroy_at(end() - 1);
                }
                --mSize;
                return begin() + index;
            }
            cxx_assert(false);
            return end();
        }

        inline iterator erase(const_iterator eraseBegin, const_iterator eraseEnd)
        {
            const size_type indexFirst = std::distance(cbegin(), eraseBegin);
            const size_type indexLast = std::distance(cbegin(), eraseEnd);
            const size_type count = indexLast - indexFirst;

            if (count == 0)
                return begin() + indexFirst;

            if ((count > 0) && (indexFirst >= 0) && (indexLast <= mSize))
            {
                if constexpr (std::is_trivially_copyable_v<value_type>) 
                {
                    if (indexLast < mSize)
                    {
                        std::memmove(
                            begin() + indexFirst, 
                            begin() + indexLast, (mSize - indexLast) * sizeof(value_type));
                    }
                }
                else
                {
                    std::move(begin() + indexLast, end(), begin() + indexFirst);
                    std::destroy(end() - count, end());
                }
                mSize -= count;
                return begin() + indexFirst;
            }
            cxx_assert(false);
            return end();
        }

        inline void push_back(const_reference value) 
        {
            if (full())
            {
                cxx_assert(false);
                return;
            }
            new (storage_end()) value_type(value);
            ++mSize;
        }

        inline void push_back(value_type&& value) 
        {
            if (full())
            {
                cxx_assert(false);
                return;
            }
            new (storage_end()) value_type(std::move(value));
            ++mSize;
        }

    private:

        inline reference data(size_t index) noexcept 
        { 
            return *reinterpret_cast<pointer>(&mData[index]); 
        }
        inline const_reference data(size_t index) const noexcept 
        { 
            return *reinterpret_cast<const_pointer>(&mData[index]); 
        }

    private:
        using storage_type = std::aligned_storage_t<sizeof(value_type), alignof(value_type)>;

        inline storage_type* storage_begin() noexcept { return &mData[0]; }
        inline storage_type* storage_end() noexcept { return &mData[mSize]; }

    private:
        std::array<storage_type, StaticCapacity> mData = {};
        size_type mSize = 0;
    };

} // namespace cxx