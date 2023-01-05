#pragma once


#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <functional>



template<typename T>
class SecondOrderDynamics
{
public:
	SecondOrderDynamics()
	{
		SetDynamicsConstants(1.0f, 1.0f, 0.0f);
	}

	SecondOrderDynamics(float frequency, float damping, float initialResponse, const T& startValue)
	{
		SetDynamicsConstants(frequency, damping, initialResponse);
		SetValue(startValue, 0.0f);
	}

	const T& Update(const T& target, float deltaTime, std::function<void(T&)> clampVelocity = [](T&) {})
	{
		// estimate target speed
		const T targetSpeed = (target - m_prevTarget) / deltaTime;
		m_prevTarget = target;

		// determine K2 that will garauntee stable and jitter free results
		const float k1dt = m_k1 * deltaTime;
		const float stableK2 = glm::max<float>(m_k2, glm::max<float>(deltaTime * deltaTime * 0.5f + k1dt * 0.5f, k1dt));

		// update values using semi-implicit euler method
		const T valueAcceleration = (target + m_k3 * targetSpeed - m_value - m_k1 * m_valueSpeed) / stableK2; 
		m_valueSpeed += deltaTime * valueAcceleration;
		clampVelocity(m_valueSpeed);
		m_value += deltaTime * m_valueSpeed;
		return m_value; 
	}

	const T& GetValue() const
	{
		return m_value;
	}

	const T& GetValueSpeed() const
	{
		return m_valueSpeed;
	}

	void SetValue(const T& inValue, const T& inValueSpeed)
	{
		m_prevTarget = inValue;
		m_value = inValue;
		m_valueSpeed = inValueSpeed;
	}

	void SetDynamicsConstants(float frequency, float damping, float initialResponse)
	{
		const float piFreq = glm::pi<float>() * frequency;
		const float twoPiFreq = 2.0f * piFreq;  

		m_k1 = damping / piFreq;
		m_k2 = 1.0f / (twoPiFreq * twoPiFreq);
		m_k3 = initialResponse * damping / twoPiFreq;
	}

private:
	T m_prevTarget = T{};	// previous target
	T m_value = T{};		// current value
	T m_valueSpeed = T{};	// current value speed
	float m_k1 = 0.0f;		// dynamics constant 1
	float m_k2 = 0.0f;		// dynamics constant 2
	float m_k3 = 0.0f;		// dynamics constant 3
};
