#include <catch.hpp>

#include <cts-core/network/bezierparameterization.h>

using namespace cts;
using namespace cts::core;


TEST_CASE("BezierParameterization/circle", "Check BezierParameterization using a circle")
{
	// https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%c3%a9zier-curves#27863181
	const double offset = 4.0 * (sqrt(2.0) - 1.0) / 3.0;
	BezierParameterization bp(vec2(0, 0), vec2(offset, 0), vec2(1, 1 - offset), vec2(1, 1));
	REQUIRE(bp.getSupportPoints()[0] == vec2(0, 0));
	REQUIRE(bp.getSupportPoints()[1] == vec2(offset, 0));
	REQUIRE(bp.getSupportPoints()[2] == vec2(1, 1-offset));
	REQUIRE(bp.getSupportPoints()[3] == vec2(1, 1));
	REQUIRE(bp.getBounds() == Bounds2({ vec2(0.0, 0.0), vec2(1.0, 1.0) }));
	REQUIRE(bp.getArcLength() == Approx(1.5707963267948966192313216916398).epsilon(0.0001));

	auto half1 = bp.getSubdividedFirst();
	REQUIRE(half1.getArcLength() == Approx(0.78539816339744830961566084581988).epsilon(0.0001));

	auto half2 = bp.getSubdividedSecond();
	REQUIRE(half2.getArcLength() == Approx(0.78539816339744830961566084581988).epsilon(0.0001));
}


TEST_CASE("BezierParameterization/line", "Check BezierParameterization using a straight line")
{
	BezierParameterization bp(vec2(0, 0), vec2(1, 0), vec2(9, 0), vec2(10, 0));
	REQUIRE(bp.getSupportPoints()[0] == vec2(0, 0));
	REQUIRE(bp.getSupportPoints()[1] == vec2(1, 0));
	REQUIRE(bp.getSupportPoints()[2] == vec2(9, 0));
	REQUIRE(bp.getSupportPoints()[3] == vec2(10, 0));
	REQUIRE(bp.getBounds() == Bounds2({ vec2(0.0, 0.0), vec2(10.0, 0.0) }));
	REQUIRE(bp.getArcLength() == Approx(10));

	REQUIRE(bp.timeToArcPosition(0.0) == 0.0);
	REQUIRE(bp.timeToArcPosition(0.1) == Approx(0.4975395));
	REQUIRE(bp.timeToArcPosition(0.5) == 5.0);
	REQUIRE(bp.timeToArcPosition(1.0) == 10.0);

	REQUIRE(bp.arcPositionToTime(0.0) == 0.0);
	REQUIRE(bp.arcPositionToTime(0.4975395) == Approx(0.1));
	REQUIRE(bp.arcPositionToTime(5.0) == Approx(0.5));
	REQUIRE(bp.arcPositionToTime(10.0) == 1.0);

	for (double i = 0; i <= 10; i+=1.0)
	{
		REQUIRE(bp.arcPositionToCoordinate(i)[0] == Approx(i).epsilon(0.001));
		REQUIRE(bp.arcPositionToCoordinate(i)[1] == 0.0);
	}
	
	auto half1 = bp.getSubdividedFirst();
	REQUIRE(half1.getSupportPoints()[0] == vec2(0, 0));
	REQUIRE(half1.getSupportPoints()[1] == vec2(0.5, 0));
	REQUIRE(half1.getSupportPoints()[2] == vec2(2.75, 0));
	REQUIRE(half1.getSupportPoints()[3] == vec2(5, 0));
	REQUIRE(half1.getBounds() == Bounds2({ vec2(0.0, 0.0), vec2(5.0, 0.0) }));
	REQUIRE(half1.getArcLength() == Approx(5));

	auto half2 = bp.getSubdividedSecond();
	REQUIRE(half2.getSupportPoints()[0] == vec2(5, 0));
	REQUIRE(half2.getSupportPoints()[1] == vec2(7.25, 0));
	REQUIRE(half2.getSupportPoints()[2] == vec2(9.5, 0));
	REQUIRE(half2.getSupportPoints()[3] == vec2(10, 0));
	REQUIRE(half2.getBounds() == Bounds2({ vec2(10.0, 0.0), vec2(5.0, 0.0) }));
	REQUIRE(half2.getArcLength() == Approx(5));
}
