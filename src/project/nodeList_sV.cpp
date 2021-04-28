/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "nodeList_sV.h"
#include "node_sV.h"
#include "../lib/bezierTools_sV.h"

#include <cmath>

#include <QDebug>

//#define DEBUG_NL
#ifdef DEBUG_NL
#include <iostream>
#endif

NodeList_sV::NodeList_sV(float minDist) :
    m_maxY(10),
    m_list(),
    m_minDist(minDist)
{
}

void NodeList_sV::setMaxY(qreal time)
{
    Q_ASSERT(time > 0);
    m_maxY = time;
}

qreal NodeList_sV::startTime(bool useMoved) const
{
    if (m_list.length() > 0) {
        if (useMoved) {
            return m_list[0].x();
        } else {
            return m_list[0].xUnmoved();
        }
    } else {
//        qDebug() << "No start time available (no nodes)";
        return 0;
    }
}
qreal NodeList_sV::endTime(bool useMoved) const
{
    if (m_list.length() > 0) {
        if (useMoved) {
            return m_list[m_list.length()-1].x();
        } else {
            return m_list[m_list.length()-1].xUnmoved();
        }
    } else {
//        qDebug() << "No end time available (no nodes)";
        return 0;
    }
}
bool NodeList_sV::isInsideCurve(qreal targetTime, bool useMoved) const
{
    return m_list.size() >= 2  &&
            startTime(useMoved) <= targetTime &&
            targetTime <= endTime(useMoved);
}
qreal NodeList_sV::totalTime() const
{
    return endTime()-startTime();
}
qreal NodeList_sV::sourceTime(qreal targetTime) const
{
    qreal srcTime = -1;
    int index = find(targetTime);
    if (index >= 0) {
        if (m_list.size() > index+1) {
            if (m_list.at(index).rightCurveType() == CurveType_Bezier
                    && m_list.at(index+1).leftCurveType() == CurveType_Bezier) {
                srcTime = BezierTools_sV::interpolateAtX(targetTime,
                                                         m_list.at(index).toQPointF(),
                                                         m_list.at(index).toQPointF()+m_list.at(index).rightNodeHandle(),
                                                         m_list.at(index+1).toQPointF()+m_list.at(index+1).leftNodeHandle(),
                                                         m_list.at(index+1).toQPointF()).y();
            } else {
                float ratio = (targetTime-m_list[index].x())/(m_list[index+1].x()-m_list[index].x());
                srcTime = m_list[index].y() + ratio*( m_list[index+1].y()-m_list[index].y() );
            }
        } else {
	    //TODO:
            if (index >= m_list.size()) {
                qDebug() << "index " << index << " is > list size: " << m_list.size();
                //Q_ASSERT(false);
            } else {
                srcTime = m_list[index].y();
            }
        }
    } else {
	// this seem because no project loaded ?
	// TODO: how can we check ?
        qDebug() << "No node before " << targetTime;
        //Q_ASSERT(false);
        if (m_list.size() > 0) {
            srcTime = m_list[0].y();
        }
    }
    return srcTime;
}

bool NodeList_sV::add(Node_sV node)
{
    bool add = true;

#ifdef DEBUG_NL
    qDebug() << "Before adding: \n" << *this;
#endif

    node.setX(qMax(.0, node.x()));
    node.setY(qMax(.0, qMin(m_maxY, node.y())));

    int pos = find(node.x());
    if (pos >= 0 && m_list.size() > pos) {
        add = fabs(node.x()-m_list.at(pos).x()) > m_minDist;
#ifdef DEBUG_NL
        qDebug() << "Left distance is " << fabs(node.x()-m_list.at(pos).x());
#endif
        if (add && m_list.size() > pos+1) {
            add = fabs(node.x()-m_list.at(pos+1).x()) > m_minDist;
#ifdef DEBUG_NL
            qDebug() << "Right distance is " << fabs(node.x()-m_list.at(pos+1).x());
#endif
        }
    }
#ifdef DEBUG_NL
    qDebug() << "Adding? " << add;
#endif
    if (add) {
        m_list.append(node);
        qSort(m_list);

        if (m_list.size() > 1) {
            m_segments.grow();
        }

        // Reset curve type of neighbours if this is a linear node
        int index = m_list.indexOf(node);
        if (index > 0 && node.leftCurveType() == CurveType_Linear) {
            m_list[index-1].setRightCurveType(CurveType_Linear);
        }
        if (index < m_list.size()-1 && node.rightCurveType() == CurveType_Linear) {
            m_list[index+1].setLeftCurveType(CurveType_Linear);
        }

        fixHandles(index-1);
        fixHandles(index);
    }
#ifdef DEBUG_NL
    qDebug() << "After adding: \n" << *this;
#endif

    validate();
    return add;
}

