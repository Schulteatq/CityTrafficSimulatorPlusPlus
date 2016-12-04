#include <cts-core/base/math.h>

namespace cts { namespace math
{

	double distance(const vec2& lhs, const vec2 rhs)
	{
		return (lhs - rhs).norm();
	}

}
}
