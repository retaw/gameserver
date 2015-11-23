#ifndef WATER_BASE_RANDOM_H
#define WATER_BASE_RANDOM_H

//#include "random.h"

#include <random>
#include <limits>

namespace water{
namespace componet{

enum class DistributionPattern
{
    NORMAL, UNIFORM
};

template<typename NumType, DistributionPattern dp, bool isInt = std::is_integral<NumType>::value>
class Distribution;

template<typename NumType>
class Distribution<NumType, DistributionPattern::UNIFORM, true>
{
public:
    Distribution(NumType minNum, NumType maxNum)
    :m_distribution(minNum, maxNum)
    {
    }

    template<typename T>
    NumType get(T& t)
    {
        return m_distribution(std::forward<T&>(t));
    }

private:
    std::uniform_int_distribution<NumType> m_distribution;
};

template<typename NumType>
class Distribution<NumType, DistributionPattern::UNIFORM, false>
{
public:
    Distribution(NumType minNum, NumType maxNum)
    :m_distribution(minNum, maxNum)
    {
    }

    template<typename T>
    NumType get(T& t)
    {
        return m_distribution(std::forward<T&>(t));
    }

private:
    std::uniform_real_distribution<NumType> m_distribution;
};

template<typename NumType>
class Distribution<NumType, DistributionPattern::NORMAL, false>
{
public:
    Distribution(NumType minNum, NumType maxNum)
    :m_distribution(minNum, maxNum)
    {
    }

    template<typename T>
    NumType get(T& t)
    {
        return m_distribution(std::forward<T&>(t));
    }

private:
    std::normal_distribution<NumType> m_distribution;
};


template<typename NumType, DistributionPattern dp = DistributionPattern::UNIFORM>
class Random
{
public:
    Random(int32_t seed = std::random_device()())
    :m_minNum(std::numeric_limits<NumType>::min()), m_maxNum(std::numeric_limits<NumType>::max())
     , m_engine(seed), m_distribution(m_minNum, m_maxNum)
    {
    }

    Random(NumType minNum, NumType maxNum, int32_t seed = std::random_device()())
    :m_minNum(minNum), m_maxNum(maxNum), m_engine(seed), m_distribution(minNum, maxNum)
    {
    }

    NumType get()
    {
        return m_distribution.get(m_engine);
    }

private:
    NumType m_minNum;
    NumType m_maxNum;
    std::mt19937_64 m_engine;
    Distribution<NumType, dp> m_distribution;
};

}}

#endif