uint NodeList_sV::deleteSelected()
{
    uint counter = 0;
    for (int i = 0; i < m_list.size(); ) {
        if (m_list.at(i).selected()) {
            m_list.removeOne(m_list.at(i));
            if (m_list.size() > 0) {
                m_segments.shrink();
            }
            counter++;
        } else {
            i++;
        }
    }
    validate();
    return counter;
}
void NodeList_sV::deleteNode(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < m_list.size());
    if (m_list.size() > 0) {
        if (m_list.size() > 1) {
            m_segments.shrink();
        }
        m_list.removeAt(index);
    }
    if (index > m_list.size() && (index-1) >= 0) {
        if (m_list.at(index-1).rightCurveType() != m_list.at(index).leftCurveType()) {
            m_list[index-1].setRightCurveType(CurveType_Linear);
            m_list[index].setLeftCurveType(CurveType_Linear);
        }
    }
    validate();
}


void NodeList_sV::select(const Node_sV *node, bool newSelection)
{
    if (newSelection) {
        unselectAll();
        const_cast<Node_sV*>(node)->select(true);
    } else {
        const_cast<Node_sV*>(node)->select(!node->selected());
    }
}

void NodeList_sV::unselectAll()
{
    for (int i = 0; i < m_list.size(); i++) {
        m_list[i].select(false);
    }
}


bool NodeList_sV::validate() const
{
    bool valid = true;
    qreal last = -m_minDist;
    for (int i = 0; i < m_list.size() && valid; i++) {
        valid =    m_list.at(i).x() >= 0
                && m_list.at(i).y() >= 0
                && m_list.at(i).x() - last >= m_minDist
                && m_list.at(i).y() <= m_maxY;
        if (!valid) {
            qDebug() << "Invalid node position for node " << i << " (" << m_list.size() << " total); Distance is " << m_list.at(i).x() - last;
            qDebug() << "Positions: " << last << "/" << m_list.at(i).x();
            Q_ASSERT(false);
            break;
        }
        last = m_list.at(i).x();
    }
    if (valid) {
        for (int i = 1; i < m_list.size(); i++) {
            float space = (m_list.at(i).x() + m_list.at(i).leftNodeHandle().x())
                    - (m_list.at(i-1).x() + m_list.at(i-1).rightNodeHandle().x());
            valid = space >= 0;
            if (!valid) {
                qDebug() << "Invalid handle position for nodes " << i-1 << " and " << i;
                qDebug() << "Positions: " << m_list.at(i-1) << " with handle " << toString(m_list.at(i-1).rightNodeHandle())
                         << ", " << m_list.at(i) << " with handle " << toString(m_list.at(i).leftNodeHandle())
                         << ", space: " << space;
                Q_ASSERT(false);
                break;
            }
        }
    }
    if (valid) {
        Q_ASSERT(   (m_list.size() == 0 && m_segments.size() == 0)
                 || (m_list.size() > 0 && m_segments.size() == m_list.size()-1) );
    }
    return valid;
}


////////// Moving

