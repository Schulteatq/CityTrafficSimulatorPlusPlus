#ifndef CTS_CORE_BEZIERPARAMETERIZATION_H__
#define CTS_CORE_BEZIERPARAMETERIZATION_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>
#include <cts-core/base/bounds.h>

#include <array>
#include <memory>

namespace cts { namespace core
{
	/**
	 * Bézier parameterization of a network connection.
	 * While a Connection only describes the topology of the network and that vehicles can travel
	 * between two nodes, its BezierParameterization describes the actual shape of the connection.
	 * It is represented by a Bézier curve defined by four support points. This class provides various
	 * convenience methods to convert between time, arc length and their corresponding world position.
	 */
	class CTS_CORE_API BezierParameterization
	{
	private:
		/// Size of m_lengthApproximationTable.
		enum { LengthApproximationTableSize = 32 };

	public:
		/// Creates a new BezierParameterization with the given parameters.
		/// \param	p0  First Bézier support point in world coordinates.
		/// \param	p1  Second Bézier support point in world coordinates.
		/// \param	p2  Third Bézier support point in world coordinates.
		/// \param	p3  Fourth Bézier support point in world coordinates.
		BezierParameterization(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3);

		// Custom copy ctor and assignment operator that do not copy the subdivision information.
		BezierParameterization(const BezierParameterization& rhs);
		BezierParameterization& operator=(BezierParameterization rhs);

		/// Returns the array of Bézier support points.
		const std::array<vec2, 4>& getSupportPoints() const;

		/// Returns the arc length of this parameterization in dm.
		double getArcLength() const;

		/// Returns the axis-aligned bounding box of this Bézier curve.
		const Bounds2& getBounds() const;

		/// Returns the BezierParameterization representing the first half (time parameterization) of this curve.
		const BezierParameterization& getSubdividedFirst() const;
		/// Returns the BezierParameterization representing the second half (time parameterization) of this curve.
		const BezierParameterization& getSubdividedSecond() const;


		/// Converts the given time on this parameterization to its corresponding world position.
		/// \param	time    Location on this curve in terms of time.
		vec2 timeToCoordinate(double time) const;

		/// Converts the given arc position on this parameterization to its corresponding world position.
		/// \param	arcPosition	Location on this curve in terms of arc position (dm).
		vec2 arcPositionToCoordinate(double arcPosition) const;

		/// Converts the given arc position on this parameterization to its time.
		/// \param	arcPosition	Location on this curve in terms of arc position (dm).
		double arcPositionToTime(double arcPosition) const;

		/// Converts the given time on this parameterization to its corresponding arc position.
		/// \param	time    Location on this curve in terms of time.
		double timeToArcPosition(double time) const;

		/// Returns the derivate of the Bézier curve at the given time.
		/// \param  time	Location on this curve in terms of time.
		vec2 derivateAtTime(double time) const;

	private:
		/// Computes the LUT to convert between time and arc position.
		void computeLengthApproximationTable();

		/// Computes the subdivision of this into two Bézier curve following the same path.
		/// The result is stored in m_subdividedFirst and m_subdividedSecond.
		void subdivide() const;

		/// Array of the four support points.
		std::array<vec2, 4> m_supportPoints; 

		/// Axis-aligned bounding box of this Bézier curve.
		Bounds2 m_bounds;

		/// LUT to convert between time and arc position.
		std::array<double, LengthApproximationTableSize> m_lengthApproximationTable;

		mutable std::unique_ptr<BezierParameterization> m_subdividedFirst;
		mutable std::unique_ptr<BezierParameterization> m_subdividedSecond;
	};

}
}

#endif // CTS_BEZIERPARAMETERIZATION_H__