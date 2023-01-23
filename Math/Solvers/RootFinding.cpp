#include "Solvers/RootFinding.h"



namespace RootFinding
{
	Result<1> NewtonRaphson(float x, std::function<float(const float& value)> g0, std::function<float(const float& value)> g1, float errorTolerance, unsigned int maxIterations)
	{
		float y = x;
		unsigned int numIterations = 0;

		while (++numIterations <= maxIterations)
		{
			const float yPrev = y;
			const float divisor = g1(y);

			constexpr float epsilon = 1.0e-7f;
			if (fabsf(divisor) < epsilon)
			{
				Result<1> result;
				result.AddError(EError::ZeroDivisor);
				return result;
			}

			y -= g0(y) / divisor;

			if (fabsf(y - yPrev) < errorTolerance)
			{
				Result<1> result;
				result.AddValue(y);
				result.m_numValues = 1;
				return result;
			}
		}

		Result<1> result;
		result.AddValue(y);
		result.m_numValues = 1;
		result.AddError(EError::MaxIterationsReached);
		return result;
	}


	Result<1> Secant(float x0, float x1, std::function<float(const float& value)> g0, float errorTolerance, unsigned int maxIterations)
	{
		float y0 = x0;
		float y1 = x1;
		float y2 = (y0 + y1) * 0.5f;
		unsigned int numIterations = 0;

		while (++numIterations <= maxIterations)
		{
			constexpr float epsilon = 1.0e-7f;
			const float divisor = g0(y1) - g0(y0);
			if (fabsf(divisor) < epsilon)
			{
				Result<1> result;
				result.AddError(EError::ZeroDivisor);
				return result;
			}

			y2 = y1 - g0(y1) * (y1 - y0) / divisor;
			if (fabsf(y1 - y0) < errorTolerance)
			{
				Result<1> result;
				result.AddValue(y2);
				result.m_numValues = 1;
				return result;
			}

			y0 = y1;
			y1 = y2;
		}

		Result<1> result;
		result.AddValue(y2);
		result.m_numValues = 1;
		result.AddError(EError::MaxIterationsReached);
		return result;
	}


	Result<2> Quadratic(float a, float b, float c)
	{
		constexpr float epsilon = 1.0e-7f;
		if (fabsf(a) < epsilon)
		{
			Result<2> result;
			result.AddError(EError::ZeroDivisor);
			return result;
		}

		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			return Result<2>();
		}

		const float scale = 1.0f / (2.0f * a);
		if (discriminant < epsilon)
		{
			Result<2> result;
			result.AddValue(-b * scale);
			result.m_numValues = 1;
			return result;
		}

		const float discriminantRoot = sqrtf(discriminant);
		Result<2> result;
		result.AddValue((-b + discriminantRoot) * scale);
		result.AddValue((-b - discriminantRoot) * scale);
		result.m_numValues = 2;
		return result;
	}


	Result<3> Cubic(float a, float b, float c, float d, float x, float errorTolerance, unsigned int maxIterations)
	{
		Result<3> result;

		// find first root using iterative solver
		const auto g0 = [a, b, c, d](const float& t) -> float
		{
			const float t2 = t * t;
			const float t3 = t2 * t;
			return a * t3 + b * t2 + c * t + d;
		};

		const auto g1 = [a, b, c](const float& t) -> float
		{
			const float t2 = t * t;
			return 3.0f * a * t2 + 2.0f * b * t + c;
		};

		const Result<1> iterativeResult = NewtonRaphson(x, g0, g1, errorTolerance, maxIterations);
		result.AddError(iterativeResult.m_errorMask);
		if (iterativeResult.m_numValues == 0)
		{
			return result;
		}
		result.AddValue(iterativeResult.m_values[0]);
		result.m_numValues = 1;

		// find other roots using factor theorem
		// calculate factored coefficients
		const float af = a;
		const float bf = a * result.m_values[0] + b;
		const float cf = bf * result.m_values[0] + c;

		// calculate remaining roots by solving factored quadratic
		const Result<2> quadraticResult = Quadratic(af, bf, cf);
		result.AddError(quadraticResult.m_errorMask);
		for (unsigned int i = 0; i < quadraticResult.m_numValues; ++i)
			result.AddValue(quadraticResult.m_values[i]);

		return result;
	}
}