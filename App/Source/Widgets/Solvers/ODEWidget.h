#pragma once


#include "Solvers/ODE.h"
#include "Widgets/WindowWidget.h"
#include <array>
#include <glm/glm.hpp>



namespace ODESystem
{
	enum class EStateDerivative : unsigned int
	{
		Position,
		Speed,

		NUM_DERIVATIVES
	};


	template<typename T, unsigned int N>
	struct StateData
	{
		std::array<T, N> m_data = {};

		StateData<T, N> operator * (float rhs) const
		{
			StateData<T, N> result;
			for (unsigned int i = 0; i < N; ++i)
				result.m_data[i] = m_data[i] * rhs;
			return result;
		}

		StateData<T, N> operator + (const StateData<T, N>& rhs) const
		{
			StateData<T, N> result;
			for (unsigned int i = 0; i < N; ++i)
				result.m_data[i] = m_data[i] + rhs.m_data[i];
			return result;
		}

		StateData<T, N>& operator += (const StateData<T, N>& rhs)
		{
			for (unsigned int i = 0; i < N; ++i)
				m_data[i] += rhs.m_data[i];
			return *this;
		}
	};


	typedef std::function<void(ODE::IState<float, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)>& state, float stepSize)> FixedSpringMethod;
	typedef std::function<void(ODE::IState<StateData<float, 2>, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)>& state, float stepSize)> FreeSpringMethod;

	typedef std::array<float, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)> FixedSpringDerivatives;
	typedef std::array<StateData<float, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)>, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)> CoupledSpringDerivatives;


	struct SingleSpringMassSystem : ODE::IState<float, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)>
	{
		float m_massPos = 1.0f;
		float m_massSpeed = 0.0f;
		float m_springConstant = 1.0f;
		float m_damping = 0.0f;

		virtual void GetDerivatives(FixedSpringDerivatives& derivatives) const override;
		virtual float GetNthDerivative(const FixedSpringDerivatives& derivatives) const override;
		virtual void SetDerivatives(const FixedSpringDerivatives& derivatives) override;
		void SolveAnalytical(const std::vector<float>& timeData, std::vector<float>& posData) const;
		void Reset(float springConstant, float damping);
	};


	struct CoupledSpringMassSystem : ODE::IState<StateData<float, 2>, static_cast<unsigned int>(EStateDerivative::NUM_DERIVATIVES)>
	{
		float m_massPos[2] = { 1.0f, 1.0f };
		float m_massSpeed[2] = { 0.0f, 0.0f };
		float m_springConstant = 1.0f;
		float m_damping = 0.0f;

		virtual void GetDerivatives(CoupledSpringDerivatives& derivatives) const override;
		virtual StateData<float, 2> GetNthDerivative(const CoupledSpringDerivatives& derivatives) const override;
		virtual void SetDerivatives(const CoupledSpringDerivatives& derivatives) override;
		void SolveAnalytical(const std::vector<float>& timeData, std::vector<float>& posData0, std::vector<float>& posData1) const;
		void Reset(float springConstant, float damping);
	};
}



class ODEWidget : public IWindowWidget
{
public:
	ODEWidget(std::weak_ptr<MessageBus> pMessageBus);

private:
	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents(float deltaTime) override;

	void GenerateSamples();

	enum class EMethod : unsigned int
	{
		ExplicitEuler,
		ExplicitMidpoint,
		ExplicitRK4,
		SemiImplicitEuler,
		VelocityVerlet,
		Ruth4,

		NUM_METHODS
	};

	enum class ESystem : unsigned int
	{
		SingleSpringMass,
		CoupledSpringMass,

		NUM_SYSTEMS
	};

	std::vector<float> m_analyticalTimeData;
	std::vector<float> m_methodTimeData;
	std::vector<float> m_analyticalSingleSpringMassData;
	std::vector<float> m_analyticalCoupledSpringMassData[2];
	std::array<const char*, static_cast<int>(EMethod::NUM_METHODS)> m_methodNames;
	std::array<const char*, static_cast<int>(ESystem::NUM_SYSTEMS)> m_systemNames;
	std::array<std::vector<float>, static_cast<int>(EMethod::NUM_METHODS)> m_singleSpringMassData;
	std::array<std::vector<float>[2], static_cast<int>(EMethod::NUM_METHODS)> m_coupledSpringMassData;
	std::array<ODESystem::FixedSpringMethod, static_cast<int>(EMethod::NUM_METHODS)> m_singleSpringMassMethods;
	std::array<ODESystem::FreeSpringMethod, static_cast<int>(EMethod::NUM_METHODS)> m_coupledSpringMassMethods;

	ESystem m_system = ESystem::SingleSpringMass;
	float m_duration = 60.0f;
	float m_fps = 60.0f;
	float m_springConstant = 1.0f;
	float m_damping = 0.1f;
	glm::u32 m_methodRenderMask = 0;
};