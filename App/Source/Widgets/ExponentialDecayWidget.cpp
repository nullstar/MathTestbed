#include "ExponentialDecayWidget.h"
#include "ExponentialDecay.h"
#include "EncyclopediaWidget.h"
#include <implot.h>
#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/ext.hpp>




void ExponentialDecayWidget::OnMessage(const MessageType& message)
{
}


const char* ExponentialDecayWidget::GetWindowName() const
{
	return "ExponentialDecay";
}


void ExponentialDecayWidget::RenderContents()
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
		const glm::vec3 dampedColour = glm::rgbColor(glm::vec3(0.0f, 0.8f, 0.8f));
		const glm::vec3 independentDampedColour = glm::rgbColor(glm::vec3(120.0f, 0.8f, 0.8f));
		const glm::vec3 normalisedDampedColour = glm::rgbColor(glm::vec3(240.0f, 0.8f, 0.8f));

		if (m_fixedData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(dampedColour.r, dampedColour.g, dampedColour.b, 1.0f));
			ImPlot::PlotLine("Fixed", m_timeData.data(), m_fixedData.m_valueData.data(), numSamples);
		}

		if (m_independentData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(independentDampedColour.r, independentDampedColour.g, independentDampedColour.b, 1.0f));
			ImPlot::PlotLine("Independent", m_timeData.data(), m_independentData.m_valueData.data(), numSamples);
		}

		if (m_normalisedData.m_render)
		{
			ImPlot::SetNextLineStyle(ImVec4(normalisedDampedColour.r, normalisedDampedColour.g, normalisedDampedColour.b, 1.0f));
			ImPlot::PlotLine("Normalised", m_timeData.data(), m_normalisedData.m_valueData.data(), numSamples);
		}

		// end plot
		ImPlot::EndPlot();
	}

	// create controls
	m_isDirty |= ImGui::SliderFloat("FPS", &m_fps, 1.0f, 120.0f);
	m_isDirty |= ImGui::SliderFloat("Duration", &m_duration, 0.1f, 10.0f);

	if (ImGui::CollapsingHeader("Fixed"))
	{
		ImGui::Checkbox("Render Fixed", &m_fixedData.m_render);
		m_isDirty |= ImGui::SliderFloat("Decay Factor", &m_fixedData.m_decayFactor, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Independent"))
	{
		ImGui::Checkbox("Render Independent", &m_independentData.m_render);
		ImGui::Checkbox("Use Frequency", &m_independentData.m_useFrequency);
		if (m_independentData.m_useFrequency)
			m_isDirty |= ImGui::SliderFloat("Frequency", &m_independentData.m_frequency, 1.0f, 1.0e6f);
		else
			m_isDirty |= ImGui::SliderFloat("Scale Per Second", &m_independentData.m_scalePerSecond, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Normalised"))
	{
		ImGui::Checkbox("Render Normalised", &m_normalisedData.m_render);
		m_isDirty |= ImGui::SliderFloat("Normalised Constant", &m_normalisedData.m_damping, 0.0f, 20.0f);
	}

	// regenerate samples if dirty
	if (m_isDirty)
		GenerateSamples();
}


void ExponentialDecayWidget::GenerateSamples()
{
	// convert between scale per second and frequency
	constexpr float epsilon = 1.0e-6f;
	if (m_independentData.m_useFrequency)
		m_independentData.m_scalePerSecond = (m_independentData.m_frequency > epsilon) ? 1.0f / m_independentData.m_frequency : 0.0f;
	else
		m_independentData.m_frequency = (m_independentData.m_scalePerSecond > epsilon) ? 1.0f / m_independentData.m_scalePerSecond : 0.0f;

	// initialise values
	constexpr float startVal = 1.0f;
	FixedExponentialDecay<float> fixedVal(startVal);
	IndependentExponentialDecay<float> independentVal(startVal);
	NormalisedExponentialDecay<float> normalisedVal(startVal);

	// generate samples
	const float deltaTime = 1.0f / m_fps;
	const glm::u32 numSamples = 1 + static_cast<glm::u32>(ceilf(m_duration / deltaTime));

	m_timeData.clear();
	m_timeData.reserve(numSamples);

	m_fixedData.m_valueData.clear();
	m_fixedData.m_valueData.reserve(numSamples);

	m_independentData.m_valueData.clear();
	m_independentData.m_valueData.reserve(numSamples);

	m_normalisedData.m_valueData.clear();
	m_normalisedData.m_valueData.reserve(numSamples);

	for (glm::u32 i = 0; i < numSamples; ++i)
	{
		m_timeData.push_back(i * deltaTime);

		m_fixedData.m_valueData.push_back(fixedVal.GetValue());
		m_independentData.m_valueData.push_back(independentVal.GetValue());
		m_normalisedData.m_valueData.push_back(normalisedVal.GetValue());

		fixedVal.Update(0, m_fixedData.m_decayFactor, deltaTime);
		independentVal.Update(0, m_independentData.m_scalePerSecond, deltaTime);
		normalisedVal.Update(0, m_normalisedData.m_damping, deltaTime);
	}

	// reset dirty flag
	m_isDirty = false;
}