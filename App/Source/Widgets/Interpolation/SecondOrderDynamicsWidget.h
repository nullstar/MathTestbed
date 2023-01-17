#pragma once


#include "Interpolation/SecondOrderDynamics.h"
#include "Widgets/WindowWidget.h"
#include <imgui.h>
#include <vector>



class SecondOrderDynamicsWidget : public IWindowWidget
{
public:
	SecondOrderDynamicsWidget(std::weak_ptr<MessageBus> pMessageBus);

private:
	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents(float deltaTime) override;
	
	void ResetData();
	void GenerateStaticData();
	void TakeRealtimeSample(float time);
	void UpdateRealtimeData(float deltaTime);

	SecondOrderDynamics<float> m_dynamics;

	std::vector<float> m_timeData;
	std::vector<float> m_targetData;
	std::vector<float> m_valueData;
	std::vector<float> m_valueSpeedData;

	float m_target = 1.0f;
	float m_duration = 10.0f;
	float m_fps = 60.0f;

	float m_frequency = 1.0f;
	float m_damping = 1.0f;
	float m_initialResponse = 0.0f;
	float m_maxSpeed = 1.0f;

	float m_prevSampleTime = 0.0f;
	float m_currentTime = 0.0f;

	bool m_clampSpeed = false;
	bool m_renderSpeed = false;
	bool m_isRealtime = false;
};