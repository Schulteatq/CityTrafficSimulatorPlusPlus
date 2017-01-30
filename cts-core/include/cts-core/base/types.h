#ifndef CTS_CORE_TYPES_H__
#define CTS_CORE_TYPES_H__

#include <cts-core/coreapi.h>
#include <limits>
#include <utility>

namespace cts
{
	namespace core
	{
		class AbstractVehicle;

		/**
		 * Simple struct modeling a vehicle paired with a distance.
		 */
		struct CTS_CORE_API VehicleDistance
		{
			/// Creates an empty VehicleDistance using a nullptr and DBL_MAX distance.
			VehicleDistance()
				: vehicle(nullptr)
				, distance(std::numeric_limits<double>::max())
			{}

			/// Creates a VehicleDistance with the given parameters
			VehicleDistance(AbstractVehicle* vehicle, double distance)
				: vehicle(vehicle)
				, distance(distance)
			{}

			/// Returns whether the vehicle pointer is null and thus the VehicleDistance is considered empty.
			bool empty() const
			{
				return vehicle == nullptr;
			}

			/// Computes the minimum of the two given VehicleDistance instances.
			static VehicleDistance min(VehicleDistance lhs, VehicleDistance rhs)
			{
				if (lhs.empty() && rhs.empty())
					return VehicleDistance();
				if (lhs.empty() && !rhs.empty())
					return rhs;
				if (!lhs.empty() && rhs.empty())
					return lhs;
				else
					return (lhs.distance < rhs.distance) ? lhs : rhs;
			}

			AbstractVehicle* vehicle;	///< The vehicle.
			double distance;			///< The corresponding distance.
		};
	}
}

#endif
