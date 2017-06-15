#include <catch.hpp>

#include <cts-core/network/network.h>

using namespace cts;
using namespace cts::core;


TEST_CASE("Network/nodes", "Check adding/removing Nodes to a Network")
{
	Network n;
	REQUIRE(n.getNodes().size() == 0);

	auto n1 = n.addNode({ 0, 0 });
	REQUIRE(n.getNodes().size() == 1);
	REQUIRE(n.getNodes()[0].get() == n1);
	REQUIRE(n.getNodes()[0]->getPosition() == vec2(0, 0));

	auto n2 = n.addNode({ 1, 0 });
	REQUIRE(n.getNodes().size() == 2);
	REQUIRE(n.getNodes()[1].get() == n2);
	REQUIRE(n.getNodes()[1]->getPosition() == vec2(1, 0));

	auto n3 = n.addNode({ 2, 0 });
	REQUIRE(n.getNodes().size() == 3);
	REQUIRE(n.getNodes()[2].get() == n3);
	REQUIRE(n.getNodes()[2]->getPosition() == vec2(2, 0));

	{
		auto tmp = n.getNodes({ { 0, 0 }, { 1, 1 } });
		REQUIRE(tmp.size() == 2);
		REQUIRE(tmp[0] == n1);
		REQUIRE(tmp[1] == n2);
	}

	auto c = n.addConnection(*n1, *n3);
	REQUIRE(n.getConnections().size() == 1);

	n.removeNode(*n2);
	REQUIRE(n.getNodes().size() == 2);
	REQUIRE(n.getNodes()[0].get() == n1);
	REQUIRE(n.getNodes()[0]->getPosition() == vec2(0, 0));
	REQUIRE(n.getNodes()[1].get() == n3);
	REQUIRE(n.getNodes()[1]->getPosition() == vec2(2, 0));
	REQUIRE(n.getConnections().size() == 1);

	{
		auto tmp = n.getNodes({ { 0, 0 }, { 1, 1 } });
		REQUIRE(tmp.size() == 1);
		REQUIRE(tmp[0] == n1);
	}

	n.removeNode(*n1);
	REQUIRE(n.getNodes().size() == 1);
	REQUIRE(n.getNodes()[0].get() == n3);
	REQUIRE(n.getNodes()[0]->getPosition() == vec2(2, 0));
	REQUIRE(n.getConnections().size() == 0);
}


TEST_CASE("Network/connections", "Check adding/removing Connections to a Network")
{
	Network n;

	auto n1 = n.addNode({ 0, 0 });
	auto n2 = n.addNode({ 1, 0 });
	auto n3 = n.addNode({ 2, 0 });
	REQUIRE(n.getConnections().size() == 0);

	auto c1 = n.addConnection(*n1, *n2);
	REQUIRE(n.getConnections().size() == 1);
	REQUIRE(&n.getConnections()[0].get() == c1);
	REQUIRE(&c1->getStartNode() == n1);
	REQUIRE(&c1->getEndNode() == n2);
	REQUIRE(n1->getOutgoingConnections().size() == 1);
	REQUIRE(n1->getOutgoingConnections()[0] == c1);
	REQUIRE(n2->getIncomingConnections().size() == 1);
	REQUIRE(n2->getIncomingConnections()[0] == c1);

	auto c2 = n.addConnection(*n2, *n3);
	auto c3 = n.addConnection(*n1, *n3);
	REQUIRE(n.getConnections().size() == 3);
	REQUIRE(&n.getConnections()[1].get() == c2);
	REQUIRE(&n.getConnections()[2].get() == c3);
	REQUIRE(&c2->getStartNode() == n2);
	REQUIRE(&c2->getEndNode() == n3);
	REQUIRE(&c3->getStartNode() == n1);
	REQUIRE(&c3->getEndNode() == n3);
	REQUIRE(n1->getOutgoingConnections().size() == 2);
	REQUIRE(n1->getOutgoingConnections()[1] == c3);
	REQUIRE(n2->getOutgoingConnections().size() == 1);
	REQUIRE(n2->getOutgoingConnections()[0] == c2);
	REQUIRE(n3->getIncomingConnections().size() == 2);
	REQUIRE(n3->getIncomingConnections()[0] == c2);
	REQUIRE(n3->getIncomingConnections()[1] == c3);

	n.removeConnection(*c2);
	REQUIRE(n.getConnections().size() == 2);
	REQUIRE(&n.getConnections()[0].get() == c1);
	REQUIRE(&n.getConnections()[1].get() == c3);
	REQUIRE(n1->getOutgoingConnections().size() == 2);
	REQUIRE(n2->getOutgoingConnections().size() == 0);
	REQUIRE(n3->getIncomingConnections().size() == 1);
	REQUIRE(n3->getIncomingConnections()[0] == c3);
}
