#ifndef CTS_GUI_LUA_ITEM_MODEL_H__
#define CTS_GUI_LUA_ITEM_MODEL_H__

#include <cts-gui/config.h>
#include <cts-gui/tools/treeitem.h>

#include <sol.hpp>

#include <QtCore/QAbstractItemModel>


namespace cts { namespace gui {

// = TreeModel items ==============================================================================
	
	/// Base class for LuaTableTreeWidget items
	class CTS_GUI_API LuaTreeItem : public TreeItem
	{
	public:
		enum ModelStyle
		{ 
			FullModel,		///< Full model containing the entire Lua data hierarchy
			CompleterModel	///< Reduced model containing only the parts needed for code completion
		};

		LuaTreeItem(ModelStyle modelStyle, const std::string& name, sol::type type, TreeItem* parent = nullptr);
		virtual ~LuaTreeItem() = default;
		virtual QVariant getData(int column, int role) const;

	protected:
		/// Returns the value of the underlying Lua object.
		virtual QString getValue() const;

		std::string m_name;          ///< Name of the variable
		sol::type m_type;            ///< Lua type of the variable
		ModelStyle m_modelStyle;     ///< Model style for this tree item
	};


	/// The Root Item
	class CTS_GUI_API LuaTreeRootItem : public TreeItem
	{
	public:
		explicit LuaTreeRootItem(TreeItem* parent = 0);
		virtual ~LuaTreeRootItem() = default;
		virtual QVariant getData(int column, int role) const;
	};


	/// Specialization for normal leafs
	class CTS_GUI_API LuaTreeItemLeaf : public LuaTreeItem
	{
	public:
		LuaTreeItemLeaf(ModelStyle modelStyle, sol::table parentTable, const std::string& name, sol::type type, TreeItem* parent);
		virtual ~LuaTreeItemLeaf() = default;

	private:
		virtual QString getValue() const;

		sol::table m_parentTable;    ///< this item's parent LuaTable
	};

	
	/// Specialization for normal table items
	class CTS_GUI_API LuaTreeItemTable : public LuaTreeItem
	{
	public:
		LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, sol::table thisTable, const std::string& name, sol::type type, TreeItem* parent);
		virtual ~LuaTreeItemTable() = default;
		virtual QVariant getData(int column, int role) const;

	private:
		//void recursiveGatherSwigMethods(const std::shared_ptr<LuaTable>& baseTable, TreeItem* parent);

		sol::table m_thisTable;	///< this item's LuaTable
		bool m_isMetatable;		///< Flag whether this item represents a Metatable (currently only used for printing purposes)
		void* m_tablePtr;		///< Raw pointer to the object behind the lua table. Used to detect cyclic references.
	};


// = TreeModel ====================================================================================


	/// QItemModel for displaying a list of pipelines and their processors in the LuaTableTreeWidget.
	class CTS_GUI_API LuaItemModel : public QAbstractItemModel 
	{
		Q_OBJECT

	public:
		explicit LuaItemModel(QObject *parent = nullptr);
		~LuaItemModel();

		void setData(sol::state* luaState, LuaTreeItem::ModelStyle modelStyle);

		QVariant data(const QModelIndex &index, int role) const;

		bool setData(const QModelIndex& index, const QVariant& value, int role);

		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

	private:
		TreeItem* m_rootItem;
	};

}
}

#endif
