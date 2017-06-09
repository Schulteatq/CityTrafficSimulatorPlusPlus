#ifndef CTS_CORE_SIGNAL_H__
#define CTS_CORE_SIGNAL_H__

#include <cts-core/coreapi.h>

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace cts { namespace core
{
	struct SignalConnection;
	class SignalBase;

	/**
	 * Base class for classes that can contain slots (i.e. can act as target for signals).
	 *
	 * SignalReceiver keeps track of its connections so that it can disconnect from the signals if 
	 * the class gets deleted.
	 */
	class CTS_CORE_API SignalReceiver
	{
		// Declare Signal as friend so that it can access addConnection() and removeConnection().
		template<typename... ArgTypes>
		friend class Signal;

		template<typename... ArgTypes>
		friend struct TypedSignalConnection;

	public:
		/// Default constructor
		SignalReceiver();

		/// Copy constructor, does \e not copy any existing signal connections from \a other.
		SignalReceiver(const SignalReceiver& other);

		/// Assignment operator, disconnects all existing connections, does not copy any existing signal connections from \a other.
		SignalReceiver& operator=(const SignalReceiver& rhs);

		/// Virtual destructor disconnects from all connected signals.
		virtual ~SignalReceiver();

		/// Returns the current number of connections.
		size_t numConnections() const;

	private:
		/// Registers the given connection with this object.
		/// \note   Used by SignalConnection
		void addConnection(SignalConnection* connection);

		/// Deregisters the given connection from this object.
		/// \note   Used by SignalConnection
		void removeConnection(SignalConnection* connection);


		std::vector<SignalConnection*> m_connectedSignals;    ///< List of all incoming connections, we do not own these pointers.
		bool m_isDeleting;
	};


	// ================================================================================================


	/**
	 * Base class for Signals.
	 * Does not implement any functionality, just provides an interface.
	 */
	class CTS_CORE_API SignalBase
	{
	public:
		virtual ~SignalBase() {}
		virtual bool disconnect(SignalConnection* connection) = 0;
	};


	// ================================================================================================


	/**
	 * Structure for describing/identifying connections between signals and slots.
	 * Instances of this class are always owned by the corresponding signal.
	 */
	struct CTS_CORE_API SignalConnection
	{
		template<typename... ArgTypes>
		friend class Signal;
		friend class SignalReceiver;

		/// Typedef for a function to be called on the slot on disconnection.
		using DisconnectFunc = std::function<void(SignalConnection*)>;
		/// Typedef for a function to be called when the owning signal is cloned.
		using CloneSignalFunc = std::function<SignalConnection*(SignalBase&)>;
		/// Typedef for a function to be called when the target slot is cloned.
		using CloneSlotFunc = std::function<SignalConnection*(SignalReceiver*)>;

		/// Constructor for a connection between the given signal and slot.
		/// If the given slot is not 0, the connection will register with it.
		/// \param  signal			Pointer to the signal, must not be 0.
		/// \param  slot			Pointer to the slot, may be 0 in the case that the slot is a free function.
		/// \param  disconnectFunc	Optional function that should be used to notify the slot that the connection 
		///							is destroyed, may be 0.
		SignalConnection(SignalBase& signal, SignalReceiver* slot, DisconnectFunc disconnectFunc = nullptr, CloneSignalFunc&& cloneSignalFunc = nullptr, CloneSlotFunc&& cloneSlotFunc = nullptr);

		/// Destructor will deregister from the connected slot.
		virtual ~SignalConnection();

		/// Removes the connection between signal and slot.
		/// \note	The object on which you call this function will be deleted and no longer exist 
		///			when this function returns.
		void disconnect();

	protected:
		virtual SignalConnection* clone(SignalBase& newSignal) const;
		virtual SignalConnection* clone(SignalReceiver* newSlot);

		SignalBase* m_signal;				///< Pointer to the signal, must not be 0.
		SignalReceiver* m_slot;				///< Pointer to the slot, may be 0 in the case that the slot is a free function.
		DisconnectFunc m_disconnectFunc;	///< Optional function that should be used to notify the slot that the connection is destroyed.
		CloneSignalFunc m_cloneSignalFunc;	///< Optional function that should be used when the owning signal is cloned.
		CloneSlotFunc m_cloneSlotFunc;		///< Optional function that should be used when the target slot is cloned.
	};


	// ================================================================================================


	/**
	 * A specific signal with the given signature.
	 * 
	 * You can connect an arbitrary amount of slots (i.e. function pointers) to this signal which
	 * will be called on emitSignal(). If you connect slots to member functions of classes inheriting
	 * from SignalReceiver, Signal will track the lifetime of the object and automatically disconnect the
	 * slot if the object is deleted. Thus, in this case the order of destruction of signal and slot
	 * does not matter.
	 * 
	 * Be aware that Signal does not implement any duplicate detection. Thus, if you connect the same 
	 * slot (i.e. member function) multiple times to the same signal, that function will be called as 
	 * many times as there are connections.
	 * 
	 * Signal will duplicate existing SignalConnections when calling its copy constructor and/or its 
	 * assignment operator.
	 *
	 * \tparam  ArgTypes    Signature of the signal/function to call.
	 *
	 * \note    This class is not thread-safe. While it should be safe to concurrently emit the same 
	 *          signal, it is not safe to concurrently connect or disconnect any slots.
	 */
	template<typename... ArgTypes>
	class Signal : public SignalBase
	{
	public:
		/// Typedef for a std::function matching ths signal type.
		using FunctionType = std::function<void(ArgTypes...)>;
		/// Bundling a unique_ptr<SignalConnection> with a slot function.
		using ConnectionDescriptor = std::pair< std::unique_ptr<SignalConnection>, FunctionType >;
		/// Typedef for a vector of ConnectionDescriptors.
		using ConnectionListType = std::vector<ConnectionDescriptor>;


		/// Default constructor
		Signal() = default;

		/// Copy constructor, does \e not copy any connections from \a other.
		Signal(const Signal<ArgTypes...>& other);

		/// Assignment operator, disconnects all existing connections, does \e not copy any connections from \a other.
		Signal<ArgTypes...>& operator=(Signal<ArgTypes...> rhs);

		/// Virtual destructor, deletes all connections and thereby also disconnects from all connected slots.
		virtual ~Signal() = default;


		/// Connects the given method pointer as slot to this signal.
		/// \param  object      Pointer to the object holding the method. Must derive from SignalReceiver.
		/// \param  methodptr   Pointer to method to call on emitSignal().
		/// \return SignalConnection object for identifying the established signal-slot connection. You can use 
		///			this when calling disconnect(). The returned pointer is owned by the signal and will be invalid 
		///			as soon as the signal is destroyed.
		/// 
		/// \note   Signal will track the lifetime of \a object and automatically disconnect the
		///         slot if \a object is deleted during the lifetime of this signal.
		template<typename T>
		SignalConnection* connect(T* object, void (T::*methodptr)(ArgTypes...));

		/// Connects the given free function as slot to this signal.
		/// \param  func			Function to call on emitSignal().
		/// \param	disconnectFunc	Optional function to call when the connection is deleted.
		/// \return SignalConnection object for identifying the established signal-slot connection. You can use 
		///			this when calling disconnect(). The returned pointer is owned by the signal and will be invalid 
		///			as soon as the signal is destroyed.
		/// 
		/// \note   This overload is potentially dangerous, since there is no way for this signal 
		///         to track the lifetime of \a func. Thus, you have to ensure that func will 
		///         remain valid for the entire lifetime of this signal or use disconnect() with 
		///         the returned connection object when needed. 
		SignalConnection* connect(FunctionType func, SignalConnection::DisconnectFunc disconnectFunc = nullptr);

		/// Removes the given connection.
		/// \param  object  Pointer to the SignalConnection object returned during connect().
		/// \return True if a slot was found and deleted, false otherwise.
		bool disconnect(SignalConnection* connection) override;

		/// Disconnects all slots of the given object from this signal.
		/// \param  object  Pointer to the object holding the slot.
		/// \return The number of connections that were found and deleted.
		size_t disconnect(SignalReceiver* object);

		/// Disconnects all slots from this signal
		/// \return The number of connections that were found and deleted.
		size_t disconnectAll();


		/// Calls all connected slots with the given arguments.
		void emitSignal(ArgTypes... args) const;


		/// Returns the current number of connections.
		size_t numConnections() const;

	private:		
		using ThisType = Signal<ArgTypes...>;

		/// The list of all outgoing connections.
		ConnectionListType m_connectedSlots;
	};


	// ================================================================================================


	template<typename... ArgTypes>
	Signal<ArgTypes...>::Signal(const Signal<ArgTypes...>& other)
	{
		/// clone all connections
		for (auto& c : other.m_connectedSlots)
		{
			c.first->clone(*this);
		}
	}


	template<typename... ArgTypes>
	Signal<ArgTypes...>& Signal<ArgTypes...>::operator=(Signal<ArgTypes...> rhs)
	{
		// copy and swap paradigm as described by Scott Meyers.
		std::swap(m_connectedSlots, rhs.m_connectedSlots);
		return *this;
	}
	

	template<typename... ArgTypes>
	template<typename T>
	SignalConnection* Signal<ArgTypes...>::connect(T* object, void (T::*methodptr)(ArgTypes...))
	{
		static_assert(std::is_base_of<SignalReceiver, T>::value, "The class of the connected member function must derive from SignalReceiver!");

		auto connection = std::make_unique<SignalConnection>(*this, object);
		connection->m_disconnectFunc = [object](SignalConnection* c) { object->removeConnection(c); };
		connection->m_cloneSignalFunc = [object, methodptr](SignalBase& newSignal) { assert(dynamic_cast<ThisType*>(&newSignal)); return static_cast<ThisType&>(newSignal).connect(object, methodptr); };
		connection->m_cloneSlotFunc = [this, methodptr](SignalReceiver* newReceiver) { /* cannot assert type since object is not yet fully constructed... */ return this->connect(static_cast<T*>(newReceiver), methodptr); };

		auto toReturn = connection.get();
		m_connectedSlots.emplace_back(std::move(connection), [object, methodptr](ArgTypes... args) { return (object->*methodptr)(std::forward<ArgTypes>(args)...); });
		object->addConnection(toReturn);
		
		return toReturn;
	}


	template<typename... ArgTypes>
	SignalConnection* Signal<ArgTypes...>::connect(FunctionType func, SignalConnection::DisconnectFunc disconnectFunction /*= nullptr*/)
	{
		auto connection = new SignalConnection(*this, nullptr, disconnectFunction);
		connection->m_cloneSignalFunc = [func, disconnectFunction](SignalBase& newSignal) { assert(dynamic_cast<ThisType*>(&newSignal)); return static_cast<ThisType&>(newSignal).connect(func, disconnectFunction); };
		m_connectedSlots.emplace_back(std::unique_ptr<SignalConnection>(connection), std::move(func));
		return connection;
	}


	template<typename... ArgTypes>
	void Signal<ArgTypes...>::emitSignal(ArgTypes... args) const
	{
		for (size_t i = 0; i < m_connectedSlots.size(); ++i)
			m_connectedSlots[i].second(std::forward<ArgTypes>(args)...);
	}


	template<typename... ArgTypes>
	bool Signal<ArgTypes...>::disconnect(SignalConnection* connection)
	{
		auto it = std::find_if(m_connectedSlots.begin(), m_connectedSlots.end(), [connection](const ConnectionDescriptor& cd) { return cd.first.get() == connection; });
		if (it == m_connectedSlots.end())
			return false;

		m_connectedSlots.erase(it);
		return true;
	}


	template<typename... ArgTypes>
	size_t Signal<ArgTypes...>::disconnect(SignalReceiver* object)
	{
		if (m_connectedSlots.empty())
			return 0;

		// classic remove-erase pattern
		auto rangeStart = std::remove_if(m_connectedSlots.begin(), m_connectedSlots.end(), [&object](const ConnectionDescriptor& cd) { return cd.first->m_slot == object; });
		if (rangeStart == m_connectedSlots.end())
			return 0;

		size_t numElem = std::distance(rangeStart, m_connectedSlots.end());
		m_connectedSlots.erase(rangeStart, m_connectedSlots.end());
		return numElem;
	}


	template<typename... ArgTypes>
	size_t Signal<ArgTypes...>::disconnectAll()
	{
		size_t numElem = m_connectedSlots.size();
		m_connectedSlots.clear();
		return numElem;
	}


	template<typename... ArgTypes>
	size_t Signal<ArgTypes...>::numConnections() const
	{
		return m_connectedSlots.size();
	}

}
}

#endif
