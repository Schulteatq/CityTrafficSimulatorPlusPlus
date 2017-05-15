#include <cts-lua/cts-lua.h>

#include <cts-core/base/log.h>
#include <cts-core/base/signal.h>
#include <cts-core/base/utils.h>
#include <cts-core/network/network.h>
#include <cts-core/simulation/simulation.h>
#include <cts-core/traffic/trafficmanager.h>


namespace cts
{
	class LuaSignalConnectionBase : public utils::NotCopyable
	{
	public:
		virtual ~LuaSignalConnectionBase() = default;
	};


	template<typename... ArgTypes>
	class LuaSignalConnection : public LuaSignalConnectionBase
	{
	public:
		LuaSignalConnection(core::Signal<ArgTypes...>& signal, sol::protected_function luaFunction)
			: m_slot(std::move(luaFunction))
			, m_connection(nullptr)
		{
			m_connection = signal.connect(
				[this](ArgTypes... args) {
					m_slot(std::forward<ArgTypes>(args)...);
				},
				[this](core::SignalConnection* c) {
					assert(c == m_connection);
					m_slot = sol::protected_function();
					m_connection = nullptr;
				}
			);
		}

		~LuaSignalConnection()
		{
			if (m_connection)
			{
				m_connection->m_disconnectFunc = nullptr;
				m_connection->disconnect();
			}
		}


	private:
		sol::protected_function m_slot;
		core::SignalConnection* m_connection;
	};


	class LuaSignalReceiver : public core::SignalReceiver, public utils::NotCopyable
	{
	public:
		LuaSignalReceiver() = default;
	
		~LuaSignalReceiver()
		{
			disconnectAll();
		}
	

		void disconnectAll()
		{
			m_connections.clear();
		}
	

		template<typename... ArgTypes>
		void connectTo(core::Signal<ArgTypes...>& signal, sol::function luaFunction)
		{
			m_connections.push_back(std::make_unique< LuaSignalConnection<ArgTypes...> >(signal, luaFunction));
		}
	
	
	private:
		std::vector< std::unique_ptr<LuaSignalConnectionBase> > m_connections;
	};


	template<typename... ArgTypes>
	void registerSignalType(sol::state& luaState, const std::string& typeName)
	{
		luaState.new_usertype< core::Signal<ArgTypes...> >(typeName
			, "new", sol::no_constructor
			, "connect", [](sol::userdata signal, sol::function function) -> std::unique_ptr< LuaSignalConnection<ArgTypes...> >
				{
					if (signal.valid() && signal.is< core::Signal<ArgTypes...> >())
					{
						return std::make_unique< LuaSignalConnection<ArgTypes...> >(signal.as< core::Signal<ArgTypes...> >(), function);
					}
					return nullptr;
				}
			, "connectTo", [](sol::userdata signal, sol::userdata receiver, sol::function function) {
				if (signal.valid() && signal.is< core::Signal<ArgTypes...> >())
				{
					if (receiver.valid() && receiver.is<LuaSignalReceiver>())
						receiver.as<LuaSignalReceiver>().connectTo(signal.as< core::Signal<ArgTypes...> >(), function);
				}
			}
		);
	}


	void lua::Registration::registerWith(sol::state& luaState)
	{
		sol::table ctsNamespace = luaState.create_table("cts");

		ctsNamespace.new_usertype<LuaSignalReceiver>("LuaSignalReceiver"
			, sol::constructors<LuaSignalReceiver>()

			, "disconnectAll", &LuaSignalReceiver::disconnectAll
		);

		ctsNamespace.new_simple_usertype<core::Node>("Node"
			// ctors
			, sol::constructors<core::Node(vec2)>()

			// functions
			, "position", sol::property(&core::Node::getPosition, &core::Node::setPosition)
			, "getInSlope", &core::Node::getInSlope
			, "setInSlope", &core::Node::setInSlope
			, "getOutSlope", &core::Node::getOutSlope
			, "setOutSlope", &core::Node::setOutSlope
		);

		
		ctsNamespace.new_usertype<core::Network>("Network"
			// ctors
			, sol::constructors<core::Network()>()

			// functions
			, "importLegacyXml", &core::Network::importLegacyXml
			, "getConnections", &core::Network::getConnections
			, "getTrafficManager", &core::Network::getTrafficManager
		);


		registerSignalType<>(luaState, "Signal<>");

		ctsNamespace.new_usertype<core::Simulation>("Simulation"
			// ctors
			//, sol::constructors<core::Simulation(core::Network&)>()

			// functions
			, "getCurrentTime", &core::Simulation::getCurrentTime
			, "getSpeed", &core::Simulation::getSpeed
			, "setSpeed", &core::Simulation::setSpeed
			, "getTicksPerSecond", &core::Simulation::getTicksPerSecond
			, "setTicksPerSecond", &core::Simulation::setTicksPerSecond
			, "reset", &core::Simulation::reset
			, "step", &core::Simulation::step
			, "start", &core::Simulation::start
			, "stop", &core::Simulation::stop

			// members
			, "s_stepped", &core::Simulation::s_stepped
		);


		ctsNamespace.new_usertype<core::TrafficManager>("TrafficManager"
			// ctors
			//, sol::constructors<core::TrafficManager()>()

			// functions
			, "globalTrafficMultiplier", sol::property(&core::TrafficManager::getGlobalTrafficMultiplier, &core::TrafficManager::setGlobalTrafficMultiplier)
			);

	}

}