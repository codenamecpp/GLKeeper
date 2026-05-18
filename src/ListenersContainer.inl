#pragma once

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
inline void ListenersContainer<TListener>::LockListeners()
{
    ++mLockListenersCounter;
}

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
inline void ListenersContainer<TListener>::UnLockListeners()
{
    cxx_assert(IsListenersLocked());

    if (--mLockListenersCounter > 0)
        return;

    // remove queued listeners
    if (!mRemoveListenersList.empty())
    {
        for (TListener* roller: mRemoveListenersList)
        {
            cxx::erase(mListenersList, roller);
        }
        mRemoveListenersList.clear();
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
inline void ListenersContainer<TListener>::AddListener(TListener* listener)
{
    if (listener == nullptr)
        return;

    // it's safe to add new listener right away
    if (!cxx::contains(mListenersList, listener))
    {
        mListenersList.push_back(listener);
    }

    if (IsListenersLocked())
    {
        cxx::erase(mRemoveListenersList, listener);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
inline void ListenersContainer<TListener>::RemoveListener(TListener* listener)
{
    if (listener == nullptr)
        return;

    // deferred
    if (IsListenersLocked())
    {
        if (!cxx::contains(mRemoveListenersList, listener))
        {
            mRemoveListenersList.push_back(listener);
        }
        return;
    }
    // immediate
    cxx::erase(mListenersList, listener);
}

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
inline void ListenersContainer<TListener>::RemoveListeners()
{
    mRemoveListenersList.clear();

    // deferred
    if (IsListenersLocked())
    {
        mRemoveListenersList = mListenersList;
        return;
    }
    // immediate
    mListenersList.clear();
}

//////////////////////////////////////////////////////////////////////////