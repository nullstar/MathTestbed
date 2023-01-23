#pragma once


#include "Widgets/WindowWidget.h"
#include "Splines/CubicHermite.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>



class RootFindingWidget : public IWindowWidget
{
public:
	RootFindingWidget(std::weak_ptr<MessageBus> pMessageBus);

private:
	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents(float deltaTime) override;

	void GenerateSplineData();

	static constexpr unsigned int m_numSplineSamples = 100;
	
	std::array<glm::dvec2, 4> m_controlNodes;
	std::array<float, m_numSplineSamples> m_keyData;
	std::array<float, m_numSplineSamples> m_valueData;
	std::vector<CubicHermite::TurningPoint> m_turningPoints;
	std::vector<float> m_crossingKeys;
	RootFinding::EError m_turningError = RootFinding::EError::None;
	RootFinding::EError m_crossingError = RootFinding::EError::None;
};