#pragma once


#include <array>
#include <vector>
#include "Solvers/RootFinding.h"



namespace CubicHermite
{
	enum class ETurningPointType : unsigned int
	{
		Maximum,
		Minimum,
		Inflection
	};


	struct TurningPoint
	{
		TurningPoint(float key, float value, ETurningPointType type) : m_key(key), m_value(value), m_type(type) {}
		float m_key = 0.0f;
		float m_value = 0.0f;
		ETurningPointType m_type = ETurningPointType::Maximum;
	};


	float EvaluateSegment(const std::array<float, 2>& p, const std::array<float, 2>& m, float t);
	float EvaluateSegmentDerivative(const std::array<float, 2>& p, const std::array<float, 2>& m, float t);
	RootFinding::EError FindSegmentTurningPoints(const std::array<float, 2>& p, const std::array<float, 2>& m, std::vector<TurningPoint>& turningPoints);
	RootFinding::EError FindSegmentCrossingPoints(const std::array<float, 2>& p, const std::array<float, 2>& m, std::vector<float>& crossingPoints);
}