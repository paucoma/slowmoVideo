/*
This file is part of slowmoVideo.
Copyright (C) 2014 Valery Brasseur <vbrasseur@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include <QtCore/QObject>

#include "exportVideoRenderTarget.h"
#include "renderTask_sV.h"
#include "../lib/video_enc.h"


exportVideoRenderTarget::exportVideoRenderTarget(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{

	m_targetDir = parentRenderTask->getRenderDirectory();
    m_filenamePattern = "rendered-%1.png";
    
    use_qt = 1;
    first = 0;
}

exportVideoRenderTarget::~exportVideoRenderTarget()
{
    qDebug() << "Closing exporter. Not deleting temporary files under " << m_targetDir.absolutePath() << ".";
}

void exportVideoRenderTarget::setTargetFile(const QString &filename)
{
    m_filename = filename;
}
void exportVideoRenderTarget::setVcodec(const QString &codec)
{
    m_vcodec = codec;
}

void exportVideoRenderTarget::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    if (!m_targetDir.exists()) {
        m_targetDir.mkpath(".");
    }
    QString path = m_targetDir.absoluteFilePath(m_filenamePattern.arg(frameNumber+1, 5, 10, QChar::fromLatin1('0')));

    bool ok;
    ok = image.save(path);
    if (!ok) {
        qDebug() << "  Writing image to " << path << " failed!";
    } else {
        qDebug() << "  Saved frame number " << frameNumber << " to " << path;
    }
    if (first == 0)
	first = frameNumber + 1;
}

void exportVideoRenderTarget::closeRenderTarget() noexcept(false)
{	
	VideoWriter* writer;

	qDebug() << "exporting temporary frame to Video" << m_filename << " using codec " << m_vcodec << "starting at " << first;
	if (m_vcodec.isEmpty()) 
		writer = CreateVideoWriter(m_filename.toStdString().c_str(),
			renderTask()->resolution().width(),
			renderTask()->resolution().height(),
			renderTask()->fps().fps(),use_qt,0);
	else
		writer = CreateVideoWriter(m_filename.toStdString().c_str(),
			renderTask()->resolution().width(),
			renderTask()->resolution().height(),
			renderTask()->fps().fps(),use_qt,m_vcodec.toStdString().c_str());
   
    if (writer == 0) {
        throw Error_sV(QObject::tr("Video could not be prepared .\n"));
    }
	exportFrames(writer, m_targetDir.absoluteFilePath(m_filenamePattern.arg("%05d")).toStdString().c_str(),first,renderTask());
	ReleaseVideoWriter( &writer );
}

  



