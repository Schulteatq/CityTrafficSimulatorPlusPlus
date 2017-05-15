#include <cts-gui/tools/luaitemmodel.h>
#include <cts-lua/cts-lua.h>

#include <cassert>

extern "C" {
#include "lapi.h"
#include "lobject.h"
}

namespace
{
	StkId index2addr(lua_State *L, int idx) {
		CallInfo *ci = L->ci;
		if (idx > 0) {
			TValue *o = ci->func + idx;
			api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
			if (o >= L->top) return 0;
			else return o;
		}
		else if (!(idx <= LUA_REGISTRYINDEX)) {  /* negative index */
			api_check(L, idx != 0 && -idx <= L->top - (ci->func + 1), "invalid index");
			return L->top + idx;
		}
		else if (idx == LUA_REGISTRYINDEX)
			return &G(L)->l_registry;
		else {  /* upvalues */
			idx = LUA_REGISTRYINDEX - idx;
			api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
			if (ttislcf(ci->func))  /* light C function? */
				return 0;  /* it has no upvalues */
			else {
				CClosure *func = clCvalue(ci->func);
				return (idx <= func->nupvalues) ? &func->upvalue[idx - 1] : 0;
			}
		}
	}


	void* getTablePtr(sol::table& table)
	{
		int numElemsPushed = table.push();
		void* toReturn = hvalue(index2addr(table.lua_state(), -1));
		lua_pop(table.lua_state(), numElemsPushed);
		return toReturn;
	}
}

namespace cts { namespace gui {

	namespace
	{
		const int COLUMN_NAME = 0;
		const int COLUMN_TYPE = 1;
		const int COLUMN_VALUE = 2;
	}


// = TreeModel items ==============================================================================


	LuaTreeItem::LuaTreeItem(ModelStyle modelStyle, const std::string& name, sol::type type, TreeItem* parent /*= nullptr*/)
		: TreeItem(parent)
		, m_name(name)
		, m_type(type) 
		, m_modelStyle(modelStyle)
	{
	}


	QVariant LuaTreeItem::getData(int column, int role) const
	{
		switch (role) {
			case Qt::EditRole: // fallthrough
			case Qt::DisplayRole:
				if (column == COLUMN_NAME)
					return QVariant(QString::fromStdString(m_name));
				else if (column == COLUMN_TYPE)
					return QVariant(QString::fromStdString(lua_typename(0, int(m_type))));
				else if (column == COLUMN_VALUE)
					return getValue();
				else
					return QVariant();
			default:
				return QVariant();
		}
	}


	QString LuaTreeItem::getValue() const
	{
		return QString("");
	}


	LuaTreeRootItem::LuaTreeRootItem(TreeItem* parent /*= 0*/)
		: TreeItem(parent)
	{}


// ================================================================================================


	QVariant LuaTreeRootItem::getData(int column, int role) const {
		if (role == Qt::DisplayRole) {
			if (column == COLUMN_NAME)
				return QVariant(QString("Name"));
			else if (column == COLUMN_TYPE)
				return QVariant(QString("Data Type"));
			else if (column == COLUMN_VALUE)
				return QVariant(QString("Value"));
		}

		return QVariant();
	}


// ================================================================================================


	LuaTreeItemLeaf::LuaTreeItemLeaf(ModelStyle modelStyle, sol::table parentTable, const std::string& name, sol::type type, TreeItem* parent)
		: LuaTreeItem(modelStyle, name, type, parent)
		, m_parentTable(parentTable)
	{
		if (parentTable[m_name][sol::metatable_key].valid())
		{
			new LuaTreeItemTable(m_modelStyle, true, parentTable[m_name][sol::metatable_key], name, sol::type::table, this);
		}
	}


	QString LuaTreeItemLeaf::getValue() const
	{
		switch (m_type)
		{
		case sol::type::string:
			return QString::fromStdString(m_parentTable[m_name].get<std::string>());
		case sol::type::number:
			return QString::number(m_parentTable[m_name].get<double>());
		case sol::type::boolean:
			return m_parentTable[m_name].get<bool>() ? "true" : "false";
		default:
			return "";
		}
	}


// ================================================================================================


	LuaTreeItemTable::LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, sol::table thisTable, const std::string& name, sol::type type, TreeItem* parent)
		: LuaTreeItem(modelStyle, name, type, parent)
		, m_thisTable(thisTable)
		, m_isMetatable(isMetatable)
		, m_tablePtr(getTablePtr(thisTable))
	{
		// fill the table with values depending on model style
		if (!m_isMetatable || m_modelStyle == FullModel)
		{
			// create a new metatable
			if (m_thisTable[sol::metatable_key].valid())
				new LuaTreeItemTable(m_modelStyle, true, m_thisTable[sol::metatable_key], name, sol::type::table, this);

			m_thisTable.for_each([this](sol::object key, sol::object value) {
				std::string itemName = key.as<std::string>();
				sol::type luaType = value.get_type();

				if (itemName == "_G")
					return;

				if (luaType == sol::type::table)
				{
					sol::table thatTable = m_thisTable[itemName];
					void* thatRawPtr = getTablePtr(thatTable);

					// detect cyclic table references
					LuaTreeItemTable* pTable = this;
					while (pTable = dynamic_cast<LuaTreeItemTable*>(pTable->_parent))
					{
						if (pTable->m_tablePtr == thatRawPtr)
						{
							new LuaTreeItemLeaf(m_modelStyle, m_thisTable, "\xF0\x9F\x94\x97 " + itemName, luaType, this);
							return;
						}
					}

					new LuaTreeItemTable(m_modelStyle, false, m_thisTable[itemName], itemName, luaType, this);
				}
				else
				{
					new LuaTreeItemLeaf(m_modelStyle, m_thisTable, itemName, luaType, this);
				}
			});
		}
		else if (m_isMetatable && m_modelStyle == CompleterModel)
		{
			//sol::function indexFun = m_thisTable["__index"];
			//if (indexFun.valid())
			//{
			//	auto L = m_thisTable.lua_state();
			//	int n = indexFun.push();
			//	sol::usertype_metatable_core& umc = sol::stack::get<sol::light<sol::usertype_metatable_core>>(L, sol::upvalue_index(1));
			//	indexFun.pop();
			//}

			// It would be really cool if we could access the original sol::usertype_metatable_core object at this point.
			// It exists somewhere in the Lua registry, but unfortunately I could not receive it yet. Thus, we use this
			// simple way of parsing the available functions.
			m_thisTable.for_each([this](sol::object key, sol::object value) {
				std::string itemName = key.as<std::string>();
				sol::type luaType = value.get_type();

				if (luaType == sol::type::function)
				{
					if ((itemName.compare(0, 2, "__") != 0) && (itemName.compare(0, 4, "\xF0\x9F\x8C\xB2") != 0))
						new LuaTreeItemLeaf(m_modelStyle, m_thisTable, itemName, luaType, this);
				}
			});
		}
		else
		{
			// should not reach this
			assert(false);
		}
	}


