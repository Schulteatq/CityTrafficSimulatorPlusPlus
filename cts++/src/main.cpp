#include <cts-core/base/log.h>
#include <cts-core/network/network.h>
#include <gui/networkrenderwidget.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

int main(int argc, char** argv)
{
	auto cs = std::make_unique<cts::core::ConsoleLogger>(false, true);
	cts::core::LogManager::get().addLogger(std::move(cs));

	cts::core::Network network;
	//network.importLegacyXml("minimal.xml");
	network.importLegacyXml("network.xml");

	QApplication app(argc, argv);
	QMainWindow mw;

	cts::gui::NetworkRenderWidget renderWidget(nullptr);
	renderWidget.setNetwork(&network);
	renderWidget.setMinimumSize(800, 600);
	mw.setCentralWidget(&renderWidget);
	mw.show();

	return app.exec();
}
