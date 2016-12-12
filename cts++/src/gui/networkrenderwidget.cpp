#include <gui/networkrenderwidget.h>

#include <cts-core/network/connection.h>
#include <cts-core/network/network.h>
#include <cts-core/network/node.h>
#include <cts-core/network/routing.h>
#include <cts-core/traffic/vehicle.h>

#include <vector>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

namespace cts { namespace gui
{

	NetworkRenderWidget::NetworkRenderWidget(QWidget* parent)
		: QWidget(parent)
		, m_network(nullptr)
		, m_selectedStartNode(nullptr)
		, m_selectedEndNode(nullptr)
		, m_routing(nullptr)
		, m_zoomFactor(1.0)
		, m_zoomOffset(0.0, 0.0)
		, m_interactionmode(InteractionMode::None)
		, m_mouseDownPosition(0.0, 0.0)
	{

	}


	NetworkRenderWidget::~NetworkRenderWidget()
	{

	}


	core::Network* NetworkRenderWidget::getNetwork() const
	{
		return m_network;
	}


	void NetworkRenderWidget::setNetwork(core::Network* value)
	{
		m_network = value;
	}


	vec2 NetworkRenderWidget::windowToWorld(const QPoint& pt) const
	{
		vec2 toReturn(pt.x(), pt.y());
		toReturn -= m_zoomOffset;
		toReturn /= m_zoomFactor;
		return toReturn;
	}


	void NetworkRenderWidget::mouseMoveEvent(QMouseEvent* e)
	{
		if (m_interactionmode == InteractionMode::MoveCanvas)
		{
			m_zoomOffset = m_mouseDownZoomOffset + (vec2(e->pos().x(), e->pos().y()) - m_mouseDownPosition);
			e->accept();
			update();
		}
	}


	void NetworkRenderWidget::mousePressEvent(QMouseEvent* e)
	{
		if (m_interactionmode != InteractionMode::None)
			return;

		if (e->button() == Qt::LeftButton)
		{
			vec2 worldCoords = windowToWorld(e->pos());
			for (auto& node : m_network->getNodes())
			{
				if (math::distance(worldCoords, node->getPosition()) < 8)
				{
					if (e->modifiers() == Qt::CTRL)
						m_selectedEndNode = node.get();
					else
						m_selectedStartNode = node.get();

					if (m_selectedStartNode && m_selectedEndNode)
					{
						m_routing = std::make_unique<core::Routing>(*m_selectedStartNode, *m_selectedEndNode, core::TypedVehicle<core::IdmMobil>());
					}

					e->accept();
					update();
				}
			}

		}
		else if (e->button() == Qt::RightButton)
		{
			m_interactionmode = InteractionMode::MoveCanvas;
			m_mouseDownPosition = vec2(e->pos().x(), e->pos().y());
			m_mouseDownZoomOffset = m_zoomOffset;
			e->accept();
		}
	}


	void NetworkRenderWidget::mouseReleaseEvent(QMouseEvent* e)
	{
		if (m_interactionmode != InteractionMode::None)
		{
			m_interactionmode = InteractionMode::None;
			e->accept();
		}
	}


	void NetworkRenderWidget::wheelEvent(QWheelEvent* e)
	{
		if (m_interactionmode != InteractionMode::None)
			return;

		if (e->modifiers() & Qt::ControlModifier)
		{
			vec2 mousePosition(e->pos().x(), e->pos().y());
			vec2 worldPosition = windowToWorld(e->pos());
			m_zoomFactor *= 1.0 + (double(e->delta()) / 1200);
			m_zoomOffset = mousePosition - (worldPosition * m_zoomFactor);

			e->accept();
			update();
		}
	}


	void NetworkRenderWidget::paintEvent(QPaintEvent* e)
	{
		if (m_network == nullptr)
			return;

		QPainter p(this);
		p.translate(m_zoomOffset.x(), m_zoomOffset.y());
		p.scale(m_zoomFactor, m_zoomFactor);

		// draw connections
		QBrush connectionBrush(Qt::gray);
		for (auto& connection : m_network->getConnections())
		{
			QPainterPath path(QPointF(connection->getParameterization().getSupportPoints()[0].x(), connection->getParameterization().getSupportPoints()[0].y()));
			path.cubicTo(
				connection->getParameterization().getSupportPoints()[1].x(), connection->getParameterization().getSupportPoints()[1].y(),
				connection->getParameterization().getSupportPoints()[2].x(), connection->getParameterization().getSupportPoints()[2].y(),
				connection->getParameterization().getSupportPoints()[3].x(), connection->getParameterization().getSupportPoints()[3].y()
				);
			p.setPen(QPen(connectionBrush, connection->getPriority()));
			p.drawPath(path);
		}

		// draw focus nodes
		if (m_selectedStartNode)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(0.0, 0.5, 0.0)), 3));
			p.drawEllipse(m_selectedStartNode->getPosition().x() - 8, m_selectedStartNode->getPosition().y() - 8, 16, 16);
		}
		if (m_selectedEndNode)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(0.5, 0.0, 0.0)), 3));
			p.drawEllipse(m_selectedEndNode->getPosition().x() - 8, m_selectedEndNode->getPosition().y() - 8, 16, 16);
		}

		if (m_routing)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(1.0, 0.5, 0.0)), 10));
			for (auto& segment : m_routing->getSegments())
			{
				auto connection = segment.startNode->getConnectionTo(*segment.targetNode);
				QPainterPath path(QPointF(connection->getParameterization().getSupportPoints()[0].x(), connection->getParameterization().getSupportPoints()[0].y()));
				path.cubicTo(
					connection->getParameterization().getSupportPoints()[1].x(), connection->getParameterization().getSupportPoints()[1].y(),
					connection->getParameterization().getSupportPoints()[2].x(), connection->getParameterization().getSupportPoints()[2].y(),
					connection->getParameterization().getSupportPoints()[3].x(), connection->getParameterization().getSupportPoints()[3].y()
					);

				p.drawPath(path);
			}
		}

		// draw nodes
		p.setPen(Qt::NoPen);
		p.setBrush(Qt::black);
		for (auto& node : m_network->getNodes())
		{
			p.drawRect(node->getPosition().x() - 4, node->getPosition().y() - 4, 8, 8);
		}

		auto toQt = [](const vec2& v)
		{
			return QPointF(v[0], v[1]);
		};

		// draw Intersections
		p.setPen(Qt::darkMagenta);
		p.setBrush(Qt::NoBrush);
		for (auto& intersection : m_network->getIntersections())
		{
			const core::BezierParameterization& ap = intersection.getFirstConnection().getParameterization();
			const core::BezierParameterization& bp = intersection.getSecondConnection().getParameterization();
			QPointF surroundingPoints[4]
			{
				toQt(ap.arcPositionToCoordinate(intersection.getFirstArcPosition() + intersection.getWaitingDistance())),
				toQt(bp.arcPositionToCoordinate(intersection.getSecondArcPosition() + intersection.getWaitingDistance())),
				toQt(ap.arcPositionToCoordinate(intersection.getFirstArcPosition() - intersection.getWaitingDistance())),
				toQt(bp.arcPositionToCoordinate(intersection.getSecondArcPosition() - intersection.getWaitingDistance()))
			};

			p.drawPolyline(surroundingPoints, 4);
		}
	}


}
}
