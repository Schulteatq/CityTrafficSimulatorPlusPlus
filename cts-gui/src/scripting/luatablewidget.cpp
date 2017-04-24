#include <cts-gui/scripting/luatablewidget.h>

#include <cassert>

namespace cts { namespace gui {

	LuaTableTreeWidget::LuaTableTreeWidget(sol::state* luaState, LuaTreeItem::ModelStyle modelStyle, QWidget* parent /*= 0*/)
		: QTreeView(parent)
		, m_luaState(luaState)
		, m_modelStyle(modelStyle)
		, m_treeModel(nullptr)
		, m_sortModel(nullptr)
	{
		setupWidget();
		setLuaState(m_luaState, m_modelStyle);
	}


	void LuaTableTreeWidget::setLuaState(sol::state* luaState, LuaTreeItem::ModelStyle modelStyle)
	{
		m_luaState = luaState;
		m_modelStyle = modelStyle;
		update();
	}


	void LuaTableTreeWidget::update()
	{
		// clear selection before setting the new data or we will encounter random crashes...
		selectionModel()->clear();

		// set new data
		m_treeModel->setData(m_luaState, m_modelStyle);
		m_sortModel->sort(0);
		expandToDepth(0);

		// adjust view
		resizeColumnToContents(0);
		resizeColumnToContents(1);
		resizeColumnToContents(2);
	}


	void LuaTableTreeWidget::setupWidget()
	{
		m_treeModel = new LuaItemModel(this);

		m_sortModel = new QSortFilterProxyModel(this);
		m_sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		m_sortModel->setSortLocaleAware(true);
		m_sortModel->setSourceModel(m_treeModel);

		setModel(m_sortModel);
		setSortingEnabled(true);
	}


	LuaItemModel* LuaTableTreeWidget::getTreeModel()
	{
		return m_treeModel;
	}

}
}
