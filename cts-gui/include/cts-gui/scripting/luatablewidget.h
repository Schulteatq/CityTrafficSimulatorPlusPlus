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
		explicit LuaTableTreeWidget(sol::state* luaState, LuaTreeItem::ModelStyle modelStyle, QWidget* parent = nullptr);

		/// Default Destructor
		virtual ~LuaTableTreeWidget() = default;

		/// Returns the data model for the TreeView.
		LuaItemModel* getTreeModel();

		/// Updates the data in the tree view using the given Lua state.
		void setLuaState(sol::state* luaState, LuaTreeItem::ModelStyle modelStyle);

	public slots:
		/// Reinitializes the LuaItemModel with the data from the Lua VM.
		void update();

	private:
		/// Sets up the widget.
		void setupWidget();

		sol::state* m_luaState;					///< Lua VM to use
		LuaTreeItem::ModelStyle m_modelStyle;	///< Model style to use
		LuaItemModel* m_treeModel;				///< Data model for the TreeView.
		QSortFilterProxyModel* m_sortModel;		///< sorting proxy model

	};
}
}

#endif
