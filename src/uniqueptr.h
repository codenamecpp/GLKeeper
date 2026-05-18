#pragma once

namespace cxx
{

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct uniqueptr_deleter
    {
    public:
        using deleter_proc_t = void (*)(T* object);
    public:
        uniqueptr_deleter() : mDeleterProc([](T* object) { delete object; }) { }
        uniqueptr_deleter(deleter_proc_t proc) : mDeleterProc(proc) { }
        template<typename TDeleter>
        uniqueptr_deleter(TDeleter proc)
            : mDeleterProc(static_cast<deleter_proc_t>(proc))
        { }
        inline void operator() (T* object) const { if (object) mDeleterProc(object); }
    public:
        deleter_proc_t mDeleterProc;
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    using uniqueptr = std::unique_ptr<T, typename uniqueptr_deleter<T>>;

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    using uniqueptr_deleter_proc_t = typename uniqueptr_deleter<T>::deleter_proc_t;

    //////////////////////////////////////////////////////////////////////////

} // namespace cxx