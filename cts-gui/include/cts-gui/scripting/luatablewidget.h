#ifndef CTS_GUI_LUATABLEWIDGET_H__
#define CTS_GUI_LUATABLEWIDGET_H__

#include <cts-gui/config.h>
#include <cts-gui/tools/luaitemmodel.h>

#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QTreeWidget>

namespace cts { namespace gui {

	/// Qt widget for showing a list of pipelines and their processors in a QTreeView.
	class CTS_GUI_API LuaTableTreeWidget : public QTreeView {
		Q_OBJECT;

	public:
		/// Creates a new LuaTableTreeWidget.
		/// \param   parent  Parent widget
		explicit LuaTableTreeWidget(QWidget* parent = 0);

		/// Default Destructor
		virtual ~LuaTableTreeWidget() = default;

		/// Returns the data model for the TreeView.
		LuaItemModel* getTreeModel();


	public slots:
		/// Updates the data in the tree view using the given Lua state.
		void update(sol::state& luaState, LuaTreeItem::ModelStyle modelStyle);

	private:
		/// Sets up the widget.
		void setupWidget();

		LuaItemModel* m_treeModel;		///< Data model for the TreeView.
		QSortFilterProxyModel* m_sortModel;	///< sorting proxy model

	};
}
}

#endif
