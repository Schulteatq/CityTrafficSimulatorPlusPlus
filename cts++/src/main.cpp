#include <cts-core/network/network.h>
#include <gui/networkrenderwidget.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

int main(int argc, char** argv)
{
	cts::core::Network network;
	network.importLegacyXml("minimal.xml");
	//network.importLegacyXml("network.xml");

	QApplication app(argc, argv);
	QMainWindow mw;

	cts::gui::NetworkRenderWidget renderWidget(nullptr);
	renderWidget.setNetwork(&network);
	renderWidget.setMinimumSize(800, 600);
	mw.setCentralWidget(&renderWidget);
	mw.show();

	return app.exec();
}
