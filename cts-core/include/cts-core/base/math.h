#ifndef CTS_CORE_MATH_H__
#define CTS_CORE_MATH_H__

#include <cts-core/coreapi.h>
#include <Eigen/Dense>

namespace cts
{
	using vec2 = Eigen::Vector2d;

	namespace math
	{
		static const double PI = 3.14159265358979323846;

		/// Comutes the distance between the two given points.
		inline double distance(const vec2& lhs, const vec2& rhs)
		{
			return (lhs - rhs).norm();
		}

		
		/// Computes the clockwise rotation of 90° of \e p.
		inline vec2 rotatedClockwise(const vec2& p)
		{
			return vec2(p[1], -p[0]);
		}


		/// Computes the counter-clockwise rotation of 90° of \e p.
		inline vec2 rotatedCounterClockwise(const vec2& p)
		{
			return vec2(-p[1], p[0]);
		}


		/// Computes the counter-clockwise rotation of \e phi of \e p.
		inline vec2 rotated(const vec2&p, double phi)
		{
			return vec2(std::cos(phi) * p[0] - std::sin(phi) * p[1], std::sin(phi) * p[0] + std::cos(phi) * p[1]);
		}


		template<typename T>
		T clamp(T value, T min, T max)
		{
			return std::min(max, std::max(min, value));
		}
	}
}

#endif
