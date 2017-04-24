#include <cts-lua/cts-lua.h>

#include <cts-core/network/network.h>
#include <cts-core/simulation/simulation.h>


namespace cts
{

	void lua::Registration::registerWith(sol::state& luaState)
	{
		sol::table ctsNamespace = luaState.create_table("cts");

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
		);


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
		);


	}

}