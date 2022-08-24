#pragma once


#include "WindowWidget.h"
#include <imgui.h>
#include <vector>



class SmoothValueWidget : public IWindowWidget
{
private:
	virtual const char* GetWindowName() const override;
	virtual void RenderContents() override;
	void GenerateSamples();

	std::vector<float> m_timeData;
	std::vector<float> m_targetData;
	std::vector<float> m_valueData;

	bool m_useFrequency = false;
	float m_smoothTime = 1.0f;
	float m_frequency = 1.0f;

	bool m_isDirty = true;
};