void NodeList_sV::moveSelected(const Node_sV &time,bool snap)
{
    qreal maxRMove = 100000;
    qreal maxLMove = -100000;
    qreal maxUMove = 100000;
    qreal maxDMove = -100000;
    const Node_sV *left = NULL;
    const Node_sV *right;
    for (int i = 0; i < m_list.size(); i++) {
        right = &m_list.at(i);

        /*
          Get the maximum allowed horizontal movement distance here such that there is no
          overlapping. For moving the selected nodes to the left, only unselected nodes
          which are directly followed by a selected node need to be taken into account.
                 O----O
                /      \               x
          -----x        \             /
                         x-----------O

         min(  ^1^,      ^-----2-----^    ) + minDist

         */
        if (left != NULL) {
            if (left->selected() && !right->selected()) {
                // Move-right distance
                maxRMove = qMin(maxRMove,
                                right->xUnmoved()+right->leftNodeHandle().x() -
                                (left->xUnmoved()+left->rightNodeHandle().x()) -
                                m_minDist);
            } else if (!left->selected() && right->selected()) {
                // Move-left distance
                maxLMove = qMax(maxLMove,
                                left->xUnmoved()+left->rightNodeHandle().x() -
                                (right->xUnmoved()+right->leftNodeHandle().x()) +
                                m_minDist);
            }
        }

        if (right->selected()) {
            maxDMove = qMax(maxDMove, -right->yUnmoved());
            maxUMove = qMin(maxUMove, m_maxY-right->yUnmoved());
        }

        left = right;
    }
    if (m_list.size() > 0 && m_list.at(0).selected()) {
        // Do not allow to move nodes to x < 0
        maxLMove = qMax(maxLMove, -m_list.at(0).xUnmoved());
    }
#ifdef DEBUG_NL
    qDebug() << "Max move: left " << maxLMove << ", right: " << maxRMove;
#endif
    Node_sV newTime(
                qMax(maxLMove, qMin(maxRMove, time.x())),
                qMax(maxDMove, qMin(maxUMove, time.y()))
                );
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i).selected()) {
            m_list[i].move(newTime);
        }
    }
}
void NodeList_sV::shift(qreal after, qreal by)
{
    int pos = nodeAfter(after);
    if (pos >= 0) {
        if (pos > 0) {
            // []----o     o----[]---   <- nodes with handles
            //        <--->             <- maximum distance
            by = qMax(by,
                      m_list.at(pos-1).xUnmoved()+m_list.at(pos-1).rightNodeHandle().x() -
                      (m_list.at(pos).xUnmoved()+m_list.at(pos).leftNodeHandle().x()) +
                      m_minDist
                      );
        }
        if (pos == 0) {
            by = qMax(by, -m_list.at(pos).xUnmoved());
        }
        for (; pos < m_list.size(); pos++) {
            m_list[pos].move(Node_sV(by, 0));
        }
    }
    if (!validate()) {
        qDebug() << "Invalid node configuration! (This should not happen.)";
    }
}

void NodeList_sV::confirmMove()
{
    for (int i = 0; i < m_list.size(); i++) {
        m_list[i].confirmMove();
    }
    validate();
}
void NodeList_sV::abortMove()
{
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i).selected()) {
            m_list[i].abortMove();
        }
    }
}

void NodeList_sV::moveHandle(const NodeHandle_sV *handle, Node_sV relPos)
{
    Node_sV otherNode;
    Node_sV *currentNode = const_cast<Node_sV*>(handle->parentNode());

    int nodeIndex = indexOf(handle->parentNode());
    Q_ASSERT(nodeIndex >= 0);
    Q_ASSERT(nodeIndex < m_list.size());

    if (handle == &currentNode->leftNodeHandle()) {
        //  o------[]
        if (nodeIndex > 0) {
            // Ensure that it does not overlap with the left node's handle (injectivity)
            otherNode = m_list.at(nodeIndex-1);
            qDebug() << "Left node: " << otherNode;
            qDebug() << "Right node: " << currentNode;
            qDebug() << "Before overlapping check: " << relPos;
            relPos.setX(qMax(relPos.x(), -(currentNode->x() - otherNode.x() - otherNode.rightNodeHandle().x())));
            qDebug() << "After overlapping check: " << relPos;
            qDebug() << "Space left: " << currentNode->x() + relPos.x() - (otherNode.x() + otherNode.rightNodeHandle().x());
        }
        // Additionally the handle has to stay on the left of its node
        relPos.setX(qMin(relPos.x(), .0));

        currentNode->setLeftNodeHandle(relPos.x(), relPos.y());

    } else {
        // []-------o
        if (nodeIndex+1 < m_list.size()) {
            otherNode = m_list.at(nodeIndex+1);
            relPos.setX(qMin(relPos.x(), otherNode.x() - currentNode->x() + otherNode.leftNodeHandle().x()));
        }
        relPos.setX(qMax(relPos.x(), .0));

        currentNode->setRightNodeHandle(relPos.x(), relPos.y());
    }
    validate();
}




