#ifndef CTS_CORE_BOUNDS_H__
#define CTS_CORE_BOUNDS_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>

#include <array>
#include <initializer_list>
#include <limits>

namespace cts { namespace core
{
	/**
	 * Utility class to represent axis-aligned bounds represented by a lower-left-front corner
	 * (component-wise minimum) and an upper-right-back corner (component-wise maximum).
	 * 
	 * \tparam	T	Scalar base element type.
	 * \tparam	N	Dimensionality of the bounds.
	 */
	template<typename T, size_t N>
	class Bounds
	{
	public:
		/// Alias for the vector type
		using VectorType = Eigen::Matrix<T, N, 1>;

		/// Creates an empty bounding box that does not contain any point and does not intersect with anything.
		Bounds()
			: m_llf(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
			, m_urb(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
		{}

		/// Creates a bounding box that consists of the single given point.
		explicit Bounds(const VectorType& v)
			: m_llf(v)
			, m_urb(v)
		{}

		/// Creates a bounding box that consists all of the points in the given array.
		template<size_t K>
		explicit Bounds(const std::array<VectorType, K>& points)
			: m_llf(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
			, m_urb(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
		{
			for (size_t i = 0; i < K; ++i)
			{
				addPoint(points[i]);
			}
		}

		/// Creates a bounding box that consists all of the points in the given initializer list.
		Bounds(std::initializer_list<VectorType> ilist)
			: Bounds()
		{
			for (auto it = ilist.begin(); it != ilist.end(); ++it)
			{
				addPoint(*it);
			}
		}

		/// Returns the lower-left-front corner (component-wise minimum) of the bounding box.
		const VectorType& getLlf() const { return m_llf; }
		/// Returns the upper-right-back corner (component-wise maximum) of the bounding box.
		const VectorType& getUrb() const { return m_urb; }
		/// Returns the center of the bounding box.
		VectorType center() const { return m_llf + (m_urb - m_llf) * 0.5; }

		/// Returns the volume of the bouning box.
		T volume() const
		{
			const auto diff = m_urb - m_llf;
			T toReturn = T(1);
			for (int i = 0; i < VectorType::RowsAtCompileTime; ++i)
			{
				toReturn *= diff[i];
			}
			return toReturn;
		}

		/// Returns whether the bounding box contains the given vector.
		bool contains(const VectorType& v) const
		{
			for (int i = 0; i < VectorType::RowsAtCompileTime; ++i)
			{
				// use negated version to correctly detect NaN
				if (!(v[i] >= m_llf[i] && v[i] <= m_urb[i]))
					return false;
			}
			return true;
		}

		/// Returns whether the bounding box intersects the given other bounding box.
		bool intersects(const Bounds<T, N>& other) const
		{
			for (int i = 0; i < VectorType::RowsAtCompileTime; ++i)
			{
				// use negated version to correctly detect NaN
				if (!(m_llf[i] <= other.m_urb[i] && m_urb[i] >= other.m_llf[i]))
					return false;
			}
			return true;
		}

		/// Extends this bounding box to include the given vector.
		void addPoint(const VectorType& v)
		{
			m_llf = m_llf.cwiseMin(v);
			m_urb = m_urb.cwiseMax(v);
		}

	private:
		VectorType m_llf;	///< The current lower-left-front corner (component-wise minimum) of this bounding box.
		VectorType m_urb;	///< The current upper-right-back corner (component-wise maximum) of this bounding box.
	};

	template<typename T, size_t N>
	std::ostream& operator<< (std::ostream& o, const Bounds<T, N>& b)
	{
		return (o << "(LLF: " << b.getLLF() << " URB: " << b.getURB() << ")");
	}

	/// Alias for the default 2D axis aligned bounding box.
	using Bounds2 = Bounds<double, 2>;

}
}

#endif
