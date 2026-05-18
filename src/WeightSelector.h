#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Random.h"

//////////////////////////////////////////////////////////////////////////

template <typename TElement>
class WeightSelector
{
private:

    //////////////////////////////////////////////////////////////////////////
    struct SelectorRecord
    {
    public:
        SelectorRecord(TElement& element, float weightLimit)
            : mElement(element)
            , mWeightLimit(weightLimit)
        {}
    public:
        TElement mElement;
        float mWeightLimit = 0.0f;
    };
    //////////////////////////////////////////////////////////////////////////

    using RecordsContainer = typename std::vector<SelectorRecord>;

public:
    WeightSelector() = default;

    // insert weighted selection item
    inline void AddSelectionElement(TElement element, float weight)
    {
        cxx_assert(weight > 0.0f);
        if (weight > 0.0f)
        {
            const float newWeightLimit = weight + GetTotalWeight();
            mSelectionRecords.emplace_back(element, newWeightLimit);
        }
    }

    // randomly select selection item
    inline TElement SelectRandomElement() const
    {
        cxx_assert(!mSelectionRecords.empty());
        if (!mSelectionRecords.empty())
        {
            float randomNumber = Random::GenerateFloat01() * GetTotalWeight();
            for (const SelectorRecord& currRecord: mSelectionRecords)
            {
                if (randomNumber < currRecord.mWeightLimit)
                    return currRecord.mElement;
            }
            return mSelectionRecords.back().mElement;
        }
        return {};
    }

    // reset selector
    inline void Clear()
    {
        mSelectionRecords.clear();
    }

    inline bool IsEmpty() const
    {
        return mSelectionRecords.empty();
    }

private:

    inline float GetTotalWeight() const
    {
        float totalWeight = 0.0f;
        if (!mSelectionRecords.empty())
        {
            totalWeight = mSelectionRecords.back().mWeightLimit;
        }
        return totalWeight;
    }
    
private:
    RecordsContainer mSelectionRecords;
};

//////////////////////////////////////////////////////////////////////////