#ifndef CTS_CORE_CONNECTION_H__
#define CTS_CORE_CONNECTION_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/types.h>
#include <cts-core/network/bezierparameterization.h>
#include <cts-core/network/intersection.h>

#include <list>
#include <vector>

namespace cts { namespace core
{
	class AbstractVehicle;
	class Network;
	class Node;

	/**
	 * A connection between two nodes.
	 * Independent of its parameterization.
	 */
	class CTS_CORE_API Connection
	{
		friend class Network;

	public:
		/// Creates a new network connection with the given parameters.
		/// The Bézier support points are taken from the start and end nodes.
		/// \param	startNode		Start network node of this connection.
		/// \param	endNode			End network node of this connection.
		Connection(const Node& startNode, const Node& endNode);

		/// Returns the start network node of this connection.
		const Node& getStartNode() const;
		/// Returns the end network node of this connection.
		const Node& getEndNode() const;
		/// Returns the Bézier curve parameterization of this connection.
		const BezierParameterization& getCurve() const;
		/// Returns the list of vehicles currently on this connection, sorted by their position.
		const std::list<AbstractVehicle*>& getVehicles() const;

		/// Returns the priority of this network connection.
		int getPriority() const;
		/// Sets the priority of this network connection.
		void setPriority(int value);

		/// Returns the target velocity of this Connection in m/s.
		double getTargetVelocity() const;
		/// Sets the target velocity of this Connection in m/s.
		void setTargetVelocity(double value);


		/// Recalculates the Bézier parameterization curve based on the node's properties.
		void updateCurve();


		/// Returns the first vehicle to be found behind \e arcPosition within \e searchDistance.
		/// If \e searchDistance exceeds the length of this Connection, the function will recursively check
		/// all following connections.
		VehicleDistance getVehicleBehind(double arcPosition, double searchDistance) const;

		/// Returns the first vehicle to be found before \e arcPosition within \e searchDistance.
		/// If \e searchDistance exceeds the length of this Connection, the function will recursively check
		/// all previous connections.
		VehicleDistance getVehicleBefore(double arcPosition, double searchDistance) const;

	private:
		std::list<AbstractVehicle*>::const_iterator vehicleIteratorBehind(double arcPosition) const;
		std::list<AbstractVehicle*>::const_iterator vehicleIteratorBefore(double arcPosition) const;

		const Node& m_startNode;                    ///< Start network node of this connection.
		const Node& m_endNode;                      ///< End network node of this connection.
		BezierParameterization m_curve;				///< Bézier curve parameterization of this connection.

		int m_priority;								///< Priority of this network connection.
		double m_targetVelocity;					///< Target velocity of this network connection in m/s.

		std::list<AbstractVehicle*> m_vehicles;     ///< List of vehicles currently on this connection, sorted by their position.
		std::vector<Intersection> m_intersections;	///< List of intersections with other Connections, sorted by their position.
	};

}
}

#endif
