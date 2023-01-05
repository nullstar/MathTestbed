#include "SecondOrderDynamicsWidget.h"
#include "EncyclopediaWidget.h"
#include <implot.h>
#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>




SecondOrderDynamicsWidget::SecondOrderDynamicsWidget(std::weak_ptr<MessageBus> pMessageBus) 
	: IWindowWidget(pMessageBus) 
{
	if (!m_isRealtime)
		GenerateStaticData();
}


void SecondOrderDynamicsWidget::OnMessage(const MessageType& message)
{
}


const char* SecondOrderDynamicsWidget::GetWindowName() const
{
	return "Second Order Dynamics";
}


void SecondOrderDynamicsWidget::RenderContents(float deltaTime)
{
	// show help
	if (ImGui::Button("[DOCS]"))
	{
		SendMessage("OpenWindow Encyclopedia SecondOrderDynamics");
	}

	// draw plot
	const ImVec2 plotSize(960.0f, 480.0f);
	if (ImPlot::BeginPlot("[Target / Value] vs [Time]", plotSize, ImPlotFlags_Crosshairs | ImPlotFlags_AntiAliased))
	{
		// setup legend
		ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside);

		// draw target and value data
		const int numSamples = static_cast<int>(m_timeData.size());
		const glm::vec3 targetColour = glm::rgbColor(glm::vec3(0.0f, 0.8f, 0.8f));
		const glm::vec3 valueColour = glm::rgbColor(glm::vec3(120.0f, 0.8f, 0.8f));
		const glm::vec3 speedColour = glm::rgbColor(glm::vec3(240.0f, 0.8f, 0.8f));

		ImPlot::SetNextLineStyle(ImVec4(targetColour.r, targetColour.g, targetColour.b, 1.0f));
		ImPlot::PlotLine("Target", m_timeData.data(), m_targetData.data(), numSamples);

		ImPlot::SetNextLineStyle(ImVec4(valueColour.r, valueColour.g, valueColour.b, 1.0f));
		ImPlot::PlotLine("Value", m_timeData.data(), m_valueData.data(), numSamples);

		if (m_renderSpeed)
		{
			ImPlot::SetNextLineStyle(ImVec4(speedColour.r, speedColour.g, speedColour.b, 1.0f));
			ImPlot::PlotLine("Speed", m_timeData.data(), m_valueSpeedData.data(), numSamples);
		}

		// end plot
		ImPlot::EndPlot();
	}

	// create controls
	bool isDirty = false;

	if (ImGui::Checkbox("Realtime", &m_isRealtime))
	{
		isDirty = true;
		if (m_isRealtime)
		{
			ResetData();
			TakeRealtimeSample(0.0f);
		}
	}

	isDirty |= ImGui::SliderFloat("Target", &m_target, -10.0f, 10.0f);
	isDirty |= ImGui::SliderFloat("Duration", &m_duration, 0.1f, 30.0f);
	isDirty |= ImGui::SliderFloat("FPS", &m_fps, 1.0f, 120.0f);

	ImGui::Separator();

	isDirty |= ImGui::SliderFloat("Frequency", &m_frequency, 0.0f, 10.0f);
	isDirty |= ImGui::SliderFloat("Damping", &m_damping, 0.0f, 10.0f);
	isDirty |= ImGui::SliderFloat("Initial Response", &m_initialResponse, -10.0f, 10.0f);

	ImGui::Separator();

	ImGui::Checkbox("Render Speed", &m_renderSpeed);
	isDirty |= ImGui::Checkbox("Clamp Speed", &m_clampSpeed);
	if (m_clampSpeed)
		isDirty |= ImGui::SliderFloat("Max Speed", &m_maxSpeed, 0.0f, 10.0f);		

	// update samples
	if (m_isRealtime)
	{
		UpdateRealtimeData(deltaTime);

		if (m_currentTime > m_duration)
			ResetData();
	}
	else if (isDirty)
	{
		GenerateStaticData();
	}
}


void SecondOrderDynamicsWidget::ResetData()
{
	// reset timers
	m_prevSampleTime = 0.0f;
	m_currentTime = 0.0f;

	// initialise second order dynamics
	m_dynamics = SecondOrderDynamics<float>(m_frequency, m_damping, m_initialResponse, 0.0f);

	// clear samples
	m_timeData.clear();
	m_targetData.clear();
	m_valueData.clear();
	m_valueSpeedData.clear();
}


void SecondOrderDynamicsWidget::GenerateStaticData()
{
	ResetData();

	// generate samples
	const float deltaTime = 1.0f / m_fps;
	const glm::u32 numSamples = 1 + static_cast<glm::u32>(ceilf(m_duration / deltaTime));

	m_timeData.reserve(numSamples);
	m_targetData.reserve(numSamples);
	m_valueData.reserve(numSamples);
	m_valueSpeedData.reserve(numSamples);

	const auto clampSpeed = [this](float& speed) -> void
	{
		if (m_clampSpeed)
			speed = glm::clamp<float>(speed, -m_maxSpeed, m_maxSpeed);
	};

	for (glm::u32 i = 0; i < numSamples; ++i)
	{
		m_timeData.push_back(i * deltaTime);
		m_targetData.push_back(m_target);
		m_valueData.push_back(m_dynamics.GetValue());
		m_valueSpeedData.push_back(m_dynamics.GetValueSpeed());

		m_dynamics.Update(m_target, deltaTime, clampSpeed);
	}
}


void SecondOrderDynamicsWidget::TakeRealtimeSample(float time)
{
	m_timeData.push_back(time);
	m_targetData.push_back(m_target);
	m_valueData.push_back(m_dynamics.GetValue());
	m_valueSpeedData.push_back(m_dynamics.GetValueSpeed());
}


void SecondOrderDynamicsWidget::UpdateRealtimeData(float deltaTime)
{
	m_dynamics.SetDynamicsConstants(m_frequency, m_damping, m_initialResponse);

	const auto clampSpeed = [this](float& speed) -> void
	{
		if (m_clampSpeed)
			speed = glm::clamp<float>(speed, -m_maxSpeed, m_maxSpeed);
	};

	m_currentTime += deltaTime;
	const float sampleDuration = 1.0f / m_fps;
	while (m_currentTime - m_prevSampleTime >= sampleDuration)
	{
		m_prevSampleTime += sampleDuration;
		m_dynamics.Update(m_target, sampleDuration, clampSpeed);
		TakeRealtimeSample(m_prevSampleTime);
	}
}