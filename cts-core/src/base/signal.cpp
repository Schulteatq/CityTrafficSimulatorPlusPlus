#include <cts-core/base/signal.h>
#include <cts-core/base/utils.h>

#include <cassert>

namespace cts { namespace core
{

	SignalReceiver::SignalReceiver()
		: m_isDeleting(false)
	{
	}


	SignalReceiver::SignalReceiver(const SignalReceiver& other) 
		: m_isDeleting(false)
	{
	}
	

	SignalReceiver& SignalReceiver::operator=(SignalReceiver rhs)
	{
		// copy and swap paradigm as described by Scott Meyers.
		std::swap(m_connectedSignals, rhs.m_connectedSignals);
		std::swap(m_isDeleting, rhs.m_isDeleting);
		return *this;
	}


	SignalReceiver::~SignalReceiver()
	{
		m_isDeleting = true;
		for (auto& c : m_connectedSignals)
			c->m_signal->disconnect(c);
	}


	void SignalReceiver::addConnection(SignalConnection* connection)
	{
		m_connectedSignals.push_back(connection);
	}


	void SignalReceiver::removeConnection(SignalConnection* connection)
	{
		if (!m_isDeleting)
		{
			utils::remove_erase(m_connectedSignals, connection);
		}
	}


	// ================================================================================================


	SignalConnection::SignalConnection(SignalBase* signal, SignalReceiver* slot)
		: m_signal(signal)
		, m_slot(slot)
	{
		if (m_slot != nullptr)
			m_slot->addConnection(this);
	}


	SignalConnection::~SignalConnection()
	{
		if (m_slot != nullptr)
			m_slot->removeConnection(this);
	}

}
}
