#include "Widgets/Solvers/ODEWidget.h"
#include "Widgets/EncyclopediaWidget.h"
#include <implot.h>
#include <glm/gtx/color_space.hpp>
#include <glm/ext.hpp>



namespace ODESystem
{
	void SingleSpringMassSystem::GetDerivatives(FixedSpringDerivatives& derivatives) const 
	{ 
		derivatives[(int)EStateDerivative::Position] = m_massPos; 
		derivatives[(int)EStateDerivative::Speed] = m_massSpeed; 
	}


	float SingleSpringMassSystem::GetNthDerivative(const FixedSpringDerivatives& derivatives) const
	{ 
		return -(derivatives[(int)EStateDerivative::Position] * m_springConstant + derivatives[(int)EStateDerivative::Speed] * m_damping); 
	}


	void SingleSpringMassSystem::SetDerivatives(const FixedSpringDerivatives& derivatives)
	{
		m_massPos = derivatives[(int)EStateDerivative::Position];
		m_massSpeed = derivatives[(int)EStateDerivative::Speed];
	}


	void SingleSpringMassSystem::SolveAnalytical(const std::vector<float>& timeData, std::vector<float>& posData) const
	{
		const glm::u32 numAnalyticalSamples = static_cast<glm::u32>(timeData.size());

		posData.clear();
		posData.reserve(numAnalyticalSamples);

		const float angularFrequency = sqrtf(m_springConstant - 0.25f * m_damping * m_damping);
		for (glm::u32 i = 0; i < numAnalyticalSamples; ++i)
		{
			const float time = timeData[i];
			const float pos = expf(-0.5f * m_damping * time) * cosf(angularFrequency * time);
			posData.push_back(pos);
		}
	}


	void SingleSpringMassSystem::Reset(float springConstant, float damping) 
	{ 
		m_massPos = 1.0f; 
		m_massSpeed = 0.0f; 
		m_springConstant = springConstant;
		m_damping = damping;
	}



	void CoupledSpringMassSystem::GetDerivatives(CoupledSpringDerivatives& derivatives) const
	{ 
		derivatives[(int)EStateDerivative::Position].m_data[0] = m_massPos[0];
		derivatives[(int)EStateDerivative::Position].m_data[1] = m_massPos[1];
		derivatives[(int)EStateDerivative::Speed].m_data[0] = m_massSpeed[0]; 
		derivatives[(int)EStateDerivative::Speed].m_data[1] = m_massSpeed[1]; 
	}


	StateData<float, 2> CoupledSpringMassSystem::GetNthDerivative(const CoupledSpringDerivatives& derivatives) const
	{ 
		StateData<float, 2> accelerations;
		accelerations.m_data[0] = -m_springConstant * (2.0f * derivatives[(int)EStateDerivative::Position].m_data[0] - derivatives[(int)EStateDerivative::Position].m_data[1]) 
			- m_damping * derivatives[(int)EStateDerivative::Speed].m_data[0];
		accelerations.m_data[1] = -m_springConstant * (2.0f * derivatives[(int)EStateDerivative::Position].m_data[1] - derivatives[(int)EStateDerivative::Position].m_data[0]) 
			- m_damping * derivatives[(int)EStateDerivative::Speed].m_data[1];
		return accelerations;
	}


	void CoupledSpringMassSystem::SetDerivatives(const CoupledSpringDerivatives& derivatives)
	{
		m_massPos[0] = derivatives[(int)EStateDerivative::Position].m_data[0];
		m_massPos[1] = derivatives[(int)EStateDerivative::Position].m_data[1];
		m_massSpeed[0] = derivatives[(int)EStateDerivative::Speed].m_data[0];
		m_massSpeed[1] = derivatives[(int)EStateDerivative::Speed].m_data[1];
	}


	void CoupledSpringMassSystem::SolveAnalytical(const std::vector<float>& timeData, std::vector<float>& posData0, std::vector<float>& posData1) const
	{
		const glm::u32 numAnalyticalSamples = static_cast<glm::u32>(timeData.size());

		posData0.clear();
		posData1.clear();
		posData0.reserve(numAnalyticalSamples);
		posData1.reserve(numAnalyticalSamples);

		constexpr float a[2] = { 1.0f, 1.0f };		// amplitude
		constexpr float p[2] = { 0.0f, 0.0f };		// phase
		
		const float w[2] = { 
			sqrtf(m_springConstant - 0.25f * m_damping * m_damping), 
			sqrtf(3.0f * m_springConstant - 0.25f * m_damping * m_damping) };

		for (glm::u32 i = 0; i < numAnalyticalSamples; ++i)
		{
			const float time = timeData[i];
			
			const float q[2] = {
				a[0] * cosf(w[0] * time + p[0]),
				a[1] * cosf(w[1] * time + p[1]) };

			const float dampingFactor = expf(-0.5f * m_damping * time);
			posData0.push_back(dampingFactor * 0.5f * (q[0] + q[1]));
			posData1.push_back(dampingFactor * 0.5f * (q[0] - q[1]));
		}
	}


