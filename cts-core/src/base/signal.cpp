#include <cts-core/base/signal.h>
#include <cts-core/base/utils.h>

#include <cassert>

namespace cts { namespace core
{

	SignalReceiver::SignalReceiver()
	{
	}


	SignalReceiver::SignalReceiver(const SignalReceiver& /*other*/) 
	{
	}
	

	SignalReceiver& SignalReceiver::operator=(SignalReceiver rhs)
	{
		// copy and swap paradigm as described by Scott Meyers.
		std::swap(m_connectedSignals, rhs.m_connectedSignals);
		return *this;
	}


	SignalReceiver::~SignalReceiver()
	{
		for (auto& c : m_connectedSignals)
		{
			c->m_disconnectFunc = nullptr;
			c->disconnect();
		}
	}


	void SignalReceiver::addConnection(SignalConnection* connection)
	{
		m_connectedSignals.push_back(connection);
	}


	void SignalReceiver::removeConnection(SignalConnection* connection)
	{
		utils::remove_erase(m_connectedSignals, connection);
	}


	// ================================================================================================


	SignalConnection::SignalConnection(SignalBase& signal, SignalReceiver* slot, DisconnectFunc&& slotDisconnecter)
		: m_signal(&signal)
		, m_slot(slot)
		, m_disconnectFunc(std::move(slotDisconnecter))
	{
	}


	SignalConnection::~SignalConnection()
	{
		if (m_disconnectFunc)
			m_disconnectFunc(this);
	}


	void SignalConnection::disconnect()
	{
		m_signal->disconnect(this);
	}


}
}
