#ifndef CTS_NETWORKRENDERWIDGET_H__
#define CTS_NETWORKRENDERWIDGET_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>

#include <memory>
#include <vector>
#include <QtWidgets/QWidget>

namespace cts { 
namespace core
{
	class Network;
	class Node;
	class Routing;
}

namespace gui
{

	class NetworkRenderWidget : public QWidget
	{
		Q_OBJECT;
	public:

		NetworkRenderWidget(QWidget* parent);
		virtual ~NetworkRenderWidget();

		/// Returns the network to display.
		core::Network* getNetwork() const;
		/// Sets the network to display to \e value.
		void setNetwork(core::Network* value);

	protected:
		enum class InteractionMode
		{
			None,
			MoveCanvas
		};

		vec2 windowToWorld(const QPoint& pt) const;

		virtual void mouseMoveEvent(QMouseEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent(QMouseEvent* e) override;
		virtual void wheelEvent(QWheelEvent* e) override;

		virtual void paintEvent(QPaintEvent* e) override;



		core::Network* m_network;

		core::Node* m_selectedStartNode;
		core::Node* m_selectedEndNode;
		std::unique_ptr<core::Routing> m_routing;

		double m_zoomFactor;
		vec2 m_zoomOffset;

		InteractionMode m_interactionmode;
		vec2 m_mouseDownPosition;
		vec2 m_mouseDownZoomOffset;
	};

}
}

#endif
