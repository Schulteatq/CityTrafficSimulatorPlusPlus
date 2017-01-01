#include <cts-core/base/algorithmicgeometry.h>

#include <algorithm>

namespace cts { namespace math
{

	// Implementation note: copy of the argument vector intended.
	std::vector<vec2> convexHull(std::vector<vec2> points)
	{
		// if we have less than 3 points, the solution is simple
		if (points.size() < 3)
			return points;

		// find point with minimum Y coordinate
		vec2 yMin = points[0];
		for (auto& p : points)
		{
			if (p[1] < yMin[1])
				yMin = p;
		}

		// compute the polar angle of each point around yMin
		std::vector< std::pair<vec2, double> > pointAngleVec;
		pointAngleVec.reserve(points.size());
		for (auto& p : points)
		{
			const auto offset = p - yMin;
			pointAngleVec.push_back(std::make_pair(p, std::atan2(offset[1], offset[0])));
		}

		// sort points by polar angle around yMin
		std::sort(pointAngleVec.begin(), pointAngleVec.end(), [yMin](const std::pair<vec2, double>& lhs, const std::pair<vec2, double>& rhs) {
			return lhs.second < rhs.second;
		});

		// now do the Graham's scan
		std::vector<vec2> toReturn;
		toReturn.reserve(pointAngleVec.size());
		toReturn.push_back(pointAngleVec[0].first);
		toReturn.push_back(pointAngleVec[1].first);
		toReturn.push_back(pointAngleVec[2].first);
		for (size_t i = 3; i < pointAngleVec.size(); ++i)
		{
			while (!leftTurn(toReturn[toReturn.size() - 2], toReturn.back(), pointAngleVec[i].first))
				toReturn.pop_back();
			toReturn.push_back(pointAngleVec[i].first);
		}

		return toReturn;
	}

}
}
