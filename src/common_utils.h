#pragma once

// small c++ std templates library extensions

template <typename Array> struct ArrayType;
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
    // stl containers helpers

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
    inline void erase_elements(TContainer& source_container, const TContainer& erase_container)
    {
        for (const auto& currElement: erase_container)
        {
            erase_elements(source_container, currElement);
        }
    }

} // namespace cxx