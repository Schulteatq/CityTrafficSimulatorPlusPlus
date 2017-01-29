#ifndef CTS_CORE_LOCATION_H__
#define CTS_CORE_LOCATION_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/signal.h>

#include <string>
#include <vector>

namespace cts { namespace core
{
	class Node;

	class CTS_CORE_API Location : public SignalReceiver
	{
	public:
		Location(const std::vector<Node*>& nodes, const std::string& title);

		/// Returns the list of nodes of this location.
		const std::vector<Node*>& getNodes() const;

		/// Returns the title of this location.
		const std::string& getTitle() const;
		/// Sets the title of this location.
		void setTitle(const std::string& value);

	private:
		void onNodeDeleted(Node* node);

		std::vector<Node*> m_nodes;		///< List of nodes of this location.
		std::string m_title;			///< Title of this location.

	};


}
}

#endif // CTS_NETWORK_H__
