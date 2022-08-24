#pragma once



class SmoothValue
{
public:
	SmoothValue();
	SmoothValue(float startValue);

	float Update(float target, float smoothTime, float deltaTime);
	float GetValue() const;

private:
	float m_value = 0.0f;
	float m_valueSpeed = 0.0f;
};	