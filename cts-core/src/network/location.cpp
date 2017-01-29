#include <cts-core/base/utils.h>
#include <cts-core/network/location.h>
#include <cts-core/network/node.h>

namespace cts { namespace core
{

	Location::Location(const std::vector<Node*>& nodes, const std::string& title)
		: m_nodes(nodes)
		, m_title(title)
	{
		for (auto node : m_nodes)
		{
			node->s_deleted.connect(this, &Location::onNodeDeleted);
		}
	}


	const std::vector<Node*>& Location::getNodes() const
	{
		return m_nodes;
	}


	const std::string& Location::getTitle() const
	{
		return m_title;
	}


	void Location::setTitle(const std::string& value)
	{
		m_title = value;
	}


	void Location::onNodeDeleted(Node* node)
	{
		utils::remove_erase(m_nodes, node);
	}


}
}
