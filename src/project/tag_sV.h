#ifndef TAG_SV_H
#define TAG_SV_H

#include "../lib/defs_sV.hpp"
#include "canvasObject_sV.h"
#include <QString>

/**
  \brief Tags are used for marking positions in time in the project.
 */
class Tag_sV : public CanvasObject_sV
{
public:
    Tag_sV(TagAxis axis = TagAxis_Source);
    Tag_sV(qreal time, QString description, TagAxis axis = TagAxis_Source);
    ~Tag_sV() {}

    TagAxis axis() const { return m_axis; }
    qreal time() const { return m_time; }
    const QString& description() const { return m_description; }

    void setAxis(TagAxis axis);
    void setTime(qreal time);
    void setDescription(QString desc); // If renaming allowed: Update preferences!

    bool operator <(const Tag_sV& other) const;

private:
    TagAxis m_axis;
    qreal m_time;
    QString m_description;

};

#endif // TAG_SV_H
