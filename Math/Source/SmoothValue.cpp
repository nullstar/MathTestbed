#include "SmoothValue.h"



SmoothValue::SmoothValue()
{
}


SmoothValue::SmoothValue(float startValue)
	: m_value(startValue)
{
}


float SmoothValue::Update(float target, float smoothTime, float deltaTime)
{
	constexpr float epsilon = 1.0e-6f;
	if (smoothTime < epsilon)
	{
		m_valueSpeed = 0.0f;
		m_value = target;
	}
	else
	{
		const float a = 2.0f / smoothTime;
		const float b = a * deltaTime;
		const float b2 = b * b;
		const float b3 = b2 * b;
		const float c = 1.0f / (1.0f + b + 0.48f * b2 + 0.235f * b3);

		const float diff = m_value - target;
		const float d = (m_valueSpeed + a * diff) * deltaTime;
		m_valueSpeed = (m_valueSpeed - a * d) * c;

		float nextValue = target + (diff + d) * c;
		if (target - m_value > 0.0f == nextValue > target)
		{
			nextValue = target;
			m_valueSpeed = (nextValue - target) / deltaTime;
		}
		m_value = nextValue;
	}

	return m_value;
}


float SmoothValue::GetValue() const
{
	return m_value;
}