#ifndef CTS_GUI_TREEITEM_H__
#define CTS_GUI_TREEITEM_H__

#include <cts-gui/config.h>

#include <QList>
#include <QVariant>

namespace cts { namespace gui {

	/// Abstract base class for TreeView items of the PipelineTreeWidget.
	class CTS_GUI_API TreeItem
	{
	public:
		/// Creates a new TreeItem 
		/// \param  parent	Parent item, will take ownership of this TreeItem.
		explicit TreeItem(TreeItem* parent = nullptr);

		/// Destructor, also deletes all child items.
		virtual ~TreeItem();

		/// Return the of this item at the column \a column.
		/// To be overwritten by subclasses.
		/// \param   column  Column
		/// \param   role    Access role (Use Qt::DisplayRole for string representation, Qt::UserRole for the internal data)
		virtual QVariant getData(int column, int role) const = 0;

		/// Sets the data at the given column to \a value using the given role.
		/// Overwrite if needed, default implementation will only return false.
		/// \param   column  Column
		/// \param   role    Access role
		/// \param   value   Value to set
		virtual bool setData(int column, int role, const QVariant& value) const;

		/// Returns the parent TreeItem
		TreeItem* getParent();

		/// Returns the \a row-th child TreeItem.
		TreeItem* getChild(int row);

		/// Returns the row of this TreeItem.
		int getRow();

		/// Returns the number of children.
		int getChildCount();

		/// Inserts a child at the given row.
		void insertChild(int row, TreeItem* child);

		/// Removes the child at the given row.
		void removeChild(int row);

		/// Replaces the child with index \a row with the given TreeItem.
		/// \param   row     Child to replace, must be < than getChildCount()
		/// \param   child   New TreeItem for the child
		void replaceChild(int row, TreeItem* child);

		/// Removes all children.
		void clearChildren();

		/// Dumps debug output of the tree hierarchy.
		/// \param t     TreeItem to dump
		/// \param level Starting indentation level
		static void dumpTree(TreeItem* t, int level = 0);

	protected:
		TreeItem* _parent;              ///< Parent TreeItem.
		QList<TreeItem*> _children;     ///< Collection of all child items.
	};


}
}

#endif 