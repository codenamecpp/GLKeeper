#pragma once

//////////////////////////////////////////////////////////////////////////

template<typename TPoolObject>
class SimplePool
{
public:

    //////////////////////////////////////////////////////////////////////////

    using OnObjectReturnProc = void (*)(TPoolObject* object);
    using OnObjectAcquireProc = void (*)(TPoolObject* object);

    //////////////////////////////////////////////////////////////////////////

    SimplePool(OnObjectReturnProc onReturnProc = nullptr, OnObjectAcquireProc onAcquireProc = nullptr)
        : mOnObjectReturnProc(onReturnProc)
        , mOnObjectAcquireProc(onAcquireProc)
    {
    }

    ~SimplePool()
    {
        cxx_assert(mAllocatedObjects.size() == mFreeObjects.size());

        for (TPoolObject* roller: mFreeObjects)
        {
            delete roller;
        }

        // make sure to free all allocated objects to avoid memory leaks
    }

    // make sure to reset object before use

    inline TPoolObject* Acquire()
    {
        TPoolObject* instance = nullptr;
        if (mFreeObjects.empty())
        {
            instance = new TPoolObject;
            mAllocatedObjects.push_back(instance);
        }
        else
        {
            instance = mFreeObjects.back();
            mFreeObjects.pop_back();
        }
        cxx_assert(instance);
        if (mOnObjectAcquireProc)
        {
            mOnObjectAcquireProc(instance);
        }
        return instance;
    }

    // make sure to reset object before return

    inline void Return(TPoolObject* instance)
    {
        cxx_assert(instance);
        cxx_assert(cxx::contains(mAllocatedObjects, instance));
        if (instance)
        {
            if (mOnObjectReturnProc)
            {
                mOnObjectReturnProc(instance);
            }
            mFreeObjects.push_back(instance);
        }
    }
private:
    std::vector<TPoolObject*> mAllocatedObjects;
    std::vector<TPoolObject*> mFreeObjects;
    // objects procs
    OnObjectReturnProc mOnObjectReturnProc;
    OnObjectAcquireProc mOnObjectAcquireProc;
};