#pragma once

//////////////////////////////////////////////////////////////////////////

// Container for listeners
// Safely handles cases where listeners are added or removed while notifications are being dispatched

// Typical usage :
// ListenersContainer.IterateListeners(callback)

//////////////////////////////////////////////////////////////////////////

template<typename TListener>
class ListenersContainer
{
public:
    ListenersContainer() = default;
    ~ListenersContainer()
    {
        cxx_assert(!IsListenersLocked());
    }

    template<typename TProc>
    inline void IterateListeners(TProc proc)
    {
        LockListeners();
        for (size_t icounter = 0, NumListeners = mListenersList.size(); icounter < NumListeners; ++icounter)
        {
            TListener* listener = mListenersList[icounter];
            // make sure listener isn't marked as removed
            if (!cxx::contains(mRemoveListenersList, listener))
            {
                proc(listener);
            }
        }
        UnLockListeners();
    }

    // Add listener to container
    void AddListener(TListener* listener);

    // Remove specific listener from container
    void RemoveListener(TListener* listener);

    // Drop all listeners
    void RemoveListeners();

private:
    void LockListeners();
    void UnLockListeners();
    bool IsListenersLocked() const { return mLockListenersCounter > 0; }

private:
    std::vector<TListener*> mListenersList;
    // deferred remove listeners
    std::vector<TListener*> mRemoveListenersList;
    int mLockListenersCounter = 0;
};

#include "ListenersContainer.inl"