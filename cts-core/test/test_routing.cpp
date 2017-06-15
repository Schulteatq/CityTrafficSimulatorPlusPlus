#include <catch.hpp>

#include <cts-core/network/connection.h>
#include <cts-core/network/network.h>
#include <cts-core/network/node.h>
#include <cts-core/network/routing.h>
#include <cts-core/traffic/vehicle.h>

using namespace cts;
using namespace cts::core;


TEST_CASE("routing/basic", "Test basic pathfinding in Routing")
{
	// Setup is as follows:
	// S1 --- M1 --- M2 --- E1
	//            \
	// S2 --- M3 --- M4 --- E2
	// 
	// Thus, the valid routes are: S1 -> E1, S1 -> E2, S2 -> E2
	Network n;
	Node s1({ 0, 0 }), m1({ 1, 0 }), m2({ 2, 0 }), e1({ 3, 0 });
	Node s2({ 0, 1 }), m3({ 1, 1 }), m4({ 2, 1 }), e2({ 3, 1 });

	Connection* c1 = n.addConnection(s1, m1);
	Connection* c2 = n.addConnection(m1, m2);
	Connection* c3 = n.addConnection(m2, e1);
	Connection* c4 = n.addConnection(m1, m4);
	Connection* c5 = n.addConnection(s2, m3);
	Connection* c6 = n.addConnection(m3, m4);
	Connection* c7 = n.addConnection(m4, e2);

	TypedVehicle<IdmMobil> v1(s1, { &e1 }, 10);
	TypedVehicle<IdmMobil> v2(s1, { &e2 }, 10);
	TypedVehicle<IdmMobil> v3(s2, { &e1 }, 10);
	TypedVehicle<IdmMobil> v4(s2, { &e2 }, 10);

	Routing r;
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[0].start == &s1);
		REQUIRE(segments[0].destination == &m1);
		REQUIRE(segments[1].connection == c2);
		REQUIRE(segments[1].start == &m1);
		REQUIRE(segments[1].destination == &m2);
		REQUIRE(segments[2].connection == c3);
		REQUIRE(segments[2].start == &m2);
		REQUIRE(segments[2].destination == &e1);
	}
	{
		r.compute(s1, { &e2 }, v2);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[0].start == &s1);
		REQUIRE(segments[0].destination == &m1);
		REQUIRE(segments[1].connection == c4);
		REQUIRE(segments[1].start == &m1);
		REQUIRE(segments[1].destination == &m4);
		REQUIRE(segments[2].connection == c7);
		REQUIRE(segments[2].start == &m4);
		REQUIRE(segments[2].destination == &e2);
	}
	{
		r.compute(s2, { &e2 }, v2);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c5);
		REQUIRE(segments[0].start == &s2);
		REQUIRE(segments[0].destination == &m3);
		REQUIRE(segments[1].connection == c6);
		REQUIRE(segments[1].start == &m3);
		REQUIRE(segments[1].destination == &m4);
		REQUIRE(segments[2].connection == c7);
		REQUIRE(segments[2].start == &m4);
		REQUIRE(segments[2].destination == &e2);
	}
	{
		r.compute(s2, { &e1 }, v2);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 0);
	}
}


