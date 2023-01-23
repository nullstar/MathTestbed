#include "Splines/CubicHermite.h"



namespace CubicHermite
{
	float EvaluateSegment(const std::array<float, 2>& p, const std::array<float, 2>& m, float t)
	{
		const float t2 = t * t;
		const float t3 = t2 * t;
		return p[0] * (2.0f * t3 - 3.0f * t2 + 1.0f)
			+ m[0] * (t3 - 2.0f * t2 + t)
			+ p[1] * (-2.0f * t3 + 3.0f * t2)
			+ m[1] * (t3 - t2);
			
	}


	float EvaluateSegmentDerivative(const std::array<float, 2>& p, const std::array<float, 2>& m, float t)
	{
		const float t2 = t * t;
		return p[0] * (6.0f * t2 - 6.0f * t)
			+ m[0] * (3.0f * t2 - 4.0f * t + 1.0f)
			+ p[1] * (-6.0f * t2 + 6.0f * t)
			+ m[1] * (3.0f * t2 - 2.0f * t);
	}


	RootFinding::EError FindSegmentTurningPoints(const std::array<float, 2>& p, const std::array<float, 2>& m, std::vector<TurningPoint>& turningPoints)
	{
		// calculate coefficients of spline derivative
		const float a = 3.0f * (2.0f * p[0] + m[0] - 2.0f * p[1] + m[1]);
		const float b = 2.0f * (-3.0f * p[0] + 3.0f * p[1] - 2.0f * m[0] - m[1]);
		const float c = m[0];

		// find the roots of the spline derivative
		const RootFinding::Result<2> roots = RootFinding::Quadratic(a, b, c);

		// assign turning points
		turningPoints.clear();
		for (unsigned int i = 0; i < roots.m_numValues; ++i)
		{
			const float root = roots.m_values[i];
			if (root >= 0.0f && root <= 1.0f)
			{
				// evaluate segments to determine turning point value
				const float value = EvaluateSegment(p, m, root);
				
				// use second spline derivative to determine turning point type
				constexpr float turningTolerance = 1.0e-7f;
				const float secondDerivative = 2.0f * a * root + b;
				const ETurningPointType turningType = (secondDerivative < -turningTolerance) ? ETurningPointType::Maximum
					: (secondDerivative > turningTolerance) ? ETurningPointType::Minimum : ETurningPointType::Inflection;

				// add turning point
				turningPoints.push_back(TurningPoint(root, value, turningType));
			}
		}
		return roots.m_errorMask;
	}


	RootFinding::EError FindSegmentCrossingPoints(const std::array<float, 2>& p, const std::array<float, 2>& m, std::vector<float>& crossingPoints)
	{
		// calculate coefficients of spline 
		const float a = 2.0f * p[0] + m[0] - 2.0f * p[1] + m[1];
		const float b = -3.0f * p[0] + 3.0f * p[1] - 2.0f * m[0] - m[1];
		const float c = m[0];
		const float d = p[0];

		// find the roots of the spline
		RootFinding::Result<3> roots = RootFinding::Cubic(a, b, c, d, 0.5f, 1.0e-3f, 100);

		if(!roots.IsValid())
			roots = RootFinding::Cubic(a, b, c, d, 0.0f, 1.0e-3f, 100);

		if(!roots.IsValid())
			roots = RootFinding::Cubic(a, b, c, d, 1.0f, 1.0e-3f, 100);

		// assign crossing points
		crossingPoints.clear();
		for (unsigned int i = 0; i < roots.m_numValues; ++i)
		{
			const float root = roots.m_values[i];
			if (root >= 0.0f && root <= 1.0f)
				crossingPoints.push_back(root);
		}
		return roots.m_errorMask;
	}
}