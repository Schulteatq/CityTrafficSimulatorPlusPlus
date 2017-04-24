#include <cts-gui/mainwindow.h>
#include <cts-gui/networkrenderwidget.h>
#include <cts-gui/scripting/luatablewidget.h>
#include <cts-gui/scripting/scriptingwidget.h>

#include <QtWidgets/QDockWidget>

#ifdef CTS_ENABLE_SCRIPTING
#include <sol.hpp>
#endif // CTS_ENABLE_SCRIPTING


namespace cts { namespace gui
{

	MainWindow::MainWindow(sol::state* luaState, QWidget* parent)
		: QMainWindow(parent)
		, m_lttw(nullptr)
		, m_luaState(luaState)
		, m_nrw(nullptr)
		, m_sw(nullptr)
	{
		setupGUI();
	}


	NetworkRenderWidget* MainWindow::getNetworkRenderWidget()
	{
		return m_nrw;
	}


	void MainWindow::setupGUI()
	{
		m_nrw = new NetworkRenderWidget(this);
		this->setCentralWidget(m_nrw);

#ifdef CTS_ENABLE_SCRIPTING
		if (m_luaState != nullptr)
		{
			m_sw = new ScriptingWidget(*m_luaState, this);
			QDockWidget* swDock = new QDockWidget(tr("Scripting Console"), this);
			swDock->setAllowedAreas(Qt::AllDockWidgetAreas);
			swDock->setWidget(m_sw);
			addDockWidget(Qt::BottomDockWidgetArea, swDock);

			m_lttw = new LuaTableTreeWidget(m_luaState, LuaTreeItem::FullModel, this);
			QDockWidget* lttwDock = new QDockWidget(tr("Lua VM Inspector"), this);
			lttwDock->setAllowedAreas(Qt::AllDockWidgetAreas);
			lttwDock->setWidget(m_lttw);
			addDockWidget(Qt::RightDockWidgetArea, lttwDock);

			connect(m_sw, &ScriptingWidget::commandExecuted, m_lttw, &LuaTableTreeWidget::update);
		}
#endif // CTS_ENABLE_SCRIPTING
	}


}
}
