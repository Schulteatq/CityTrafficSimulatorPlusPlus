#include <cts-gui/scripting/luatablewidget.h>

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

	namespace {
		const int COLUMN_NAME = 0;
		const int COLUMN_TYPE = 1;
		const int COLUMN_VALUE = 2;

	}

// = TreeModel items ==============================================================================

	LuaTreeItem::LuaTreeItem(ModelStyle modelStyle, const std::string& name, sol::type type, TreeItem* parent /*= nullptr*/)
		: TreeItem(parent)
		, _name(name)
		, _type(type) 
		, _modelStyle(modelStyle)
	{
	}

	QVariant LuaTreeItem::getData(int column, int role) const {
		switch (role) {
			case Qt::EditRole: // fallthrough
			case Qt::DisplayRole:
				if (column == COLUMN_NAME)
					return QVariant(QString::fromStdString(_name));
				else if (column == COLUMN_TYPE)
					return QVariant(QString::fromStdString(lua_typename(0, int(_type))));
				else if (column == COLUMN_VALUE)
					return getValue();
				else
					return QVariant();
			default:
				return QVariant();
		}
	}

	QString LuaTreeItem::getValue() const {
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

	LuaTreeRootItem::~LuaTreeRootItem() {
	}

// ================================================================================================

	LuaTreeItemLeaf::LuaTreeItemLeaf(ModelStyle modelStyle, sol::table parentTable, const std::string& name, sol::type type, TreeItem* parent)
		: LuaTreeItem(modelStyle, name, type, parent)
		, _parentTable(parentTable)
	{
		if (parentTable[_name][sol::metatable_key].valid())
		{
			new LuaTreeItemTable(_modelStyle, true, parentTable[_name][sol::metatable_key], name, sol::type::table, this);
		}
	}

	LuaTreeItemLeaf::~LuaTreeItemLeaf() {
	}
	
	QString LuaTreeItemLeaf::getValue() const {
		if (_type == sol::type::string || _type == sol::type::number || _type == sol::type::boolean)
		{
			return QString::fromStdString(_parentTable[_name].get<std::string>());
		}
		else if (_type == sol::type::userdata)
		{
			return "<userdata>";
		}
		return "";
	}

// ================================================================================================

	LuaTreeItemTable::LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, sol::table thisTable, const std::string& name, sol::type type, TreeItem* parent)
		: LuaTreeItem(modelStyle, name, type, parent)
		, _thisTable(thisTable)
		, _isMetatable(isMetatable)
		, _dataPtr(getTablePtr(thisTable))
	{
		// create a new metatable
		if (_thisTable[sol::metatable_key].valid())
			new LuaTreeItemTable(_modelStyle, true, _thisTable[sol::metatable_key], name, sol::type::table, this);

		// fill the table with values depending on model style
		if (_modelStyle == FULL_MODEL)
		{
			_thisTable.for_each([this](sol::object key, sol::object value) {
				std::string itemName = key.as<std::string>();
				sol::type luaType = value.get_type();

				if (itemName == "_G")
					return;

				if (luaType == sol::type::table)
				{
					sol::table thatTable = _thisTable[itemName];
					void* thatRawPtr = getTablePtr(thatTable);

					// detect cyclic table references
					LuaTreeItemTable* pTable = this;
					while (pTable = dynamic_cast<LuaTreeItemTable*>(pTable->_parent))
					{
						if (pTable->_dataPtr == thatRawPtr)
						{
							new LuaTreeItemLeaf(_modelStyle, _thisTable, "\xF0\x9F\x94\x97 " + itemName, luaType, this);
							return;
						}
					}

					new LuaTreeItemTable(_modelStyle, false, _thisTable[itemName], itemName, luaType, this);
				}
				else
				{
					new LuaTreeItemLeaf(_modelStyle, _thisTable, itemName, luaType, this);
				}
			});
		}
		else if (_modelStyle == COMPLETER_MODEL) {
			//auto& valueMap = thisTable->getValueMap();
			//if (! _isMetatable) {
			//	// for regular tables, just explore the whole table
			//	for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
			//		const std::string& itemName = it->first;
			//		int luaType = it->second.luaType;
			//
			//		if (itemName == "_G")
			//			continue;
			//
			//		if (luaType == LUA_TTABLE) 
			//			new LuaTreeItemTable(_modelStyle, false, thisTable->getTable(itemName), itemName, luaType, this);
			//		else 
			//			new LuaTreeItemLeaf(_modelStyle, thisTable, itemName, luaType, this);
			//	}
			//}
			//else {
			//	// for metatables, just gather all instance methods
			//	auto fnTable = thisTable->getTable(".fn");
			//	if (fnTable) {
			//		recursiveGatherSwigMethods(thisTable, this);
			//	}
			//
			//	auto instanceTable = thisTable->getTable(".instance");
			//	if (instanceTable) {
			//		recursiveGatherSwigMethods(instanceTable, this);
			//	}
			//}
		}
	}

	LuaTreeItemTable::~LuaTreeItemTable() {

	}

	QVariant LuaTreeItemTable::getData(int column, int role) const {
		if (_isMetatable && column == COLUMN_NAME && (role == Qt::EditRole || role == Qt::DisplayRole)) {
			switch (_modelStyle) {
				case FULL_MODEL:
					return QVariant("[Metatable]");
				case COMPLETER_MODEL:
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

	LuaTableTreeModel::LuaTableTreeModel(QObject *parent /*= 0*/)
		: QAbstractItemModel(parent)
		, _rootItem(new LuaTreeRootItem(0))
	{
	}

	LuaTableTreeModel::~LuaTableTreeModel() {
		delete _rootItem;
	}

	QVariant LuaTableTreeModel::data(const QModelIndex &index, int role) const {
		if (!index.isValid())
			return QVariant();

		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		return item->getData(index.column(), role);
	}

	bool LuaTableTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
		if (!index.isValid())
			return false;

		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		return item->setData(index.column(), role, value);
	}

	Qt::ItemFlags LuaTableTreeModel::flags(const QModelIndex &index) const {
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

	QVariant LuaTableTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return _rootItem->getData(section, role);

		return QVariant();
	}

	QModelIndex LuaTableTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const {
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		TreeItem* parentItem;

		if (!parent.isValid())
			parentItem = _rootItem;
		else
			parentItem = static_cast<TreeItem*>(parent.internalPointer());

		TreeItem* childItem = parentItem->getChild(row);
		if (childItem)
			return createIndex(row, column, childItem);
		else
			return QModelIndex();
	}

	QModelIndex LuaTableTreeModel::parent(const QModelIndex &index) const {
		if (!index.isValid())
			return QModelIndex();

		TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
		TreeItem* parentItem = childItem->getParent();

		if (parentItem == _rootItem)
			return QModelIndex();

		return createIndex(parentItem->getRow(), 0, parentItem);
	}

	int LuaTableTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const {
		TreeItem* parentItem;
		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			parentItem = _rootItem;
		else
			parentItem = static_cast<TreeItem*>(parent.internalPointer());

		return parentItem->getChildCount();
	}

	int LuaTableTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const {
		return 3;
	}

	void LuaTableTreeModel::setData(sol::state* luaVmState, LuaTreeItem::ModelStyle modelStyle) {
		beginResetModel();

		_itemMap.clear();
		delete _rootItem;
		_rootItem = new LuaTreeRootItem();

		if (luaVmState)
			new LuaTreeItemTable(modelStyle, false, luaVmState->globals(), "[Global Variables]", sol::type::table, _rootItem);

		endResetModel();
	}
	
// = LuaTableTreeWidget ===========================================================================

	LuaTableTreeWidget::LuaTableTreeWidget(QWidget* parent /*= 0*/)
		: QTreeView(parent)
		, _treeModel(nullptr)
		, _sortModel(nullptr)
	{
		setupWidget();
	}

	LuaTableTreeWidget::~LuaTableTreeWidget() {

	}

	void LuaTableTreeWidget::update(sol::state& luaVmState, LuaTreeItem::ModelStyle modelStyle) {
		// clear selection before setting the new data or we will encounter random crashes...
		selectionModel()->clear();

		// set new data
		_treeModel->setData(&luaVmState, modelStyle);
		_sortModel->sort(0);
		expandToDepth(0);

		// adjust view
		resizeColumnToContents(0);
		resizeColumnToContents(1);
		resizeColumnToContents(2);
	}

	void LuaTableTreeWidget::setupWidget() {
		_treeModel = new LuaTableTreeModel(this);

		_sortModel = new QSortFilterProxyModel(this);
		_sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		_sortModel->setSortLocaleAware(true);
		_sortModel->setSourceModel(_treeModel);

		setModel(_sortModel);
		setSortingEnabled(true);
	}

	LuaTableTreeModel* LuaTableTreeWidget::getTreeModel() {
		return _treeModel;
	}

}
}
