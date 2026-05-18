#include "stdafx.h"
#include "Random.h"
#include <random>

namespace Random
{

//////////////////////////////////////////////////////////////////////////

static thread_local std::mt19937 gRngEngine {std::random_device {} ()};
static thread_local std::uniform_int_distribution<int> gRngIntsDistribution
    {
        std::numeric_limits<int>::min(), 
        std::numeric_limits<int>::max()
    };
static thread_local std::uniform_real_distribution<float> gRngFloatsDistribution01 {0.0f, 1.0f};
static thread_local std::uniform_real_distribution<float> gRngFloatsDistribution 
    {
        std::numeric_limits<float>::min(), 
        std::numeric_limits<float>::max()
    };

//////////////////////////////////////////////////////////////////////////

void SetLocalThreadSeed(unsigned int seed)
{
    gRngEngine.seed(seed);
}

unsigned int GenerateUint()
{
    return gRngEngine();
}

unsigned int GenerateUint(unsigned int minValue, unsigned int maxValue)
{
    if ((minValue == maxValue) || (maxValue < minValue))
    {
        return minValue;
    }
    std::uniform_int_distribution<unsigned int> distribution {minValue, maxValue};
    return distribution(gRngEngine);
}

int GenerateInt()
{
    return gRngIntsDistribution(gRngEngine);
}

int GenerateInt(int minValue, int maxValue)
{
    if ((minValue == maxValue) || (maxValue < minValue))
    {
        return minValue;
    }
    std::uniform_int_distribution<int> distribution {minValue, maxValue};
    return distribution(gRngEngine);
}

float GenerateFloat01()
{
    return gRngFloatsDistribution01(gRngEngine);
}

float GenerateFloat()
{
    return gRngFloatsDistribution(gRngEngine);
}

float GenerateFloat(float minValue, float maxValue)
{
    if ((minValue == maxValue) || (maxValue < minValue))
    {
        return minValue;
    }
    std::uniform_real_distribution<float> distribution {minValue, maxValue};
    return distribution(gRngEngine);
}

} // namespace Random