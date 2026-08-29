#pragma once

namespace cxx
{

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    class any_vector
    {
    private:
        //////////////////////////////////////////////////////////////////////////
        struct vtable_t 
        {
            T*      (*data)             (void*);
            size_t  (*size)             (const void*);
            void    (*clear)            (void*);
            void    (*push_back_cref)   (void*, const T&);
            void    (*push_back_move)   (void*, T&&);
            void    (*assign)           (void*, const T*, const T*);
            void    (*reserve)          (void*, size_t);
        };
        //////////////////////////////////////////////////////////////////////////
        template <typename TVector>
        static constexpr const vtable_t* get_vtable()
        {
            static const vtable_t vtable_for_vector_t = 
            {
                [](void* v)                 { return static_cast<TVector*>(v)->data(); },
                [](const void* v)           { return static_cast<const TVector*>(v)->size(); },
                [](void* v)                 { static_cast<TVector*>(v)->clear(); },
                [](void* v, const T& val)   { static_cast<TVector*>(v)->push_back(val); },
                [](void* v, T&& val)        { static_cast<TVector*>(v)->push_back(std::move(val)); },
                [](void* v, const T* b, const T* e) { static_cast<TVector*>(v)->assign(b, e); },
                [](void* v, size_t reserve_capacity){ static_cast<TVector*>(v)->reserve(reserve_capacity); }
            };
            return &vtable_for_vector_t;
        }
        //////////////////////////////////////////////////////////////////////////
        void* mVector;
        const vtable_t* mVtbl;
        //////////////////////////////////////////////////////////////////////////
    public:
        constexpr any_vector() noexcept = delete;
        constexpr any_vector(const any_vector&) noexcept = default;
        template<typename TVector, 
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<TVector>, any_vector> >>
        any_vector(TVector&& sourceVector)
            : mVector(&sourceVector)
            , mVtbl(get_vtable<std::remove_reference_t<TVector>>())
        {
            static_assert(!std::is_rvalue_reference_v<TVector&&>, 
                "Cannot create any_vector from a temporary rvalue container!");
        }
        constexpr any_vector& operator = (const any_vector&) noexcept = default;

        //////////////////////////////////////////////////////////////////////////

        // interface
        inline void clear() { mVtbl->clear(mVector); }
        inline void push_back(const T& value) 
        {
            mVtbl->push_back_cref(mVector, value);
        }
        inline void push_back(T&& value) 
        {
            mVtbl->push_back_move(mVector, std::move(value));
        }
        inline void reserve(size_t reserve_capacity) 
        { 
            mVtbl->reserve(mVector, reserve_capacity); 
        }
        inline void assign(const T* it_begin, const T* it_end)
        {
            mVtbl->assign(mVector, it_begin, it_end);
        }
        inline T* data() noexcept { return mVtbl->data(mVector); }
        inline const T* data() const noexcept { return mVtbl->data(mVector); }
        inline size_t size() const noexcept 
        { 
            return mVtbl->size(mVector); 
        }
        inline bool empty() const noexcept { return size() == 0; }

        // iterators
        inline T& operator[](size_t i) noexcept { return data()[i]; }
        inline const T& operator[](size_t i) const noexcept { return data()[i]; }
        inline T* begin() noexcept { return data(); }
        inline T* end() noexcept { return data() + size(); }
        inline const T* begin() const noexcept { return data(); }
        inline const T* end() const noexcept { return data() + size(); }
        inline const T* cbegin() const noexcept { return data(); }
        inline const T* cend() const noexcept { return data() + size(); }

        //////////////////////////////////////////////////////////////////////////

        // helpers
        template <typename U>
        inline void assign(const std::vector<U>& container)
        {
            mVtbl->assign(mVector, container.data(), container.data() + container.size());
        }

        template <typename U>
        inline void assign(const std::pmr::vector<U>& container)
        {
            mVtbl->assign(mVector, container.data(), container.data() + container.size());
        }

        template <typename U, int StaticCapacity>
        inline void assign(const cxx::static_vector<U, StaticCapacity>& container)
        {
            mVtbl->assign(mVector, container.data(), container.data() + container.size());
        }
    };

    //////////////////////////////////////////////////////////////////////////

} // namespace cxx