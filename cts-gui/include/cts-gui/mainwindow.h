#ifndef CTS_GUI_MAINWINDOW_H__
#define CTS_GUI_MAINWINDOW_H__

#include <cts-gui/config.h>

#include <QtWidgets/QMainWindow>

namespace cts { namespace gui
{
	class LuaTableTreeWidget;
	class NetworkRenderWidget;
	class ScriptingWidget;

	class CTS_GUI_API MainWindow : public QMainWindow
	{
		Q_OBJECT;

	public:
		MainWindow(QWidget* parent = nullptr);

		NetworkRenderWidget* getNetworkRenderWidget();

	private:
		void setupGUI();

		NetworkRenderWidget* m_nrw;
	};

}
}

#endif
