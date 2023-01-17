#pragma once


#include "Widgets/WindowWidget.h"
#include "Interpolation/ExponentialDecay.h"
#include <imgui.h>
#include <vector>



class ExponentialDecayWidget : public IWindowWidget
{
public:
	ExponentialDecayWidget(std::weak_ptr<MessageBus> pMessageBus);

private:
	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents(float deltaTime) override;

	void ResetData();
	void GenerateStaticData();
	void TakeRealtimeSample(float time);
	void UpdateRealtimeData(float deltaTime);

	std::vector<float> m_timeData;
	std::vector<float> m_targetData;

	struct FixedExponentialDecayData
	{
		FixedExponentialDecay<float> m_value;
		std::vector<float> m_valueData;
		float m_decayFactor = 0.926f;
		bool m_render = true;
	} m_fixedData;

	struct IndependentExponentialDecayData
	{
		IndependentExponentialDecay<float> m_value;
		std::vector<float> m_valueData;
		float m_frequency = 100.0f;
		float m_scalePerSecond = 0.01f;
		bool m_useFrequency = false;
		bool m_render = true;
	} m_independentData;

	struct NormalisedExponentialDecayData
	{
		NormalisedExponentialDecay<float> m_value;
		std::vector<float> m_valueData;
		float m_damping = 1.0f;
		bool m_render = true;
	} m_normalisedData;

	float m_target = 0.0f;
	float m_duration = 10.0f;
	float m_fps = 60.0f;
	float m_prevSampleTime = 0.0f;
	float m_currentTime = 0.0f;
	bool m_isRealtime = false;
};