	QVariant LuaTreeItemTable::getData(int column, int role) const
	{
		if (m_isMetatable && column == COLUMN_NAME && (role == Qt::EditRole || role == Qt::DisplayRole)) {
			switch (m_modelStyle) {
				case FullModel:
					return QVariant("[Metatable]");
				case CompleterModel:
					return QVariant("[Methods]");
				default:
					return QVariant("");
			}
		}
		else {
			return LuaTreeItem::getData(column, role);
		}
	}


	//void LuaTreeItemTable::recursiveGatherSwigMethods(const std::shared_ptr<LuaTable>& baseTable, TreeItem* parent) {
	//	// first get functions
	//	auto fnTable = baseTable->getTable(".fn");
	//	if (fnTable) {
	//		auto& valueMap = fnTable->getValueMap();
	//		for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
	//			const std::string& itemName = it->first;
	//			int luaType = it->second.luaType;
	//			if (luaType == LUA_TFUNCTION && itemName.substr(0, 2) != "__") 
	//				new LuaTreeItemLeaf(_modelStyle, fnTable, itemName, luaType, parent);
	//		}
	//	}
	//
	//	// now walk through base classes and recursively gather their methods
	//	auto basesTable = baseTable->getTable(".bases");
	//	if (basesTable) {
	//		auto& valueMap = basesTable->getValueMap();
	//		for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
	//			const std::string& itemName = it->first;
	//			int luaType = it->second.luaType;
	//
	//			if (luaType == LUA_TTABLE)
	//				recursiveGatherSwigMethods(basesTable->getTable(itemName), parent);
	//		}
	//	}
	//}


	// = LuaTableTreeModel ============================================================================
	

	LuaItemModel::LuaItemModel(QObject *parent /*= 0*/)
		: QAbstractItemModel(parent)
		, m_rootItem(new LuaTreeRootItem(0))
	{}


	LuaItemModel::~LuaItemModel()
	{
		delete m_rootItem;
	}


	QVariant LuaItemModel::data(const QModelIndex &index, int role) const
	{
		if (!index.isValid())
			return QVariant();

		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		return item->getData(index.column(), role);
	}


	bool LuaItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid())
			return false;

		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		return item->setData(index.column(), role, value);
	}


	Qt::ItemFlags LuaItemModel::flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return 0;

		switch (index.column()) {
			case COLUMN_TYPE:
				return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
			case COLUMN_NAME:
				return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
			case COLUMN_VALUE:
				return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		}

		return 0;
	}


	QVariant LuaItemModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return m_rootItem->getData(section, role);

		return QVariant();
	}


	QModelIndex LuaItemModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		TreeItem* parentItem;

		if (!parent.isValid())
			parentItem = m_rootItem;
		else
			parentItem = static_cast<TreeItem*>(parent.internalPointer());

		TreeItem* childItem = parentItem->getChild(row);
		if (childItem)
			return createIndex(row, column, childItem);
		else
			return QModelIndex();
	}


	QModelIndex LuaItemModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid())
			return QModelIndex();

		TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
		TreeItem* parentItem = childItem->getParent();

		if (parentItem == m_rootItem)
			return QModelIndex();

		return createIndex(parentItem->getRow(), 0, parentItem);
	}


	int LuaItemModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
	{
		TreeItem* parentItem;
		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			parentItem = m_rootItem;
		else
			parentItem = static_cast<TreeItem*>(parent.internalPointer());

		return parentItem->getChildCount();
	}


	int LuaItemModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
	{
		return 3;
	}


	void LuaItemModel::setData(sol::state* luaVmState, LuaTreeItem::ModelStyle modelStyle)
	{
		beginResetModel();

		{
			std::lock_guard<std::recursive_mutex> lock(lua::globalInterpreterLock);

			delete m_rootItem;
			m_rootItem = new LuaTreeRootItem();

			if (luaVmState)
				new LuaTreeItemTable(modelStyle, false, luaVmState->globals(), "[Global Variables]", sol::type::table, m_rootItem);
		
			//sol::object mt = luaVmState->registry()["sol.cts::core::Network.user"];
			//bool v = mt.valid();
			//auto t = mt.get_type();
			//auto ptr = mt.as< sol::light<sol::usertype_metatable_core> >();
			//auto t = typeid(*ptr).name();
			//sol::usertype_metatable_core* umc = static_cast<sol::usertype_metatable_core*>(ptr);
		}

		endResetModel();
	}

}
}
