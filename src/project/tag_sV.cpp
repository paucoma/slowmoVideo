#include "tag_sV.h"


Tag_sV::Tag_sV(TagAxis axis) :
    m_axis(axis)
{ }

Tag_sV::Tag_sV(qreal time, QString description, TagAxis axis) :
    m_axis(axis),
    m_time(time),
    m_description(description)
{ }

void Tag_sV::setAxis(TagAxis axis)
{
    m_axis = axis;
}

void Tag_sV::setDescription(QString desc)
{
    m_description = desc;
}

void Tag_sV::setTime(qreal time)
{
    m_time = time;
}

bool Tag_sV::operator <(const Tag_sV &other) const
{
    return m_time < other.time();
}