	void CoupledSpringMassSystem::Reset(float springConstant, float damping) 
	{ 
		m_massPos[0] = 1.0f; 
		m_massPos[1] = 0.0f; 
		m_massSpeed[0] = 0.0f;
		m_massSpeed[1] = 0.0f;
		m_springConstant = springConstant;
		m_damping = damping;
	}
}



ODEWidget::ODEWidget(std::weak_ptr<MessageBus> pMessageBus) 
	: IWindowWidget(pMessageBus) 
{
	// fill out method names
	m_methodNames[static_cast<int>(EMethod::ExplicitEuler)] = "Explicit Euler";
	m_methodNames[static_cast<int>(EMethod::ExplicitMidpoint)] = "Explicit Midpoint";
	m_methodNames[static_cast<int>(EMethod::ExplicitRK4)] = "Explicit RK4";
	m_methodNames[static_cast<int>(EMethod::SemiImplicitEuler)] = "Semi-Implicit Euler";
	m_methodNames[static_cast<int>(EMethod::VelocityVerlet)] = "Velocity Verlet";
	m_methodNames[static_cast<int>(EMethod::Ruth4)] = "Ruth 4";

	// fill out system names
	m_systemNames[static_cast<int>(ESystem::SingleSpringMass)] = "Single Spring Mass";
	m_systemNames[static_cast<int>(ESystem::CoupledSpringMass)] = "Coupled Spring Mass";

	// fill out spring methods
	m_singleSpringMassMethods[static_cast<int>(EMethod::ExplicitEuler)] = ODE::ExplicitEuler<float, 2>;
	m_singleSpringMassMethods[static_cast<int>(EMethod::ExplicitMidpoint)] = ODE::ExplicitMidpoint<float, 2>;
	m_singleSpringMassMethods[static_cast<int>(EMethod::ExplicitRK4)] = ODE::ExplicitRK4<float, 2>;
	m_singleSpringMassMethods[static_cast<int>(EMethod::SemiImplicitEuler)] = ODE::SemiImplicitEuler<float, 2>;
	m_singleSpringMassMethods[static_cast<int>(EMethod::VelocityVerlet)] = ODE::VelocityVerlet<float>;
	m_singleSpringMassMethods[static_cast<int>(EMethod::Ruth4)] = ODE::Ruth4<float>;

	m_coupledSpringMassMethods[static_cast<int>(EMethod::ExplicitEuler)] = ODE::ExplicitEuler<ODESystem::StateData<float, 2>, 2>;
	m_coupledSpringMassMethods[static_cast<int>(EMethod::ExplicitMidpoint)] = ODE::ExplicitMidpoint<ODESystem::StateData<float, 2>, 2>;
	m_coupledSpringMassMethods[static_cast<int>(EMethod::ExplicitRK4)] = ODE::ExplicitRK4<ODESystem::StateData<float, 2>, 2>;
	m_coupledSpringMassMethods[static_cast<int>(EMethod::SemiImplicitEuler)] = ODE::SemiImplicitEuler<ODESystem::StateData<float, 2>, 2>;
	m_coupledSpringMassMethods[static_cast<int>(EMethod::VelocityVerlet)] = ODE::VelocityVerlet<ODESystem::StateData<float, 2>>;
	m_coupledSpringMassMethods[static_cast<int>(EMethod::Ruth4)] = ODE::Ruth4<ODESystem::StateData<float, 2>>;

	// set default renderable methods
	m_methodRenderMask |= 1 << static_cast<glm::u32>(EMethod::ExplicitEuler);
	m_methodRenderMask |= 1 << static_cast<glm::u32>(EMethod::ExplicitMidpoint);
	m_methodRenderMask |= 1 << static_cast<glm::u32>(EMethod::ExplicitRK4);

	// generate starting samples
	GenerateSamples();
}


void ODEWidget::OnMessage(const MessageType& message)
{
}


