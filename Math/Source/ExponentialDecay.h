#pragma once


#include <math.h>



template<typename T>
class FixedExponentialDecay
{
public:
	FixedExponentialDecay()
	{
	}

	FixedExponentialDecay(const T& startValue)
		: m_value(startValue)
	{
	}

	const T& Update(const T& target, float decayFactor, float deltaTime)
	{
		m_value = target + (m_value - target) * decayFactor;
		return m_value;
	}

	const T& GetValue() const	
	{
		return m_value;
	}

private:
	T m_value = {};
};	



template<typename T>
class IndependentExponentialDecay
{
public:
	IndependentExponentialDecay()
	{
	}

	IndependentExponentialDecay(const T& startValue)
		: m_value(startValue)
	{
	}

	const T& Update(const T& target, float scalePerSecond, float deltaTime)
	{
		m_value = target + (m_value - target) * powf(scalePerSecond, deltaTime);
		return m_value;
	}

	const T& GetValue() const
	{
		return m_value;
	}

private:
	T m_value = {};
};	



template<typename T>
class NormalisedExponentialDecay
{
public:
	NormalisedExponentialDecay() 
	{
	}

	NormalisedExponentialDecay(const T& startValue)
		: m_value(startValue)
	{
	}

	const T& Update(const T& target, float damping, float deltaTime, float scalePerSecond = 0.01f)
	{
		m_value = target + (m_value - target) * powf(scalePerSecond, deltaTime * damping);
		return m_value;
	}

	const T& GetValue() const
	{
		return m_value;
	}

private:
	T m_value = {};
};	