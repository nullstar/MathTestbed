#include "Widgets/Interpolation/ExponentialDecayWidget.h"
#include "Widgets/EncyclopediaWidget.h"
#include <implot.h>
#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/ext.hpp>




ExponentialDecayWidget::ExponentialDecayWidget(std::weak_ptr<MessageBus> pMessageBus) 
	: IWindowWidget(pMessageBus) 
{
	if (!m_isRealtime)
		GenerateStaticData();
}


void ExponentialDecayWidget::OnMessage(const MessageType& message)
{
}


const char* ExponentialDecayWidget::GetWindowName() const
{
	return "ExponentialDecay";
}


void ExponentialDecayWidget::RenderContents(float deltaTime)
{
	// show menu
	if (ImGui::Button("[DOCS]"))
	{
		SendMessage("OpenWindow Encyclopedia ExponentialDecay");
	}

	// draw plot
	const ImVec2 plotSize(960.0f, 480.0f);
	if (ImPlot::BeginPlot("[Value] vs [Time]", plotSize, ImPlotFlags_Crosshairs | ImPlotFlags_AntiAliased))
	{
		// setup legend
		ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside);

		// draw target and value data
		const int numSamples = static_cast<int>(m_timeData.size());
		const glm::vec3 targetColour = glm::rgbColor(glm::vec3(0.0f, 0.0f, 0.5f));
		const glm::vec3 fixedColour = glm::rgbColor(glm::vec3(0.0f, 0.8f, 0.8f));
		const glm::vec3 independentColour = glm::rgbColor(glm::vec3(120.0f, 0.8f, 0.8f));
		const glm::vec3 normalisedColour = glm::rgbColor(glm::vec3(240.0f, 0.8f, 0.8f));

		ImPlot::SetNextLineStyle(ImVec4(targetColour.r, targetColour.g, targetColour.b, 1.0f));
		ImPlot::PlotLine("Target", m_timeData.data(), m_targetData.data(), numSamples);

		if (m_fixedData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(fixedColour.r, fixedColour.g, fixedColour.b, 1.0f));
			ImPlot::PlotLine("Fixed", m_timeData.data(), m_fixedData.m_valueData.data(), numSamples);
		}

		if (m_independentData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(independentColour.r, independentColour.g, independentColour.b, 1.0f));
			ImPlot::PlotLine("Independent", m_timeData.data(), m_independentData.m_valueData.data(), numSamples);
		}

		if (m_normalisedData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(normalisedColour.r, normalisedColour.g, normalisedColour.b, 1.0f));
			ImPlot::PlotLine("Normalised", m_timeData.data(), m_normalisedData.m_valueData.data(), numSamples);
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

	if (ImGui::CollapsingHeader("Fixed"))
	{
		ImGui::Checkbox("Render Fixed", &m_fixedData.m_render);
		isDirty |= ImGui::SliderFloat("Decay Factor", &m_fixedData.m_decayFactor, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Independent"))
	{
		ImGui::Checkbox("Render Independent", &m_independentData.m_render);
		ImGui::Checkbox("Use Frequency", &m_independentData.m_useFrequency);
		if (m_independentData.m_useFrequency)
			isDirty |= ImGui::SliderFloat("Frequency", &m_independentData.m_frequency, 1.0f, 1.0e6f);
		else
			isDirty |= ImGui::SliderFloat("Scale Per Second", &m_independentData.m_scalePerSecond, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Normalised"))
	{
		ImGui::Checkbox("Render Normalised", &m_normalisedData.m_render);
		isDirty |= ImGui::SliderFloat("Normalised Constant", &m_normalisedData.m_damping, 0.0f, 20.0f);
	}

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


void ExponentialDecayWidget::ResetData()
{
	// reset timers
	m_prevSampleTime = 0.0f;
	m_currentTime = 0.0f;

	// initialise values
	constexpr float startVal = 1.0f;
	m_fixedData.m_value = FixedExponentialDecay<float>(startVal);
	m_independentData.m_value = IndependentExponentialDecay<float>(startVal);
	m_normalisedData.m_value = NormalisedExponentialDecay<float>(startVal);

	// clear samples
	m_timeData.clear();
	m_targetData.clear();
	m_fixedData.m_valueData.clear();
	m_independentData.m_valueData.clear();
	m_normalisedData.m_valueData.clear();
}


void ExponentialDecayWidget::GenerateStaticData()
{
	ResetData();

	// convert between scale per second and frequency
	constexpr float epsilon = 1.0e-6f;
	if (m_independentData.m_useFrequency)
		m_independentData.m_scalePerSecond = (m_independentData.m_frequency > epsilon) ? 1.0f / m_independentData.m_frequency : 0.0f;
	else
		m_independentData.m_frequency = (m_independentData.m_scalePerSecond > epsilon) ? 1.0f / m_independentData.m_scalePerSecond : 0.0f;

	// generate samples
	const float deltaTime = 1.0f / m_fps;
	const glm::u32 numSamples = 1 + static_cast<glm::u32>(ceilf(m_duration / deltaTime));

	m_timeData.reserve(numSamples);
	m_targetData.reserve(numSamples);
	m_fixedData.m_valueData.reserve(numSamples);
	m_independentData.m_valueData.reserve(numSamples);
	m_normalisedData.m_valueData.reserve(numSamples);

	for (glm::u32 i = 0; i < numSamples; ++i)
	{
		m_timeData.push_back(i * deltaTime);
		m_targetData.push_back(m_target);

		m_fixedData.m_valueData.push_back(m_fixedData.m_value.GetValue());
		m_independentData.m_valueData.push_back(m_independentData.m_value.GetValue());
		m_normalisedData.m_valueData.push_back(m_normalisedData.m_value.GetValue());

		m_fixedData.m_value.Update(m_target, m_fixedData.m_decayFactor, deltaTime);
		m_independentData.m_value.Update(m_target, m_independentData.m_scalePerSecond, deltaTime);
		m_normalisedData.m_value.Update(m_target, m_normalisedData.m_damping, deltaTime);
	}
}


void ExponentialDecayWidget::TakeRealtimeSample(float time)
{
	m_timeData.push_back(time);
	m_targetData.push_back(m_target);
	m_fixedData.m_valueData.push_back(m_fixedData.m_value.GetValue());
	m_independentData.m_valueData.push_back(m_independentData.m_value.GetValue());
	m_normalisedData.m_valueData.push_back(m_normalisedData.m_value.GetValue());
}


void ExponentialDecayWidget::UpdateRealtimeData(float deltaTime)
{
	m_currentTime += deltaTime;
	const float sampleDuration = 1.0f / m_fps;
	while (m_currentTime - m_prevSampleTime >= sampleDuration)
	{
		m_prevSampleTime += sampleDuration;
		m_fixedData.m_value.Update(m_target, m_fixedData.m_decayFactor, sampleDuration);
		m_independentData.m_value.Update(m_target, m_independentData.m_scalePerSecond, sampleDuration);
		m_normalisedData.m_value.Update(m_target, m_normalisedData.m_damping, sampleDuration);
		TakeRealtimeSample(m_prevSampleTime);
	}
}