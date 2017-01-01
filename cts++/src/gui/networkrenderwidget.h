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
			None,				///< No interaction.
			MoveCanvas,			///< Moving the canvas (i.e. panning the view).
			MoveNode,			///< Moving one or multiple nodes.
			MoveInSlope,		///< Move the incoming slope of one or multiple nodes.
			MoveOutSlope,		///< Move the outgoing slope of one or multiple nodes.
			CreateNode,			///< Interaction directly after adding one or multiple nodes (combined move node and move out slope).
			DragRubberband		///< Drag a rubber band to perform node selection on release.
		};

		struct NodeSelection
		{
			core::Node* node;
			vec2 originalPosition;
		};


		vec2 windowToWorld(const QPoint& pt) const;

		virtual void mouseMoveEvent(QMouseEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent(QMouseEvent* e) override;
		virtual void wheelEvent(QWheelEvent* e) override;

		virtual void keyPressEvent(QKeyEvent* e) override;

		virtual void paintEvent(QPaintEvent* e) override;


		QPainterPath roundedConvexHullPath(const std::vector<vec2>& points, double radius) const;

		core::Network* m_network;

		core::Node* m_selectedStartNode;
		core::Node* m_selectedEndNode;
		std::unique_ptr<core::Routing> m_routing;

		double m_zoomFactor;
		vec2 m_zoomOffset;

		InteractionMode m_interactionmode;
		vec2 m_mouseDownPosition;
		vec2 m_mousePosition;
		std::vector<NodeSelection> m_selectedNodes;
	};

}
}

#endif