TEST_CASE("routing/shortest", "Test Routing finding the path with the shortest distance")
{
	// Setup is as follows:
	// S1 --- M1 --- M2 --- M3 --- M4 --- E1
	//            \                    /
	//              ---- M5 ----------
	// 
	// where the path through M5 is longer
	Network n;
	Node s1({ 0, 0 }), m1({ 1, 0 }), m2({ 2, 0 }), m3({ 3, 0 }), m4({ 4, 0 }), e1({ 5, 0 });
	Node m5({ 2.5, 1 });

	Connection* c1 = n.addConnection(s1, m1);
	Connection* c2 = n.addConnection(m1, m2);
	Connection* c3 = n.addConnection(m2, m3);
	Connection* c4 = n.addConnection(m3, m4);
	Connection* c5 = n.addConnection(m4, e1);
	Connection* c6 = n.addConnection(m1, m5);
	Connection* c7 = n.addConnection(m5, e1);

	TypedVehicle<IdmMobil> v1(s1, { &e1 }, 10);

	Routing r;
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 5);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[0].start == &s1);
		REQUIRE(segments[0].destination == &m1);
		REQUIRE(segments[1].connection == c2);
		REQUIRE(segments[1].start == &m1);
		REQUIRE(segments[1].destination == &m2);
		REQUIRE(segments[2].connection == c3);
		REQUIRE(segments[2].start == &m2);
		REQUIRE(segments[2].destination == &m3);
		REQUIRE(segments[3].connection == c4);
		REQUIRE(segments[3].start == &m3);
		REQUIRE(segments[3].destination == &m4);
		REQUIRE(segments[4].connection == c5);
		REQUIRE(segments[4].start == &m4);
		REQUIRE(segments[4].destination == &e1);
	}

	// now move M2 so that the path through M5 is shorter
	m2.setPosition({ 2, -1 });
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[0].start == &s1);
		REQUIRE(segments[0].destination == &m1);
		REQUIRE(segments[1].connection == c6);
		REQUIRE(segments[1].start == &m1);
		REQUIRE(segments[1].destination == &m5);
		REQUIRE(segments[2].connection == c7);
		REQUIRE(segments[2].start == &m5);
		REQUIRE(segments[2].destination == &e1);
	}
}


TEST_CASE("routing/fastest", "Test Routing finding the path with the fastest target velocity")
{
	// Setup is as follows:
	// S1 --- M1 --- M2 --- E1
	//            \      /
	//             - M3 -
	// 
	// where the path through M3 is longer but faster
	Network n;
	Node s1({ 0, 0 }), m1({ 1, 0 }), m2({ 2, 0 }), e1({ 3, 0 });
	Node m3({ 2, 1 });

	Connection* c1 = n.addConnection(s1, m1);
	Connection* c2 = n.addConnection(m1, m2);
	Connection* c3 = n.addConnection(m2, e1);
	Connection* c4 = n.addConnection(m1, m3);
	Connection* c5 = n.addConnection(m3, e1);
	c1->setTargetVelocity(8);
	c2->setTargetVelocity(8);
	c3->setTargetVelocity(20);
	c4->setTargetVelocity(20);
	c5->setTargetVelocity(20);

	TypedVehicle<IdmMobil> v1(s1, { &e1 }, 20);

	Routing r;
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[1].connection == c4);
		REQUIRE(segments[2].connection == c5);
	}

	// now update the velocity of c1 and c2 so that the direct path is faster again.
	c2->setTargetVelocity(20);
	c3->setTargetVelocity(20);
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[1].connection == c2);
		REQUIRE(segments[2].connection == c3);
	}

	// now update the velocity of c3 and c4 to 30 so that the bottom path is faster again. 
	// However, since the vehicle's max velocity is 20, it still choses the top one
	c4->setTargetVelocity(30);
	c5->setTargetVelocity(30);
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[1].connection == c2);
		REQUIRE(segments[2].connection == c3);
	}
}


TEST_CASE("routing/crowded", "Test Routing finding the path with the least crowded path")
{
	// Setup is as follows:
	// S1 --- M1 --- M2 --- E1
	//            \      /
	//             - M3 -
	// 
	// where the path through M3 is longer but less crowded
	Network n;
	Node s1({ 0, 0 }), m1({ 1, 0 }), m2({ 2, 0 }), e1({ 3, 0 });
	Node m3({ 2, 1 });

	Connection* c1 = n.addConnection(s1, m1);
	Connection* c2 = n.addConnection(m1, m2);
	Connection* c3 = n.addConnection(m2, e1);
	Connection* c4 = n.addConnection(m1, m3);
	Connection* c5 = n.addConnection(m3, e1);

	TypedVehicle<IdmMobil> v1(s1, { &e1 }, 20);
	TypedVehicle<IdmMobil> v2(m2, { &e1 }, 20);

	Routing r;
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[1].connection == c4);
		REQUIRE(segments[2].connection == c5);
	}

	// add a vehicoe to c5, so that the top path is less crowded/faster
	TypedVehicle<IdmMobil> v3(m3, { &e1 }, 20);
	{
		r.compute(s1, { &e1 }, v1);
		auto& segments = r.getSegments();
		REQUIRE(segments.size() == 3);
		REQUIRE(segments[0].connection == c1);
		REQUIRE(segments[1].connection == c2);
		REQUIRE(segments[2].connection == c3);
	}

}
