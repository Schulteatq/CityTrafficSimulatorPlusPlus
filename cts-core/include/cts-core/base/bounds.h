#ifndef CTS_CORE_BOUNDS_H__
#define CTS_CORE_BOUNDS_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>

#include <array>
#include <limits>

namespace cts { namespace core
{
	template<typename T, size_t N>
	class Bounds
	{
	public:
		using VectorType = Eigen::Matrix<T, N, 1>;
		using ThisType = Bounds<T, N>;

		Bounds()
			: m_llf(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
			, m_urb(VectorType::Constant(std::numeric_limits<T>::quiet_NaN()))
		{}

		explicit Bounds(const VectorType& v)
			: m_llf(v)
			, m_urb(v)
		{}

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

		const VectorType& getLlf() const { return m_llf; }
		const VectorType& getUrb() const { return m_urb; }

		VectorType center() const { return m_llf + (m_urb - m_llf) * 0.5; }

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

		bool contains(const VectorType& v) const
		{
			for (int i = 0; i < VectorType::RowsAtCompileTime; ++i)
			{
				if (v[i] < m_llf[i] || v[i] > m_urb[i])
					return false;
			}
			return true;
		}


		bool intersects(const ThisType& other) const
		{
			for (int i = 0; i < VectorType::RowsAtCompileTime; ++i)
			{
				if (m_llf[i] > other.m_urb[i] || m_urb[i] < other.m_llf[i])
					return false;
			}
			return true;
		}


		void addPoint(const VectorType& v)
		{
			m_llf = m_llf.cwiseMin(v);
			m_urb = m_urb.cwiseMax(v);
		}

	private:
		VectorType m_llf;
		VectorType m_urb;
	};

	template<typename T, size_t N>
	std::ostream& operator<< (std::ostream& o, const Bounds<T, N>& b)
	{
		return (o << "(LLF: " << b.getLLF() << " URB: " << b.getURB() << ")");
	}


	using Bounds2 = Bounds<double, 2>;

}
}

#endif
