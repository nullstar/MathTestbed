#include "MessageBus.h"



MessageBus::MessageBus()
{
	m_pMessageInputBuffer = std::make_unique<MessageArray>();
	m_pMessageOutputBuffer = std::make_unique<MessageArray>();
}


void MessageBus::AddReceiver(const std::shared_ptr<IMessageReceiver>& pReceiver)
{
	m_receiverInputBuffer.push_back(pReceiver);
}


void MessageBus::AddMessage(const MessageType& message)
{
	m_pMessageInputBuffer->push_back(message);
}


void MessageBus::BroadcastMessages()
{
	// add new receivers
	m_receivers.insert(m_receivers.end(), m_receiverInputBuffer.begin(), m_receiverInputBuffer.end());
	m_receiverInputBuffer.clear();

	// swap message buffer so we can continue receiving messages
	std::swap(m_pMessageInputBuffer, m_pMessageOutputBuffer);

	// broadcast messages to all valid receivers and remove invlid receivers
	if (!m_pMessageOutputBuffer->empty())
	{
		for (MessageReceiverArray::iterator receiverIter = m_receivers.begin(); receiverIter != m_receivers.end();)
		{
			std::shared_ptr<IMessageReceiver> pReceiver = receiverIter->lock();
			if (!pReceiver)
			{
				receiverIter = m_receivers.erase(receiverIter);
				continue;
			}

			for (const std::string& message : *m_pMessageOutputBuffer)
				pReceiver->OnMessage(message);

			++receiverIter;
		}

		m_pMessageOutputBuffer->clear();
	}
}