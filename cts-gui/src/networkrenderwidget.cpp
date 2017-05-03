#include <cts-gui/networkrenderwidget.h>

#include <cts-core/base/algorithmicgeometry.h>
#include <cts-core/base/bounds.h>
#include <cts-core/base/log.h>
#include <cts-core/network/bezierparameterization.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/network.h>
#include <cts-core/network/node.h>
#include <cts-core/network/routing.h>
#include <cts-core/simulation/simulation.h>
#include <cts-core/traffic/vehicle.h>

#include <cmath>
#include <iostream>
#include <vector>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

namespace cts { namespace gui
{
	namespace
	{
		QPointF toQt(const vec2& v)
		{
			return QPointF(v[0], v[1]);
		}
	}


	NetworkRenderWidget::NetworkRenderWidget(QWidget* parent)
		: QWidget(parent)
		, m_network(nullptr)
		, m_simulation(nullptr)
		, m_routing(new core::Routing())
		, m_zoomFactor(1.0)
		, m_zoomOffset(0.0, 0.0)
		, m_interactionmode(InteractionMode::None)
		, m_mouseDownPosition(0.0, 0.0)
		, m_drawDebugInfo(false)
	{
		setFocusPolicy(Qt::StrongFocus);
		connect(this, SIGNAL(updateRequested()), this, SLOT(update()));
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


	core::Simulation* NetworkRenderWidget::getSimulation() const
	{
		return m_simulation;
	}


	void NetworkRenderWidget::setSimulation(core::Simulation* value)
	{
		if (m_simulation != value)
		{
			if (m_simulation != nullptr)
				m_simulation->s_stepped.disconnect(this);

			m_simulation = value;

			if (m_simulation != nullptr)
				m_simulation->s_stepped.connect(this, &NetworkRenderWidget::onSimulationStep);
		}
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
		m_mousePosition = windowToWorld(e->pos());
		const auto offset = m_mousePosition - m_mouseDownPosition;

		switch (m_interactionmode)
		{
		case InteractionMode::None:
			return;
		case InteractionMode::MoveCanvas:
			LOG_DEBUG("gui.NetworkRenderWidget", "[" << e->pos().x() << ", " << e->pos().y() << "], " << m_zoomOffset << ", " << offset << "\n");
			m_zoomOffset += offset;
			break;
		case InteractionMode::MoveNode:
			for (auto& ns : m_selectedNodes)
			{
				ns.node->setPosition(ns.originalPosition + offset);
			}
			break;
		case InteractionMode::MoveInSlope:
			for (auto& ns : m_selectedNodes)
			{
				ns.node->setInSlope(ns.originalPosition - offset);
				if ((e->modifiers() & Qt::AltModifier) == 0)
					ns.node->setOutSlope(ns.originalPosition - offset);
			}
			break;
		case InteractionMode::MoveOutSlope:
			for (auto& ns : m_selectedNodes)
			{
				ns.node->setOutSlope(ns.originalPosition + offset);
				if ((e->modifiers() & Qt::AltModifier) == 0)
					ns.node->setInSlope(ns.originalPosition + offset);
			}
			break;
		case InteractionMode::CreateNode:
		{
			core::Node* base = m_selectedNodes[0].node;
			base->setOutSlope(m_mousePosition - base->getPosition());
			base->setInSlope(base->getOutSlope());

			for (size_t i = 1; i < m_selectedNodes.size(); ++i)
			{
				core::Node* cur = m_selectedNodes[i].node;

				double rotation = std::atan2(m_selectedNodes[0].originalPosition[1], m_selectedNodes[0].originalPosition[0]) - std::atan2(m_selectedNodes[i].originalPosition[1], m_selectedNodes[i].originalPosition[0]);
				cur->setPosition(base->getPosition() - (math::rotated(base->getOutSlope(), rotation).normalized() * m_selectedNodes[i].originalPosition.norm()));

				double baseDistance = math::distance(base->getPosition(), base->getIncomingConnections().front()->getStartNode().getPosition());
				double curDistance = math::distance(cur->getPosition(), cur->getIncomingConnections().front()->getStartNode().getPosition());
				double stretch = std::pow(curDistance / baseDistance, 2.0);
				cur->setInSlope(base->getInSlope() * stretch);
				cur->setOutSlope(base->getOutSlope() * stretch);
			}
			break;
		}
		case InteractionMode::DragRubberband:
			// nothing to do here
			break;
		}

		e->accept();
		update();
	}


	void NetworkRenderWidget::mousePressEvent(QMouseEvent* e)
	{
		if (m_interactionmode != InteractionMode::None)
			return;

		m_mouseDownPosition = windowToWorld(e->pos());
		m_mousePosition = m_mouseDownPosition;
		if (e->button() == Qt::LeftButton)
		{
			// Select nodes/slope manipulators or start drawing a rubber band for selection.
			if (e->modifiers() == Qt::NoModifier)
			{
				// check whether we clicked on one of the selected nodes
				for (auto it = m_selectedNodes.begin(); it != m_selectedNodes.end(); ++it)
				{
					// check whether we clicked on the out slope manipulator
					if ((m_mouseDownPosition - it->node->getPosition() - it->node->getOutSlope()).cwiseAbs().maxCoeff() < 8)
					{
						std::swap(*it, *m_selectedNodes.begin());
						for (auto& ns : m_selectedNodes)
						{
							ns.originalPosition = ns.node->getOutSlope();
						}
						m_interactionmode = InteractionMode::MoveOutSlope;
						break;
					}

					
					// check whether we clicked on the in slope manipulator
					if ((m_mouseDownPosition - it->node->getPosition() + it->node->getInSlope()).cwiseAbs().maxCoeff() < 8)
					{
						std::swap(*it, *m_selectedNodes.begin());
						for (auto& ns : m_selectedNodes)
						{
							ns.originalPosition = ns.node->getInSlope();
						}
						m_interactionmode = InteractionMode::MoveInSlope;
						break;
					}


					// check whether we clicked on the node
					if ((m_mouseDownPosition - it->node->getPosition()).cwiseAbs().maxCoeff() < 8)
					{
						std::swap(*it, *m_selectedNodes.begin());
						for (auto& ns : m_selectedNodes)
						{
							ns.originalPosition = ns.node->getPosition();
						}
						m_interactionmode = InteractionMode::MoveNode;
						break;
					}
				}

				if (m_interactionmode == InteractionMode::None)
				{
					for (auto& node : m_network->getNodes())
					{
						if ((m_mouseDownPosition - node->getPosition()).cwiseAbs().maxCoeff() < 8)
						{
							m_selectedNodes = { { node.get(), m_mouseDownPosition } };
							m_interactionmode = InteractionMode::MoveNode;
						}
					}
				}

				if (m_interactionmode == InteractionMode::None)
					m_interactionmode = InteractionMode::DragRubberband;
			}

			// Connect nodes
			else if (e->modifiers() == Qt::AltModifier)
			{
				if (!m_selectedNodes.empty())
				{
					for (auto& node : m_network->getNodes())
					{
						if ((m_mouseDownPosition - node->getPosition()).cwiseAbs().maxCoeff() < 8)
						{
							for (auto& ns : m_selectedNodes)
							{
								if (ns.node->getConnectionTo(*node) == nullptr)
								{
									auto connection = m_network->addConnection(*ns.node, *node);
									connection->setPriority(5);
								}
							}
						}
					}

				}
			}

			// Add new nodes and connect them to the current selection.
			else if (e->modifiers() == Qt::ControlModifier)
			{
				if (m_selectedNodes.empty())
				{
					auto newNode = m_network->addNode(m_mouseDownPosition);
					m_selectedNodes = { { newNode, vec2(0.0, 0.0) } };
				}
				else
				{
					std::vector<NodeSelection> newNodes;
					for (size_t i = 0; i < m_selectedNodes.size(); ++i)
					{
						const auto offset = m_selectedNodes[i].node->getPosition() - m_selectedNodes[0].node->getPosition();
						core::Node* newNode = m_network->addNode(m_mouseDownPosition + offset);
						newNode->setOutSlope((newNode->getPosition() - m_selectedNodes[i].node->getPosition()).normalized() * 30.0);
						newNode->setInSlope(newNode->getOutSlope() * -1.0);

						auto connection = m_network->addConnection(*m_selectedNodes[i].node, *newNode);
						connection->setPriority(5);
						newNodes.push_back({ newNode, offset });
					}
					m_selectedNodes = newNodes;
				}
				m_interactionmode = InteractionMode::CreateNode;
			}

			e->accept();
			update();
		}

		// move canvas
		else if (e->button() == Qt::RightButton)
		{
			m_interactionmode = InteractionMode::MoveCanvas;
			e->accept();
		}
	}


	void NetworkRenderWidget::mouseReleaseEvent(QMouseEvent* e)
	{
		if (m_interactionmode != InteractionMode::None)
		{
			if (m_interactionmode == InteractionMode::DragRubberband)
			{
				m_selectedNodes.clear();
				const core::Bounds2 bounds{ m_mouseDownPosition, m_mousePosition };
				for (auto& node : m_network->getNodes(bounds))
				{
					m_selectedNodes.push_back({ node, node->getPosition() });
				}
			}

			m_interactionmode = InteractionMode::None;
			e->accept();
			update();
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


	void NetworkRenderWidget::keyPressEvent(QKeyEvent* e)
	{
		switch (e->key())
		{
		case Qt::Key_Delete:
			for (auto& ns : m_selectedNodes)
			{
				m_network->removeNode(*ns.node);
			}
			m_selectedNodes.clear();
			break;
		case Qt::Key_F:
			m_selectedStartNodes.clear();
			for (auto& ns : m_selectedNodes)
			{
				m_selectedStartNodes.push_back(ns.node);
			}
			break;
		case Qt::Key_T:
			m_selectedEndNodes.clear();
			for (auto& ns : m_selectedNodes)
			{
				m_selectedEndNodes.push_back(ns.node);
			}
			break;
		case Qt::Key_D:
			m_drawDebugInfo = !m_drawDebugInfo;
			break;
		case  Qt::Key_Space:
			m_simulation->stop();
			m_simulation->step();
			break;
		}
	}


	void NetworkRenderWidget::paintEvent(QPaintEvent* e)
	{
		if (m_network == nullptr)
			return;

		QPainter p(this);
		p.translate(m_zoomOffset.x(), m_zoomOffset.y());
		p.scale(m_zoomFactor, m_zoomFactor);

		std::lock_guard<std::mutex> lockGuard(m_simulation->getMutex());

		// draw connections
		QBrush connectionBrush(Qt::gray);
		for (auto& connection : m_network->getConnections())
		{
			QPainterPath path(QPointF(connection->getCurve().getSupportPoints()[0].x(), connection->getCurve().getSupportPoints()[0].y()));
			path.cubicTo(
				connection->getCurve().getSupportPoints()[1].x(), connection->getCurve().getSupportPoints()[1].y(),
				connection->getCurve().getSupportPoints()[2].x(), connection->getCurve().getSupportPoints()[2].y(),
				connection->getCurve().getSupportPoints()[3].x(), connection->getCurve().getSupportPoints()[3].y()
				);
			p.setPen(QPen(connectionBrush, connection->getPriority()));
			p.drawPath(path);
		}


		// draw nodes
		p.setPen(Qt::NoPen);
		p.setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 0.5)));
		for (auto& node : m_network->getNodes())
		{
			p.drawRect(node->getPosition().x() - 4, node->getPosition().y() - 4, 8, 8);
		}


		// draw focus nodes
		for (auto node : m_selectedStartNodes)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(0.0, 0.5, 0.0)), 3));
			p.drawEllipse(node->getPosition().x() - 8, node->getPosition().y() - 8, 16, 16);
		}
		for (auto node : m_selectedEndNodes)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(0.5, 0.0, 0.0)), 3));
			p.drawEllipse(node->getPosition().x() - 8, node->getPosition().y() - 8, 16, 16);
		}

		if (!m_selectedNodes.empty())
		{
			// draw in/out slope manipulators 
			p.setPen(Qt::black);
			p.setBrush(Qt::NoBrush);
			for (auto& sn : m_selectedNodes)
			{
				const core::Node& node = *sn.node;
				QPointF inPos = toQt(node.getPosition() - node.getInSlope());
				QPointF outPos = toQt(node.getPosition() + node.getOutSlope());
				p.drawLine(toQt(node.getPosition()), inPos);
				p.drawLine(toQt(node.getPosition()), outPos);
				p.drawEllipse(inPos, 8, 8);
				p.drawRect(outPos.x() - 8, outPos.y() - 8, 16, 16);
			}

			std::vector<vec2> points;
			points.reserve(m_selectedNodes.size());
			for (auto& sn : m_selectedNodes)
			{
				points.push_back(sn.node->getPosition());
			}

			QPainterPath path = roundedConvexHullPath(points, 16);
			p.setPen(QColor::fromRgbF(1.0, 0.75, 0.0));
			p.setBrush(QBrush(QColor::fromRgbF(1.0, 0.75, 0.0, 0.2)));
			p.drawPath(path);
		}

		if (m_routing)
		{
			p.setPen(QPen(QBrush(QColor::fromRgbF(1.0, 0.5, 0.0)), 10));
			for (auto& segment : m_routing->getSegments())
			{
				auto connection = segment.start->getConnectionTo(*segment.destination);
				QPainterPath path(QPointF(connection->getCurve().getSupportPoints()[0].x(), connection->getCurve().getSupportPoints()[0].y()));
				path.cubicTo(
					connection->getCurve().getSupportPoints()[1].x(), connection->getCurve().getSupportPoints()[1].y(),
					connection->getCurve().getSupportPoints()[2].x(), connection->getCurve().getSupportPoints()[2].y(),
					connection->getCurve().getSupportPoints()[3].x(), connection->getCurve().getSupportPoints()[3].y()
					);

				p.drawPath(path);
			}
		}

		// draw Intersections debug information
		if (m_drawDebugInfo)
		{
			p.setPen(Qt::darkMagenta);
			p.setBrush(Qt::NoBrush);
			for (auto& intersection : m_network->getIntersections())
			{
				const core::BezierParameterization& ap = intersection->getFirstConnection().getCurve();
				const core::BezierParameterization& bp = intersection->getSecondConnection().getCurve();
				QPointF surroundingPoints[5]
				{
					toQt(ap.arcPositionToCoordinate(intersection->getFirstArcPosition() + intersection->getWaitingDistance())),
					toQt(bp.arcPositionToCoordinate(intersection->getSecondArcPosition() + intersection->getWaitingDistance())),
					toQt(ap.arcPositionToCoordinate(intersection->getFirstArcPosition() - intersection->getWaitingDistance())),
					toQt(bp.arcPositionToCoordinate(intersection->getSecondArcPosition() - intersection->getWaitingDistance())),
					toQt(ap.arcPositionToCoordinate(intersection->getFirstArcPosition() + intersection->getWaitingDistance()))
				};

				p.drawPolyline(surroundingPoints, 5);

				vec2 center = ap.arcPositionToCoordinate(intersection->getFirstArcPosition());
				for (auto& it : intersection->m_aCrossingVehicles)
				{
					vec2 vpos = it.first->getCurrentConnection()->getCurve().arcPositionToCoordinate(it.first->getCurrentArcPosition());
					if (it.second.willWaitInFront)
						p.setPen(Qt::darkRed);
					else
						p.setPen(Qt::darkGreen);

					p.drawLine(toQt(center), toQt(vpos));
					p.drawText(toQt((center + vpos) / 2.0), tr("d:%1, t: [%2, %3]").arg(it.second.remainingDistance).arg(it.second.blockingTime[0]).arg(it.second.blockingTime[1]));
				}
				for (auto& it : intersection->m_bCrossingVehicles)
				{
					vec2 vpos = it.first->getCurrentConnection()->getCurve().arcPositionToCoordinate(it.first->getCurrentArcPosition());
					if (it.second.willWaitInFront)
						p.setPen(Qt::darkRed);
					else
						p.setPen(Qt::darkGreen);

					p.drawLine(toQt(center), toQt(vpos));
					p.drawText(toQt((center + vpos) / 2.0), tr("d:%1, t: [%2, %3]").arg(it.second.remainingDistance).arg(it.second.blockingTime[0]).arg(it.second.blockingTime[1]));
				}
			}
		}

		// draw vehicles
		p.setPen(Qt::NoPen);
		p.setBrush(QBrush(QColor::fromRgbF(0.0, 0.75, 1.0, 1.0)));
		for (auto& it : m_network->getTrafficManager().getVehicles())
		{
			if (it->getCurrentConnection() == nullptr)
				continue;

			const double time = it->getCurrentConnection()->getCurve().arcPositionToTime(it->getCurrentArcPosition());
			const vec2 pos = it->getCurrentConnection()->getCurve().timeToCoordinate(time);
			const vec2 orientation = it->getCurrentConnection()->getCurve().derivateAtTime(time).normalized();
			const vec2 normal = math::rotatedClockwise(orientation);

			QPointF points[4]
			{
				toQt(pos - 8.0 * normal),
				toQt(pos + 8.0 * normal),
				toQt(pos - it->getLength() * orientation + 8.0 * normal),
				toQt(pos - it->getLength() * orientation - 8.0 * normal)
			};
			p.drawPolygon(points, 4);

			if (m_drawDebugInfo)
			{
				p.setPen(Qt::black);
				p.drawText(toQt(pos), tr("%1").arg(it->debugId));
			}
		}

		// draw rubberband
		if (m_interactionmode == InteractionMode::DragRubberband)
		{
			p.setPen(Qt::darkGray);
			p.setBrush(QBrush(QColor::fromRgbF(0.0, 0.0, 0.0, 0.2)));
			p.drawRect(m_mouseDownPosition[0], m_mouseDownPosition[1], m_mousePosition[0] - m_mouseDownPosition[0], m_mousePosition[1] - m_mouseDownPosition[1]);
		}
	}


	void NetworkRenderWidget::closeEvent(QCloseEvent* e)
	{
		m_simulation->stop();
	}


	QPainterPath NetworkRenderWidget::roundedConvexHullPath(const std::vector<vec2>& points, double radius) const
	{
		QPainterPath toReturn;
		std::vector<vec2> hull = math::convexHull(points);
		if (hull.size() == 0)
		{
			return toReturn;
		}
		else if (hull.size() == 1)
		{
			toReturn.addEllipse(hull.front()[0] - radius, hull.front()[1] - radius, 2.0 * radius, 2.0 * radius);
			return toReturn;
		}

		// add the first two points of the hull again to facilitate building the path
		hull.push_back(hull[0]);
		hull.push_back(hull[1]);

		for (size_t i = 0; i < hull.size() - 2; ++i)
		{
			const vec2& cur = hull[i];
			const vec2& next = hull[i + 1];
			const vec2& nextNext = hull[i + 2];

			// compute orthogonal vectors to point-to-point directions
			const vec2 ortho = math::rotatedClockwise(next - cur).normalized() * radius;
			const vec2 nextOrtho = math::rotatedClockwise(nextNext - next).normalized() * radius;

			// compute angles for the rounded corners
			const double start = -std::atan2(ortho[1], ortho[0]) * 180 / math::PI;
			const double end = -std::atan2(nextOrtho[1], nextOrtho[0]) * 180 / math::PI;
			double angle = end - start;
			if (angle > 0)
				angle -= 360.0;

			// Add the path segments
			if (i == 0)
				toReturn.moveTo(cur[0] + ortho[0], cur[1] + ortho[1]);
			toReturn.lineTo(next[0] + ortho[0], next[1] + ortho[1]);
			toReturn.arcTo(next[0] - radius, next[1] - radius, 2.0 * radius, 2.0 * radius, start, angle);
		}

		return toReturn;
	}


	void NetworkRenderWidget::onSimulationStep()
	{
		emit updateRequested();
	}


}
}