////////// Curve

void NodeList_sV::setCurveType(qreal segmentTime, CurveType type)
{
    int left, right;
    findBySegment(segmentTime, left, right);
#ifdef DEBUG_NL
    qDebug() << "Setting curve type for nodes " << left << " and " << right;
#endif
    if (left != -1) {
        m_list[left].setRightCurveType(type);
    }
    if (right != -1) {
        m_list[right].setLeftCurveType(type);
    }
}
void NodeList_sV::fixHandles(int leftIndex)
{
    if (leftIndex >= 0 && (leftIndex+1) < m_list.size()) {
        qreal right = m_list.at(leftIndex+1).x() - m_list.at(leftIndex).x();
        qreal leftHandle = m_list.at(leftIndex).rightNodeHandle().x();
        qreal rightHandle = m_list.at(leftIndex+1).leftNodeHandle().x();

        if (leftHandle < 0) { leftHandle = 0; }
        if (rightHandle > 0) { rightHandle = 0; }

        if (leftHandle > right+rightHandle && (leftHandle-rightHandle) > 0) {
            qreal factor = right / (leftHandle - rightHandle);
            qDebug() << "Factor: " << factor << ", left: " << leftHandle << ", right: " << rightHandle << ", distance: " << right;
            leftHandle *= factor;
            rightHandle *= factor;
            qDebug() << "After scaling: left: " << leftHandle << ", right: " << rightHandle;
            Q_ASSERT(leftHandle <= right+rightHandle);
        }
        m_list[leftIndex].setRightNodeHandle(leftHandle, m_list.at(leftIndex).rightNodeHandle().y());
        m_list[leftIndex+1].setLeftNodeHandle(rightHandle, m_list.at(leftIndex+1).leftNodeHandle().y());
    }
}

/**
 * on error return int indicating error type
 * hint : maybe add error method ?
 */
int NodeList_sV::setSpeed(qreal segmentTime, qreal speed)
{
    int error = 0;
    int left, right;
    findBySegment(segmentTime, left, right);
    if (left >= 0 && right >= 0) {
        Node_sV *leftN = &m_list[left];
        Node_sV *rightN = &m_list[right];
        qreal y = leftN->y() + speed*(rightN->x()-leftN->x());
        if (y > m_maxY || y < 0) {
            if (y > m_maxY) {
                qDebug() << speed << "x speed would shoot over maximum time. Correcting.";
                error = -1;
                y = m_maxY;
            } else {
                qDebug() << speed << "x speed goes below 0. Correcting.";
                error = -2;
                y = 0;
            }
            qreal xNew = leftN->x() + (y - leftN->y())/speed;
            rightN->setY(y);
            if (xNew - leftN->x() >= m_minDist) {
                add(Node_sV(xNew, y));
            } else {
                qDebug() << "New node would be too close, not adding it.";
                error = -3;
            }
        } else {
            rightN->setY(y);
        }
    } else {
        qDebug() << "Outside segment.";
        error = -4;
    }
    validate();
    return error;
}




////////// Access

int NodeList_sV::indexOf(const Node_sV *node) const
{
    return m_list.indexOf(*node);
}

