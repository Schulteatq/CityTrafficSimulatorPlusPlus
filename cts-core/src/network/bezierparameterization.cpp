#include <cts-core/network/bezierparameterization.h>

#include <algorithm>
#include <cassert>

namespace cts
{
namespace core
{

	BezierParameterization::BezierParameterization(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
		: m_supportPoints({ { p0, p1, p2, p3 } })
		, m_bounds(m_supportPoints)
	{
		computeLengthApproximationTable();
	}


	BezierParameterization::BezierParameterization(const BezierParameterization& rhs)
		: m_supportPoints(rhs.m_supportPoints)
		, m_bounds(rhs.m_bounds)
		, m_lengthApproximationTable(rhs.m_lengthApproximationTable)
	{}

	BezierParameterization& BezierParameterization::operator=(BezierParameterization rhs)
	{
		std::swap(m_supportPoints, rhs.m_supportPoints);
		std::swap(m_bounds, rhs.m_bounds);
		std::swap(m_lengthApproximationTable, rhs.m_lengthApproximationTable);
		std::swap(m_subdividedFirst, rhs.m_subdividedFirst);
		std::swap(m_subdividedSecond, rhs.m_subdividedSecond);
		return *this;
	}

	const std::array<vec2, 4>& BezierParameterization::getSupportPoints() const
	{
		return m_supportPoints;
	}


	double BezierParameterization::getArcLength() const
	{
		return m_lengthApproximationTable.back();
	}


	const Bounds2& BezierParameterization::getBounds() const
	{
		return m_bounds;
	}


	const BezierParameterization& BezierParameterization::getSubdividedFirst() const
	{
		if (m_subdividedFirst == nullptr)
			subdivide();
		return *m_subdividedFirst;
	}


	const BezierParameterization& BezierParameterization::getSubdividedSecond() const
	{
		if (m_subdividedSecond == nullptr)
			subdivide();
		return *m_subdividedSecond;
	}


	vec2 BezierParameterization::timeToCoordinate(double t) const
	{
		t = math::clamp(t, 0.0, 1.0);

		// direct computation
		const double tt = t - 1;
		const double tt2 = tt * tt;
		const double tt3 = tt2 * tt;

		return -(m_supportPoints[0] * tt3) + t * (3.0 * m_supportPoints[1] * tt2 + t * (3.0 * m_supportPoints[2] - 3.0 * m_supportPoints[2] * t + m_supportPoints[3] * t));
	}


	vec2 BezierParameterization::arcPositionToCoordinate(double position) const
	{
		return timeToCoordinate(arcPositionToTime(position));
	}


	double BezierParameterization::arcPositionToTime(double position) const
	{
		if (position <= m_lengthApproximationTable[0])
			return m_lengthApproximationTable[0];
		else if (position >= m_lengthApproximationTable.back())
			return m_lengthApproximationTable.back();

		auto it = std::lower_bound(m_lengthApproximationTable.begin(), m_lengthApproximationTable.end(), position);
		assert(it != m_lengthApproximationTable.end() && it != m_lengthApproximationTable.begin());
		double diff = *it - *(it - 1);
		double lborder = double(std::distance(m_lengthApproximationTable.begin(), it - 1));
		return (lborder + (position - *(it - 1)) / diff) / double(LengthApproximationTableSize - 1);
	}


	double BezierParameterization::timeToArcPosition(double time) const
	{
		if (time <= 0.0)
			return 0.0;
		if (time >= 1.0)
			return m_lengthApproximationTable.back();

		int index = int(time * (LengthApproximationTableSize - 1));
		return m_lengthApproximationTable[index] + ((time * (LengthApproximationTableSize - 1)) - index) * (m_lengthApproximationTable[index + 1] - m_lengthApproximationTable[index]);
	}


	vec2 BezierParameterization::derivateAtTime(double time) const
	{
		const auto& p0 = m_supportPoints[0];
		const auto& p1 = m_supportPoints[1];
		const auto& p2 = m_supportPoints[2];
		const auto& p3 = m_supportPoints[3];
		//3 (-p0 + p1 + 2 (p0 - 2 p1 + p2) t + (-p0 + 3 p1 - 3 p2 + p3) t^2)
		return 3.0 * (-p0 + p1 + 2.0 * (p0 - 2.0 * p1 + p2) * time + (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * time * time);
	}


	void BezierParameterization::computeLengthApproximationTable()
	{
		vec2 lastPoint = m_supportPoints[0];
		double sum = 0.0;
		for (int i = 0; i < LengthApproximationTableSize; ++i)
		{
			double t = double(i) / LengthApproximationTableSize;
			auto currentPoint = timeToCoordinate(t);

			sum += (currentPoint - lastPoint).norm();
			lastPoint = currentPoint;
			m_lengthApproximationTable[i] = sum;
		}
	}


	void BezierParameterization::subdivide() const
	{
		// First Iteration
		const vec2 p01 = m_supportPoints[0] + ((m_supportPoints[1] - m_supportPoints[0]) * 0.5);
		const vec2 p11 = m_supportPoints[1] + ((m_supportPoints[2] - m_supportPoints[1]) * 0.5);
		const vec2 p21 = m_supportPoints[2] + ((m_supportPoints[3] - m_supportPoints[2]) * 0.5);

		// Second Iteration:
		const vec2 p02 = p01 + ((p11 - p01) * 0.5);
		const vec2 p12 = p11 + ((p21 - p11) * 0.5);

		// Third Iteration:
		const vec2 p03 = p02 + ((p12 - p02) * 0.5);

		m_subdividedFirst = std::make_unique<BezierParameterization>(m_supportPoints[0], p01, p02, p03);
		m_subdividedSecond = std::make_unique<BezierParameterization>(p03, p12, p21, m_supportPoints[3]);
	}

}
}