const char* ODEWidget::GetWindowName() const
{
	return "Ordinary Differential Equations";
}


void ODEWidget::RenderContents(float deltaTime)
{
	// show help
	if (ImGui::Button("[DOCS]"))
	{
		SendMessage("OpenWindow Encyclopedia OrdinaryDifferentialEquations");
	}

	// draw plot
	const ImVec2 plotSize(960.0f, 480.0f);
	ImPlot::SetNextAxisLimits(ImAxis_Y1, -3.0f, 3.0f);
	ImPlot::SetNextAxisToFit(ImAxis_X1);
	if (ImPlot::BeginPlot("[Target / Value] vs [Time]", plotSize, ImPlotFlags_Crosshairs | ImPlotFlags_AntiAliased))
	{
		// setup legend and axes
		ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside);
		const int numMethodSamples = static_cast<int>(m_methodTimeData.size());
		constexpr int numMethods = static_cast<int>(EMethod::NUM_METHODS);

		if (m_system == ESystem::SingleSpringMass)
		{
			// draw analytical data
			const int numAnalyticalSamples = static_cast<int>(m_analyticalTimeData.size());
			const glm::vec3 analyticalColour = glm::rgbColor(glm::vec3(0.0f, 0.0f, 0.5f));
			ImPlot::SetNextLineStyle(ImVec4(analyticalColour.r, analyticalColour.g, analyticalColour.b, 1.0f));
			ImPlot::PlotLine("Analytical", m_analyticalTimeData.data(), m_analyticalSingleSpringMassData.data(), numAnalyticalSamples);

			// draw method data
			for (int methodIndex = 0; methodIndex < numMethods; ++methodIndex)
			{
				if (m_methodRenderMask & (1 << methodIndex))
				{
					const glm::vec3 methodColour = glm::rgbColor(glm::vec3(360.0f * methodIndex / (float)numMethods, 0.8f, 0.8f));
					ImPlot::SetNextLineStyle(ImVec4(methodColour.r, methodColour.g, methodColour.b, 1.0f));
					ImPlot::PlotLine(m_methodNames[methodIndex], m_methodTimeData.data(), m_singleSpringMassData[methodIndex].data(), numMethodSamples);
				}
			}
		}
		else if (m_system == ESystem::CoupledSpringMass)
		{
			// draw analytical data
			const int numAnalyticalSamples = static_cast<int>(m_analyticalTimeData.size());
			const glm::vec3 analyticalColour = glm::rgbColor(glm::vec3(0.0f, 0.0f, 0.5f));
			ImPlot::SetNextLineStyle(ImVec4(analyticalColour.r, analyticalColour.g, analyticalColour.b, 1.0f));
			ImPlot::PlotLine("Analytical_0", m_analyticalTimeData.data(), m_analyticalCoupledSpringMassData[0].data(), numAnalyticalSamples);
			ImPlot::SetNextLineStyle(ImVec4(analyticalColour.r, analyticalColour.g, analyticalColour.b, 1.0f));
			ImPlot::PlotLine("Analytical_1", m_analyticalTimeData.data(), m_analyticalCoupledSpringMassData[1].data(), numAnalyticalSamples);

			// draw method data
			for (int methodIndex = 0; methodIndex < numMethods; ++methodIndex)
			{
				if (m_methodRenderMask & (1 << methodIndex))
				{
					const std::string name0 = std::string(m_methodNames[methodIndex]) + "_0";
					const std::string name1 = std::string(m_methodNames[methodIndex]) + "_1";

					const glm::vec3 methodColour = glm::rgbColor(glm::vec3(360.0f * methodIndex / (float)numMethods, 0.8f, 0.8f));
					ImPlot::SetNextLineStyle(ImVec4(methodColour.r, methodColour.g, methodColour.b, 1.0f));
					ImPlot::PlotLine(name0.c_str(), m_methodTimeData.data(), m_coupledSpringMassData[methodIndex][0].data(), numMethodSamples);
					ImPlot::SetNextLineStyle(ImVec4(methodColour.r, methodColour.g, methodColour.b, 1.0f));
					ImPlot::PlotLine(name1.c_str(), m_methodTimeData.data(), m_coupledSpringMassData[methodIndex][1].data(), numMethodSamples);
				}
			}
		}

		// end plot
		ImPlot::EndPlot();
	}

	// create controls
	bool isDirty = false;
            
	int systemIndex = static_cast<int>(m_system);
	ImGui::Combo("System", &systemIndex, &m_systemNames[0], static_cast<int>(ESystem::NUM_SYSTEMS));
	m_system = static_cast<ESystem>(systemIndex);

	isDirty |= ImGui::SliderFloat("Duration", &m_duration, 0.1f, 120.0f);
	isDirty |= ImGui::SliderFloat("FPS", &m_fps, 1.0f, 240.0f);
	isDirty |= ImGui::SliderFloat("Spring Constant", &m_springConstant, 1.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	isDirty |= ImGui::SliderFloat("Damping", &m_damping, 0.0f, 2.0f);

	ImGui::Separator();

	for (int methodIndex = 0; methodIndex < static_cast<int>(EMethod::NUM_METHODS); ++methodIndex)
	{
		const glm::u32 methodBit = (1 << methodIndex);
		bool renderMethod = (m_methodRenderMask & methodBit) != 0;
		ImGui::Checkbox(m_methodNames[methodIndex], &renderMethod);
		if (renderMethod) { m_methodRenderMask |= methodBit; }
		else { m_methodRenderMask &= ~methodBit; }
	}

	// generate samples if needed
	if (isDirty)
		GenerateSamples();
}


