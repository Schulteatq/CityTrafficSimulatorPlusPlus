#include <catch.hpp>

#include <cts-core/base/bounds.h>
#include <cts-core/base/math.h>

using namespace cts;
using namespace cts::core;


TEST_CASE("bounds/ctors", "Check constructors")
{
	Bounds2 b1;
	REQUIRE(std::isnan(b1.getLlf()[0]));
	REQUIRE(std::isnan(b1.getLlf()[1]));
	REQUIRE(std::isnan(b1.getUrb()[0]));
	REQUIRE(std::isnan(b1.getUrb()[1]));

	Bounds2 b2(vec2(1.0, 2.0));
	REQUIRE(b2.getLlf() == vec2(1.0, 2.0));
	REQUIRE(b2.getUrb() == vec2(1.0, 2.0));

	Bounds2 b3{ vec2(-1.0, 2.0), vec2(0.0, -1.0), vec2(10.0, -20.0) };
	REQUIRE(b3.getLlf() == vec2(-1.0, -20.0));
	REQUIRE(b3.getUrb() == vec2(10.0, 2.0));

	std::array<vec2, 3> a{ vec2(-1.0, 2.0), vec2(0.0, -1.0), vec2(10.0, -20.0) };
	Bounds2 b4(a);
	REQUIRE(b4.getLlf() == vec2(-1.0, -20.0));
	REQUIRE(b4.getUrb() == vec2(10.0, 2.0));
}


TEST_CASE("bounds/funcs", "Check member functions")
{
	Bounds2 b1;
	REQUIRE(std::isnan(b1.center()[0]));
	REQUIRE(std::isnan(b1.center()[1]));
	REQUIRE(std::isnan(b1.volume()));
	REQUIRE(b1.contains(vec2(0.0, 0.0)) == false);
	REQUIRE(b1.contains(vec2::Constant(std::numeric_limits<double>::quiet_NaN())) == false);

	Bounds2 b2{ vec2(-1.0, 2.0), vec2(10.0, -1.0) };
	REQUIRE(b2.center() == vec2(4.5, 0.5));
	REQUIRE(b2.volume() == 33.0);
	REQUIRE(b2.contains(vec2(0.0, 0.0)) == true);
	REQUIRE(b2.contains(vec2(-2.0, 0.0)) == false);
	REQUIRE(b2.contains(vec2(20.0, 0.0)) == false);
	REQUIRE(b2.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b2.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b2.contains(vec2::Constant(std::numeric_limits<double>::quiet_NaN())) == false);
	REQUIRE(b2.intersects(b1) == false);
	REQUIRE(b1.intersects(b2) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(2, 0) }) == true);
	REQUIRE(b2.intersects(Bounds2{ vec2(0, 2) }) == true);
	REQUIRE(b2.intersects(Bounds2{ vec2(-2, -2) }) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(-2, 0) }) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(0, 4) }) == false);

	Bounds2 b3 = b2;
	REQUIRE(b3.center() == vec2(4.5, 0.5));
	REQUIRE(b3.volume() == 33.0);
	REQUIRE(b3.contains(vec2(0.0, 0.0)) == true);
	REQUIRE(b3.contains(vec2(-2.0, 0.0)) == false);
	REQUIRE(b3.contains(vec2(20.0, 0.0)) == false);
	REQUIRE(b3.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b3.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b3.contains(vec2::Constant(std::numeric_limits<double>::quiet_NaN())) == false);

	b2.addPoint(vec2(-3.0, 12.0));
	REQUIRE(b2.center() == vec2(3.5, 5.5));
	REQUIRE(b2.volume() == 169.0);
	REQUIRE(b2.contains(vec2(0.0, 0.0)) == true);
	REQUIRE(b2.contains(vec2(-2.0, 0.0)) == true);
	REQUIRE(b2.contains(vec2(-4.0, 0.0)) == false);
	REQUIRE(b2.contains(vec2(10.0, 0.0)) == true);
	REQUIRE(b2.contains(vec2(20.0, 0.0)) == false);
	REQUIRE(b2.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b2.contains(vec2(0.0, -3.0)) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(-2, 0) }) == true);
	REQUIRE(b2.intersects(Bounds2{ vec2(0, 12) }) == true);
	REQUIRE(b2.intersects(Bounds2{ vec2(-4, -4) }) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(-4, 0) }) == false);
	REQUIRE(b2.intersects(Bounds2{ vec2(0, 14) }) == false);
}
