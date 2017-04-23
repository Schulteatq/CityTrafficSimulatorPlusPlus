#ifndef CTS_GUI_LUATABLEWIDGET_H__
#define CTS_GUI_LUATABLEWIDGET_H__

#include <cts-gui/config.h>
#include <cts-gui/tools/treeitem.h>

#include <sol.hpp>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QVariant>
#include <QtWidgets/QTreeWidget>

#include <map>
#include <memory>


namespace cts { namespace gui {
	class DataContainer;

// = TreeModel items ==============================================================================
	
	/// Base class for LuaTableTreeWidget items
	class CTS_GUI_API LuaTreeItem : public TreeItem {
	public:
		enum ModelStyle { FULL_MODEL, COMPLETER_MODEL };

		LuaTreeItem(ModelStyle modelStyle, const std::string& name, sol::type type, TreeItem* parent = nullptr);

		// Virtual Destructor
		virtual ~LuaTreeItem() {}

		/// \see TreeItem::getData()
		virtual QVariant getData(int column, int role) const;

	protected:
		std::string _name;          ///< Name of the variable
		sol::type _type;            ///< Lua type of the variable
		ModelStyle _modelStyle;     ///< Model style for this tree item

	private:
		virtual QString getValue() const;
	};


	/// The Root Item
	class CTS_GUI_API LuaTreeRootItem : public TreeItem {
	public:
		explicit LuaTreeRootItem(TreeItem* parent = 0);
		virtual ~LuaTreeRootItem();

		/// \see TreeItem::getData()
		virtual QVariant getData(int column, int role) const;
	};


	/// Specialization for normal leafs
	class CTS_GUI_API LuaTreeItemLeaf : public LuaTreeItem {
	public:
		/**
		 * Creates a new TreeItem for a QtDataHandle
		 * \param   name        Name of the Lua variable
		 * \param   type        Lua type of the variable
		 * \param   parent      Parent TreeItem
		 */
		LuaTreeItemLeaf(ModelStyle modelStyle, sol::table parentTable, const std::string& name, sol::type type, TreeItem* parent);

		/// Destructor
		virtual ~LuaTreeItemLeaf();

	private:
		sol::table _parentTable;    ///< this item's parent LuaTable
		virtual QString getValue() const;
	};

	
	/// Specialization for normal table items
	class CTS_GUI_API LuaTreeItemTable : public LuaTreeItem {
	public:
		/**
		 * Creates a new TreeItem for a QtDataHandle
		 * \param   name        Name of the Lua variable
		 * \param   type        Lua type of the variable
		 * \param   parent      Parent TreeItem
		 */
		LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, sol::table thisTable, const std::string& name, sol::type type, TreeItem* parent);

		/// Destructor
		virtual ~LuaTreeItemTable();

		/// \see TreeItem::getData()
		virtual QVariant getData(int column, int role) const;

	private:
		//void recursiveGatherSwigMethods(const std::shared_ptr<LuaTable>& baseTable, TreeItem* parent);

		sol::table _thisTable;		///< this item's LuaTable
		bool _isMetatable;			///< Flag whether this item represents a Metatable (currently only used for printing purposes)
		void* _dataPtr;
	};

// = TreeModel ====================================================================================

	/**
	 * QItemModel for displaying a list of pipelines and their processors in the LuaTableTreeWidget.
	 */
	class CTS_GUI_API LuaTableTreeModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		explicit LuaTableTreeModel(QObject *parent = 0);
		~LuaTableTreeModel();

		void setData(sol::state* luaVmState, LuaTreeItem::ModelStyle modelStyle);

		QVariant data(const QModelIndex &index, int role) const;

		/**
		 * Sets the data at index \a index to \a value.
		 * \param   index   position where to set data
		 * \param   value   new data
		 * \param   role    edit role
		 **/
		bool setData(const QModelIndex& index, const QVariant& value, int role);

		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

	private:
		TreeItem* _rootItem;
		std::map<QString, LuaTreeItemLeaf*> _itemMap;  ///< Mapping the QtDataHandle Keys to the TreeItems
	};

// = Widget =======================================================================================

	/**
	 * Qt widget for showing a list of pipelines and their processors in a QTreeView.
	 */
	class CTS_GUI_API LuaTableTreeWidget : public QTreeView {
		Q_OBJECT;

	public:
		/**
		 * Creates a new LuaTableTreeWidget.
		 * \param   parent  Parent widget
		 */
		explicit LuaTableTreeWidget(QWidget* parent = 0);

		/**
		 * Destructor
		 */
		virtual ~LuaTableTreeWidget();

		/**
		 * Returns the data model for the TreeView.
		 * \return  _treeModel
		 */
		LuaTableTreeModel* getTreeModel();


	public slots:
		/**
		 * Updates the data in the tree view by the given collection of pipelines \a pipelines.
		 * \param   pipelines   
		 */
		void update(sol::state& luaVmState, LuaTreeItem::ModelStyle modelStyle);



	private:
		/**
		 * Sets up the widget.
		 */
		void setupWidget();

		LuaTableTreeModel* _treeModel;		///< Data model for the TreeView.
		QSortFilterProxyModel* _sortModel;	///< sorting proxy model

	};
}
}

#endif
