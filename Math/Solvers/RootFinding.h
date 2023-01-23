#pragma once


#include <math.h>
#include <functional>
#include <array>



namespace RootFinding
{
	enum class EError : unsigned int
	{
		None					= 0,
		ZeroDivisor				= 1 << 0,
		MaxIterationsReached	= 1 << 1
	};


	template<unsigned int N>
	struct Result
	{
		bool IsValid() const 
		{ 
			return m_errorMask == EError::None; 
		}

		void AddError(EError error)
		{
			m_errorMask = EError((unsigned int)m_errorMask | (unsigned int)error);
		}

		void AddValue(float value)
		{
			m_values[m_numValues] = value;
			++m_numValues;
		}

		std::array<float, N> m_values = { };
		unsigned int m_numValues = 0;
		EError m_errorMask = EError::None;
	};


	Result<1> NewtonRaphson(float x, std::function<float(const float& value)> g0, std::function<float(const float& value)> g1, float errorTolerance, unsigned int maxIterations);
	Result<1> Secant(float x0, float x1, std::function<float(const float& value)> g0, float errorTolerance, unsigned int maxIterations);
	Result<2> Quadratic(float a, float b, float c);
	Result<3> Cubic(float a, float b, float c, float d, float x, float errorTolerance, unsigned int maxIterations);
}
