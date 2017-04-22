#ifndef CTS_NETWORKRENDERWIDGET_H__
#define CTS_NETWORKRENDERWIDGET_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>
#include <cts-core/base/signal.h>
#include <cts-gui/config.h>

#include <memory>
#include <vector>
#include <QtWidgets/QWidget>

namespace cts { 
namespace core
{
	class Network;
	class Node;
	class Routing;
	class Simulation;
}

namespace gui
{

	class CTS_GUI_API NetworkRenderWidget : public QWidget, public core::SignalReceiver
	{
		Q_OBJECT;
	public:

		NetworkRenderWidget(QWidget* parent);
		virtual ~NetworkRenderWidget();

		/// Returns the network to display.
		core::Network* getNetwork() const;
		/// Sets the network to display to \e value.
		void setNetwork(core::Network* value);

		/// Returns the the Simulation object to use.
		core::Simulation* getSimulation() const;
		/// Sets the the Simulation object to use.
		void setSimulation(core::Simulation* value);

		core::Simulation& getSimulation();

	signals:
		void updateRequested();

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

		virtual void closeEvent(QCloseEvent* e) override;

		QPainterPath roundedConvexHullPath(const std::vector<vec2>& points, double radius) const;


		void onSimulationStep();


		core::Network* m_network;
		core::Simulation* m_simulation;

		std::vector<core::Node*> m_selectedStartNodes;
		std::vector<core::Node*> m_selectedEndNodes;
		std::unique_ptr<core::Routing> m_routing;

		double m_zoomFactor;
		vec2 m_zoomOffset;

		InteractionMode m_interactionmode;
		vec2 m_mouseDownPosition;
		vec2 m_mousePosition;
		std::vector<NodeSelection> m_selectedNodes;
		bool m_drawDebugInfo;
	};

}
}

#endif
