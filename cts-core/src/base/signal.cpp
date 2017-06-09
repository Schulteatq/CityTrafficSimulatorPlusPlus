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
		for (auto& c : other.m_connectedSignals)
		{
			c->clone(this);
		}
	}
	

	SignalReceiver& SignalReceiver::operator=(const SignalReceiver& rhs)
	{
		if (&rhs == this)
			return *this;

		m_isDeleting = true;
		for (auto& c : m_connectedSignals)
		{
			c->disconnect();
		}
		m_isDeleting = false;

		for (auto& c : rhs.m_connectedSignals)
		{
			c->clone(this);
		}
		return *this;
	}


	SignalReceiver::~SignalReceiver()
	{
		m_isDeleting = true;
		for (auto& c : m_connectedSignals)
		{
			c->disconnect();
		}
	}


	size_t SignalReceiver::numConnections() const
	{
		return m_connectedSignals.size();
	}


	void SignalReceiver::addConnection(SignalConnection* connection)
	{
		m_connectedSignals.push_back(connection);
	}


	void SignalReceiver::removeConnection(SignalConnection* connection)
	{
		if (!m_isDeleting)
			utils::remove_erase(m_connectedSignals, connection);
	}


	// ================================================================================================


	SignalConnection::SignalConnection(SignalBase& signal, SignalReceiver* slot, DisconnectFunc slotDisconnecter, CloneSignalFunc&& cloneSignalFunc, CloneSlotFunc&& cloneSlotFunc)
		: m_signal(&signal)
		, m_slot(slot)
		, m_disconnectFunc(std::move(slotDisconnecter))
		, m_cloneSignalFunc(std::move(cloneSignalFunc))
		, m_cloneSlotFunc(std::move(cloneSlotFunc))
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


	SignalConnection* SignalConnection::clone(SignalBase& newSignal) const
	{
		if (m_cloneSignalFunc)
			return m_cloneSignalFunc(newSignal);
		return nullptr;
	}


	SignalConnection* SignalConnection::clone(SignalReceiver* newSlot)
	{
		if (m_cloneSlotFunc)
			return m_cloneSlotFunc(newSlot);
		return nullptr;
	}


}
}