void ODEWidget::GenerateSamples()
{
	// generate analytical samples
	constexpr float analyticalDeltaTime = 1.0f / 120.0f;
	const glm::u32 numAnalyticalSamples = 1 + static_cast<glm::u32>(ceilf(m_duration / analyticalDeltaTime));

	m_analyticalTimeData.clear();
	m_analyticalTimeData.reserve(numAnalyticalSamples);
	for (glm::u32 i = 0; i < numAnalyticalSamples; ++i)
		m_analyticalTimeData.push_back(i * analyticalDeltaTime);

	ODESystem::SingleSpringMassSystem singleSpringMassSystem;
	singleSpringMassSystem.Reset(m_springConstant, m_damping);
	singleSpringMassSystem.SolveAnalytical(m_analyticalTimeData, m_analyticalSingleSpringMassData);

	ODESystem::CoupledSpringMassSystem coupledSpringMassSystem;
	coupledSpringMassSystem.Reset(m_springConstant, m_damping);
	coupledSpringMassSystem.SolveAnalytical(m_analyticalTimeData, m_analyticalCoupledSpringMassData[0], m_analyticalCoupledSpringMassData[1]);

	// generate method data
	const float methodDeltaTime = 1.0f / m_fps;
	const glm::u32 numMethodSamples = 1 + static_cast<glm::u32>(ceilf(m_duration / methodDeltaTime));

	m_methodTimeData.clear();
	m_methodTimeData.reserve(numMethodSamples);

	for (glm::u32 i = 0; i < numMethodSamples; ++i)
	{
		const float time = i * methodDeltaTime;
		m_methodTimeData.push_back(time);
	}

	for (int methodIndex = 0; methodIndex < static_cast<int>(EMethod::NUM_METHODS); ++methodIndex)
	{
		singleSpringMassSystem.Reset(m_springConstant, m_damping);

		std::vector<float>& data = m_singleSpringMassData[methodIndex];
		data.clear();
		data.reserve(numMethodSamples);

		const ODESystem::FixedSpringMethod springMethod = m_singleSpringMassMethods[methodIndex];
		for (glm::u32 i = 0; i < numMethodSamples; ++i)
		{
			data.push_back(singleSpringMassSystem.m_massPos);
			springMethod(singleSpringMassSystem, methodDeltaTime);
		}
	}

	for (int methodIndex = 0; methodIndex < static_cast<int>(EMethod::NUM_METHODS); ++methodIndex)
	{
		coupledSpringMassSystem.Reset(m_springConstant, m_damping);

		std::vector<float>& data0 = m_coupledSpringMassData[methodIndex][0];
		std::vector<float>& data1 = m_coupledSpringMassData[methodIndex][1];
		data0.clear();
		data0.reserve(numMethodSamples);
		data1.clear();
		data1.reserve(numMethodSamples);

		const ODESystem::FreeSpringMethod springMethod = m_coupledSpringMassMethods[methodIndex];
		for (glm::u32 i = 0; i < numMethodSamples; ++i)
		{
			data0.push_back(coupledSpringMassSystem.m_massPos[0]);
			data1.push_back(coupledSpringMassSystem.m_massPos[1]);
			springMethod(coupledSpringMassSystem, methodDeltaTime);
		}
	}
}