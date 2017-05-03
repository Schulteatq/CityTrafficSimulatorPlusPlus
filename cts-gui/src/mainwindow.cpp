#include <cts-gui/mainwindow.h>
#include <cts-gui/networkrenderwidget.h>

#include <QtWidgets/QDockWidget>

namespace cts { namespace gui
{

	MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
		, m_nrw(nullptr)
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
	}


}
}
