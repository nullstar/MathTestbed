#include "Widgets/Solvers/RootFindingWidget.h"
#include "Widgets/EncyclopediaWidget.h"
#include <implot.h>
#include <glm/gtx/color_space.hpp>



RootFindingWidget::RootFindingWidget(std::weak_ptr<MessageBus> pMessageBus)
	: IWindowWidget(pMessageBus)
{
	m_controlNodes = {
		glm::dvec2(0.0, -0.1),
		glm::dvec2(0.2, 0.1),
		glm::dvec2(0.8, -0.1),
		glm::dvec2(1.0, 0.1) };
	GenerateSplineData();
}


void RootFindingWidget::OnMessage(const MessageType& message)
{
}


const char* RootFindingWidget::GetWindowName() const
{
	return "Root Finding";
}


void RootFindingWidget::RenderContents(float deltaTime)
{
	bool isDirty = false;

	// draw plot
	const ImVec2 plotSize(960.0f, 480.0f);
	if (ImPlot::BeginPlot("##Spline", plotSize, ImPlotFlags_AntiAliased))
	{
		// render curve
		const glm::vec3 splineRgb = glm::rgbColor(glm::vec3(180.0f, 0.8f, 0.8f));
		const glm::vec3 tangentRgb = glm::rgbColor(glm::vec3(0.0f, 0.0f, 0.5f));
		const ImVec4 splineColour(splineRgb.x, splineRgb.y, splineRgb.z, 1.0f);
		const ImVec4 tangentColour(tangentRgb.x, tangentRgb.y, tangentRgb.z, 1.0f);

		ImPlot::SetNextLineStyle(tangentColour);
        ImPlot::PlotLine("##m0", &m_controlNodes[0].x, &m_controlNodes[0].y, 2, 0, sizeof(glm::dvec2));
		ImPlot::SetNextLineStyle(tangentColour);
        ImPlot::PlotLine("##m1", &m_controlNodes[2].x, &m_controlNodes[2].y, 2, 0, sizeof(glm::dvec2));

		ImPlot::SetNextLineStyle(splineColour);
		ImPlot::PlotLine("##HermiteCurve", m_keyData.data(), m_valueData.data(), (int)m_valueData.size());

		isDirty |= ImPlot::DragPoint(1, &m_controlNodes[1].x, &m_controlNodes[1].y, tangentColour, 4);
		isDirty |= ImPlot::DragPoint(2, &m_controlNodes[2].x, &m_controlNodes[2].y, tangentColour, 4);

		const glm::dvec2 delta0 = m_controlNodes[1] - m_controlNodes[0];
		const glm::dvec2 delta1 = m_controlNodes[2] - m_controlNodes[3];

		if (ImPlot::DragPoint(0, &m_controlNodes[0].x, &m_controlNodes[0].y, splineColour, 4))
		{
			isDirty = true;
			m_controlNodes[1] = m_controlNodes[0] + delta0;
		}

		if (ImPlot::DragPoint(3, &m_controlNodes[3].x, &m_controlNodes[3].y, splineColour, 4))
		{
			isDirty = true;
			m_controlNodes[2] = m_controlNodes[3] + delta1;
		}

		// render turning points
		const glm::vec3 turningRgb = glm::rgbColor(glm::vec3(m_turningError == RootFinding::EError::None ? 60.0f : 0.0f, 0.8f, 0.8f));
		const ImVec4 turningColour(turningRgb.x, turningRgb.y, turningRgb.z, 1.0f);
		const ImVec2 turningOffsets[3] = { ImVec2(0, -15), ImVec2(0, 15), ImVec2(0, 0) };
		const char* turningNames[3] = { "MAX TURN", "MIN TURN", "INFLECTION" };

		for (const CubicHermite::TurningPoint& point : m_turningPoints)
		{
			const int typeIndex = (int)point.m_type;
			ImPlot::Annotation(point.m_key, point.m_value, turningColour, turningOffsets[typeIndex], false, turningNames[typeIndex]);
		}

		// render crossing keys
		const glm::vec3 crossingRgb = glm::rgbColor(glm::vec3(m_crossingError == RootFinding::EError::None ? 60.0f : 0.0f, 0.8f, 0.8f));
		const ImVec4 crossingColour(crossingRgb.x, crossingRgb.y, crossingRgb.z, 1.0f);
		for (float key : m_crossingKeys)
			ImPlot::Annotation(key, 0.0f, crossingColour, ImVec2(-15, -15), false, "CROSS");

		// end plot
		ImPlot::EndPlot();
	}

	// generate data
	if (isDirty)
		GenerateSplineData();
}


void RootFindingWidget::GenerateSplineData()
{
	// get spline data from control nodes
	std::array<float, 2> keys;
	std::array<float, 2> values;
	std::array<float, 2> tangents;

	keys[0] = static_cast<float>(m_controlNodes[0].x);
	keys[1] = static_cast<float>(m_controlNodes[3].x);
	values[0] = static_cast<float>(m_controlNodes[0].y);
	values[1] = static_cast<float>(m_controlNodes[3].y);

	constexpr float epsilon = 1.0e-6f;
	constexpr float maxTangent = 1.0f / epsilon;
	const float keyRange0 = static_cast<float>(m_controlNodes[1].x - m_controlNodes[0].x);
	const float keyRange1 = static_cast<float>(m_controlNodes[2].x - m_controlNodes[3].x);
	tangents[0] = fabsf(keyRange0) > epsilon ? static_cast<float>((m_controlNodes[1].y - m_controlNodes[0].y) / keyRange0) : maxTangent;
	tangents[1] = fabsf(keyRange1) > epsilon ? static_cast<float>((m_controlNodes[2].y - m_controlNodes[3].y) / keyRange1) : maxTangent;

	// calculate spline data
	const float keyRange = keys[1] - keys[0];
	if (fabsf(keyRange) > epsilon)
	{
		const float keyStep = keyRange / (m_numSplineSamples - 1);
		for (int i = 0; i < m_numSplineSamples; ++i)
		{
			const float key = keys[0] + i * keyStep;
			const float param = (key - keys[0]) / keyRange;
			m_keyData[i] = key;
			m_valueData[i] = CubicHermite::EvaluateSegment(values, tangents, param);
		}
	}

	// calculate turning points
	m_turningError = CubicHermite::FindSegmentTurningPoints(values, tangents, m_turningPoints);
	for (CubicHermite::TurningPoint& point : m_turningPoints)
		point.m_key = keys[0] + keyRange * point.m_key;

	// calculate crossing keys
	m_crossingError = CubicHermite::FindSegmentCrossingPoints(values, tangents, m_crossingKeys);
	for (float& key : m_crossingKeys)
		key = keys[0] + keyRange * key;
}