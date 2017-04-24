#ifndef CTS_GUI_MAINWINDOW_H__
#define CTS_GUI_MAINWINDOW_H__

#include <cts-gui/config.h>

#include <QtWidgets/QMainWindow>

namespace sol
{
	class state;
}

namespace cts { namespace gui
{
	class LuaTableTreeWidget;
	class NetworkRenderWidget;
	class ScriptingWidget;

	class CTS_GUI_API MainWindow : public QMainWindow
	{
		Q_OBJECT;

	public:
		MainWindow(sol::state* luaState = nullptr, QWidget* parent = nullptr);

		NetworkRenderWidget* getNetworkRenderWidget();

	private:
		void setupGUI();

		NetworkRenderWidget* m_nrw;
		sol::state* m_luaState;

#ifdef CTS_ENABLE_SCRIPTING
		LuaTableTreeWidget* m_lttw;
		ScriptingWidget* m_sw;
#endif // CTS_ENABLE_SCRIPTING
	};

}
}

#endif
