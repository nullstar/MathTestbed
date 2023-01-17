#pragma once


#include <math.h>
#include <functional>



namespace NewtonRaphson
{
	template<typename T>
	T Solve(const T& x, std::function<T(const T& value)> g0, std::function<T(const T& value)> g1, std::function<bool(const T& x)> isNearlyZero, std::function<bool(const T& error)> shouldStopEvaluation)
	{
		T y = x;
		while (true)
		{
			const T yPrev = y;
			const T divisor = g1(y);

			if (isNearlyZero(divisor))
				return y;

			y -= g0(y) / divisor;

			if (shouldStopEvaluation(y - yPrev))
				return y;
		}
	}


	template<typename T>
	T SolveSecant(const T& x0, const T& x1, std::function<T(const T& value)> g0, std::function<bool(const T& value)> isNearlyZero, std::function<bool(const T& error)> shouldStopEvaluation)
	{
		T y0 = x0;
		T y1 = x1;
		T y2 = (y0 + y1) * 0.5f;

		while (true)
		{
			const T divisor = g0(y1) - g0(y0);
			if (isNearlyZero(divisor))
				return y2;

			y2 = y1 - g0(y1) * (y1 - y0) / divisor;
			if (shouldStopEvaluation(y1 - y0))
				return y2;

			y0 = y1;
			y1 = y2;
		}
	}
}
