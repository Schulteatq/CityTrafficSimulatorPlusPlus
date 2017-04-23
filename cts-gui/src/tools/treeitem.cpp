#include <cts-gui/tools/treeitem.h>

#include <cassert>
#include <iostream>

namespace cts { namespace gui {

	TreeItem::TreeItem(TreeItem* parent /*= 0*/)
		: _parent(parent)
	{
		if (_parent != 0)
			_parent->_children.append(this);
	}


	TreeItem::~TreeItem()
	{
		qDeleteAll(_children);
	}


	TreeItem* TreeItem::getParent()
	{
		return _parent;
	}


	TreeItem* TreeItem::getChild(int row)
	{
		return _children.value(row);
	}


	int TreeItem::getRow()
	{
		if (_parent)
			return _parent->_children.indexOf(const_cast<TreeItem*>(this));

		return 0;
	}


	int TreeItem::getChildCount()
	{
		return _children.count();
	}


	bool TreeItem::setData(int column, int role, const QVariant& value) const
	{
		return false;
	}


	void TreeItem::insertChild(int row, TreeItem* child)
	{
		assert(row < 0 || row > _children.size());

		_children.insert(row, child);
		child->_parent = this;
	}


	void TreeItem::removeChild(int row)
	{
		assert(row < 0 || row > _children.size());
		delete _children.takeAt(row);
	}


	void TreeItem::replaceChild(int row, TreeItem* child)
	{
		assert(row < getChildCount());
		delete _children[row];
		_children[row] = child;
		child->_parent = this;
	}


	void TreeItem::clearChildren()
	{
		qDeleteAll(_children);
		_children.clear();
	}


	void TreeItem::dumpTree(TreeItem* t, int level /*= 0*/)
	{
		std::cout << std::string(level, ' ') << t << "\n";
		for (int i = 0; i < t->getChildCount(); ++i) {
			TreeItem::dumpTree(t->getChild(i), level + 1);
		}
	}

}
}