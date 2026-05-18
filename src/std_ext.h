#pragma once

// small c++ std templates library extensions

using ByteArray = std::vector<unsigned char>;

template <typename Array>
struct ArrayType;

template <typename TElement, int NumElements>
struct ArrayType<TElement[NumElements]>
{
    enum { Countof = NumElements };
};

template <typename TElement, int NumElements>
constexpr int CountOf(const TElement(&)[NumElements])
{
    return NumElements;
}

template <typename TElement>
inline void SafeDelete(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete elementPointer;
        elementPointer = nullptr;
    }
}

template<typename TElement>
inline void SafeDeleteArray(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete [] elementPointer;
        elementPointer = nullptr;
    }
}

namespace cxx
{

    template<typename TContainer, typename TElement>
    inline void erase_elements(TContainer& container, const TElement& element)
    {
        auto remove_iterator = std::remove(container.begin(), container.end(), element);
        if (remove_iterator != container.end())
        {
            container.erase(remove_iterator, container.end());
        }
    }

    template<typename TContainer>
    inline void erase_elements(TContainer& container, const TContainer& erase_container)
    {
        for (const auto& currElement: erase_container)
        {
            erase_elements(container, currElement);
        }
    }

    // erase elements shortcut for generic conteiner
    template<typename TContainer, typename TValue>
    inline bool erase(TContainer& inContainer, const TValue& value)
    {
        auto remove_if_iterator = std::remove(inContainer.begin(), inContainer.end(), value);
        if (remove_if_iterator != inContainer.end())
        {
            inContainer.erase(remove_if_iterator, inContainer.end());
            return true;
        }
        return false;
    }

    // erase elements with custom predicate shortcut for generic conteiner
    template<typename TContainer, typename TPred>
    inline bool erase_if(TContainer& inContainer, TPred removePred)
    {
        auto remove_if_iterator = std::remove_if(std::begin(inContainer), std::end(inContainer), removePred);
        if (remove_if_iterator != std::end(inContainer))
        {
            inContainer.erase(remove_if_iterator, std::end(inContainer));
            return true;
        }
        return false;
    }

    // check whether container contains specific item
    template<typename TContainer, typename TValue>
    inline bool contains(const TContainer& container, const TValue& element)
    {
        auto find_iterator = std::find(std::cbegin(container), std::cend(container), element);
        return find_iterator != std::cend(container);
    }

    // check whether container contains specific item
    template<typename TContainer, typename TPred>
    inline bool contains_if(const TContainer& container, TPred funcPred)
    {
        auto find_iterator = std::find_if(std::cbegin(container), std::cend(container), funcPred);
        return find_iterator != std::cend(container);
    }

    template<typename TKey, typename TValue, typename TMap>
    inline TValue get_value_or_default(const TMap& sourceMap, const TKey& key, TValue defaultValue)
    {
        auto it = sourceMap.find(key);
        if (it == sourceMap.end())
            return defaultValue;
        return it->second;
    }

    template<typename TContainer, typename TProc>
    inline int get_first_index_if(const TContainer& container, TProc proc)
    {
        int counter = 0;
        for (const auto& roller: container)
        {
            if (proc(roller)) return counter;
            ++counter;
        }
        return -1;
    }

    template<typename TContainer, typename TValue>
    inline int get_item_index(const TContainer& container, const TValue& value)
    {
        int counter = 0;
        for (const TValue& roller: container)
        {
            if (roller == value) return counter;
            ++counter;
        }
        return -1;
    }

} // namespace cxx