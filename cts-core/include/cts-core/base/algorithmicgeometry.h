#ifndef CTS_CORE_ALGORITHMIC_GEOMETRY_H__
#define CTS_CORE_ALGORITHMIC_GEOMETRY_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>

#include <vector>

namespace cts
{
	namespace math
	{
		/// Computes the orientation of the three points a, b, c.
		/// \param  a	First point.
		/// \param  b 	Second point.
		/// \param  c 	Third point.
		/// \return Left turn: &lt; 0, collinear: = 0, right turn: &gt; 0
		inline double orientation(const vec2& a, const vec2& b, const vec2& c)
		{
			return a[0] * b[1] + a[1] * c[0] + b[0] * c[1] - c[0] * b[1] - c[1] * a[0] - b[0] * a[1];
		}


		/// Checks whether the three points are collinear.
		/// \param  a	First point.
		/// \param  b 	Second point.
		/// \param  c 	Third point.
		inline bool collinear(const vec2& a, const vec2& b, const vec2& c)
		{
			return (orientation(a, b, c) == 0);
		}


		/// Checks whether the three points form a left turn.
		/// \param  a	First point.
		/// \param  b 	Second point.
		/// \param  c 	Third point.
		inline bool leftTurn(const vec2& a, const vec2& b, const vec2& c)
		{
			return (orientation(a, b, c) > 0);
		}


		/// Checks whether the three points form a right turn.
		/// \param  a	First point.
		/// \param  b 	Second point.
		/// \param  c 	Third point.
		inline bool rightTurn(const vec2& a, const vec2& b, const vec2& c)
		{
			return (orientation(a, b, c) > 0);
		}


		CTS_CORE_API std::vector<vec2> convexHull(std::vector<vec2> points);
	}
}

#endif
