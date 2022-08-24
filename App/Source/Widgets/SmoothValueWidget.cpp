#include "SmoothValueWidget.h"
#include "SmoothValue.h"
#include <implot.h>
#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>




const char* SmoothValueWidget::GetWindowName() const
{
    return "Smooth Value";
}


void SmoothValueWidget::RenderContents()
{
	// show help
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::BulletText("Smooth Value.");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	// draw plot
	const ImVec2 plotSize(960.0f, 480.0f);
    if(ImPlot::BeginPlot("[Target / Value] vs [Time]", plotSize, ImPlotFlags_Crosshairs | ImPlotFlags_AntiAliased)) 
    {
        // setup legend
        ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside);

        // draw target and value data
		const int numSamples = static_cast<int>(m_timeData.size());
		const glm::vec3 targetColour = glm::rgbColor(glm::vec3(0.0f, 0.8f, 0.8f));
		const glm::vec3 valueColour = glm::rgbColor(glm::vec3(180.0f, 0.8f, 0.8f));

        ImPlot::SetNextLineStyle(ImVec4(targetColour.r, targetColour.g, targetColour.b, 1.0f));
        ImPlot::PlotLine("Target", m_timeData.data(), m_targetData.data(), numSamples);

        ImPlot::SetNextLineStyle(ImVec4(valueColour.r, valueColour.g, valueColour.b, 1.0f));
        ImPlot::PlotLine("Value", m_timeData.data(), m_valueData.data(), numSamples);

		// end plot
        ImPlot::EndPlot();
    }

	// create controls
	m_isDirty |= ImGui::SliderFloat("Smooth Time", &m_smoothTime, 0.0f, 10.0f);

	// regenerate samples if dirty
	if (m_isDirty)
		GenerateSamples();
}


void SmoothValueWidget::GenerateSamples()
{
	// initialise smooth value
	SmoothValue smoothVal(0.0f);

	// generate samples
	constexpr float deltaTime = 1.0f / 60.0f;
	constexpr glm::u32 numSamples = 60;
	constexpr float target = 1.0f;

	m_timeData.clear();
	m_targetData.clear();
	m_valueData.clear();

	m_timeData.reserve(numSamples);
	m_targetData.reserve(numSamples);
	m_valueData.reserve(numSamples);

	for (glm::u32 i = 0; i < numSamples; ++i)
	{
		m_timeData.push_back(i * deltaTime);
		m_targetData.push_back(target);
		m_valueData.push_back(smoothVal.GetValue());

		smoothVal.Update(target, m_smoothTime, deltaTime);
	}

	// reset dirty flag
	m_isDirty = false;
}