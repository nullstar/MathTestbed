#pragma once


#include "../MessageBus.h"



class IWindowWidget : public IMessageReceiver
{
public:
	IWindowWidget(std::weak_ptr<MessageBus> pMessageBus);
	virtual ~IWindowWidget();

	void Render();
	void Focus();
	void Close();
	bool WantsToClose() const;

	bool SendMessage(const MessageType& message) const;

private:
	virtual const char* GetWindowName() const = 0;
	virtual void RenderContents() = 0;

	std::weak_ptr<MessageBus> m_pMessageBus;
	bool m_wantsFocus = true;
	bool m_isOpen = true;
};