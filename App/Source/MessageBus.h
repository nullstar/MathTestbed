#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>




typedef std::string MessageType;




class IMessageReceiver
{
private:
	virtual void OnMessage(const MessageType& message) = 0;
	friend class MessageBus;
};




class MessageBus
{
public:
	MessageBus(); 

	void AddReceiver(const std::shared_ptr<IMessageReceiver>& pReceiver);
	void AddMessage(const MessageType& message);
	void BroadcastMessages();
	
private:
	typedef std::vector<MessageType> MessageArray;
	typedef std::shared_ptr<MessageArray> MessageArrayPtr;
	MessageArrayPtr m_pMessageInputBuffer = nullptr;
	MessageArrayPtr m_pMessageOutputBuffer = nullptr;

	typedef std::vector<std::weak_ptr<IMessageReceiver>> MessageReceiverArray;
	MessageReceiverArray m_receivers;
	MessageReceiverArray m_receiverInputBuffer;
};