#ifndef CTS_CORE_INTERSECTION_H__
#define CTS_CORE_INTERSECTION_H__

#include <cts-core/coreapi.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/node.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{
	class Connection;

	/**
	 * Represents a logical intersection between two network connections.
	 * An Intersection is parameterized by the two connections intersecting and the its location
	 * on them.
	 */
	class CTS_CORE_API Intersection
	{
	public:
		/// Creates a new Intersection with the given parameters.
		/// \param	aConnection     First network connection intersecting.
		/// \param	aTime           Location of the intersection on aConnection.
		/// \param	bConnection     Second network connection intersecting.
		/// \param	bTime           Location of the intersection on bConnection.
		Intersection(const Connection& aConnection, double aTime, const Connection& bConnection, double bTime);

		/// Checks whether vehicles should keep this intersection clear in case that they won't be 
		/// able to pass it completely.
		bool avoidBlocking() const;

		/// Returns the first network connection intersection.
		const Connection& getFirstConnection() const;
		/// Returns the second network connection intersection.
		const Connection& getSecondConnection() const;

		double getFirstTime() const;
		double getSecondTime() const;

		double getFirstArcPosition() const;
		double getSecondArcPosition() const;

		vec2 getFirstCoordinate() const;
		vec2 getSecondCoordinate() const;

		/// Return the distance vehicles should keep in case they need to wait in front.
		double getWaitingDistance() const;

	private:
		const Connection* m_aConnection;    ///< First network connection intersecting.
		const Connection* m_bConnection;    ///< Second network connection intersecting.

		double m_aTime;                     ///< Location of the intersection on aConnection in terms of parameterization time.
		double m_bTime;                     ///< Location of the intersection on bConnection in terms of parameterization time.
		double m_aArcPosition;              ///< Location of the intersection on aConnection in terms of arc length.
		double m_bArcPosition;              ///< Location of the intersection on bConnection in terms of arc length.
		double m_waitingDistance;           ///< Distance vehicles should keep in case they need to wait in front.
	};
}
}

#endif