int NodeList_sV::find(qreal time) const
{
    int pos;
    for (
         pos = 0;
         m_list.size() > (pos+1) && m_list.at(pos+1).x() <= time;
         pos++
         ) {}
    if (m_list.size() == 0 || (pos == 0 && time < m_list[0].x())) {
#ifdef DEBUG_NL
        if (m_list.size() > 0) {
            std::cout.precision(30);
            std::cout << "find(): time: " << time << ", left boundary: " << m_list[0].x()
                      << ", unmoved: " << m_list[0].xUnmoved()
                      << ", diff: " << m_list[pos].x()-time << std::endl;
        }
#endif
        pos = -1;
    }
    return pos;
}
int NodeList_sV::find(QPointF pos, qreal tdelta) const
{
    for (int i = 0; i < m_list.size(); i++) {
        if (std::pow(m_list.at(i).xUnmoved() - pos.x(), 2) + std::pow(m_list.at(i).yUnmoved()-pos.y(), 2)
                < std::pow(tdelta, 2)) {
            return i;
        }
    }
    return -1;
}

void NodeList_sV::findBySegment(qreal tx, int &leftIndex_out, int &rightIndex_out) const
{
    for (int i = 0; i < m_list.size(); i++) {
        leftIndex_out = i-1;
        rightIndex_out = i;
        if (m_list.at(i).xUnmoved() > tx) {
            break;
        }
        if (i == m_list.size()-1) {
            leftIndex_out = i;
            rightIndex_out = -1;
        }
    }
}

QList<NodeList_sV::PointerWithDistance> NodeList_sV::objectsNear(QPointF pos, qreal tmaxdist) const
{
    qreal maxdist2 = std::pow(tmaxdist, 2);

    QList<PointerWithDistance> objects;
    qreal dist;
    for (int i = 0; i < m_list.size(); i++) {

        dist = dist2(m_list.at(i).toQPointF()  -  pos);
        if (dist <= maxdist2) {
            objects << PointerWithDistance(&m_list[i], dist, PointerWithDistance::Node);
        }

        if (m_list.at(i).leftCurveType() != CurveType_Linear) {
            dist = dist2(m_list.at(i).toQPointF() + m_list.at(i).leftNodeHandle()  -  pos);
            if (dist <= maxdist2) {
                objects << PointerWithDistance(&m_list[i].leftNodeHandle(), dist, PointerWithDistance::Handle);
            }
        }
        if (m_list.at(i).rightCurveType() != CurveType_Linear) {
            dist = dist2(m_list.at(i).toQPointF() + m_list.at(i).rightNodeHandle()  -  pos);
            if (dist <= maxdist2) {
                objects << PointerWithDistance(&m_list[i].rightNodeHandle(), dist, PointerWithDistance::Handle);
            }
        }
        if (i > 0) {
            if (m_list.at(i-1).x() < pos.x() && m_list.at(i).x() > pos.x()) {
                objects << PointerWithDistance(&m_segments.at(i-1), std::pow(sourceTime(pos.x()) - pos.y(), 2), PointerWithDistance::Segment);
            }
        }
    }

    qSort(objects);
    return objects;
}
qreal NodeList_sV::dist2(QPointF point) const
{
    return std::pow(point.x(), 2) + std::pow(point.y(), 2);
}

int NodeList_sV::nodeAfter(qreal time) const
{
    int pos = 0;
    while (m_list.size() > pos) {
        if (m_list.at(pos).xUnmoved() >= time) {
            break;
        }
        pos++;
    }
    if (pos >= m_list.size()) {
        pos = -1;
    }
    Q_ASSERT(pos < 0 || m_list.at(pos).xUnmoved() >= time);
    return pos;
}

const Node_sV& NodeList_sV::at(int i) const { return m_list.at(i); }
Node_sV& NodeList_sV::operator[](int i) { return m_list[i]; }
int NodeList_sV::size() const { return m_list.size(); }

SegmentList_sV* NodeList_sV::segments()
{
    return &m_segments;
}





////////// Debug

QDebug operator<<(QDebug dbg, const NodeList_sV &list)
{
    for (int i = 0; i < list.size(); i++) {
        dbg.nospace() << i << ": " << list.at(i) << " ";
    }
    return dbg.maybeSpace();
}

