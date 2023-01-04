#pragma once


#include "WindowWidget.h"
#include <imgui.h>
#include <vector>



class ExponentialDecayWidget : public IWindowWidget
{
public:
	ExponentialDecayWidget(std::weak_ptr<MessageBus> pMessageBus) : IWindowWidget(pMessageBus) {}

private:
	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents() override;
	void GenerateSamples();

	std::vector<float> m_timeData;

	struct FixedExponentialDecayData
	{
		std::vector<float> m_valueData;
		float m_decayFactor = 0.926f;
		bool m_render = true;
	} m_fixedData;

	struct IndependentExponentialDecayData
	{
		std::vector<float> m_valueData;
		float m_frequency = 100.0f;
		float m_scalePerSecond = 0.01f;
		bool m_useFrequency = false;
		bool m_render = true;
	} m_independentData;

	struct NormalisedExponentialDecayData
	{
		std::vector<float> m_valueData;
		float m_damping = 1.0f;
		bool m_render = true;
	} m_normalisedData;

	float m_fps = 60.0f;
	float m_duration = 1.0f;
	bool m_isDirty = true;
};