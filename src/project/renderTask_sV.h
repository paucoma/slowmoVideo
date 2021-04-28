/*
* 2014 Valery Brasseur <vbrasseur@gmail.com>
*/

#ifndef RENDERTASK_SV_H
#define RENDERTASK_SV_H

#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QTime>
#include <QtCore/QElapsedTimer>

#include "renderPreferences_sV.h"

class Project_sV;
class AbstractRenderTarget_sV;



#include <QObject>
#include <QMutex>

class RenderTask_sV : public QObject 
{
    Q_OBJECT
    
public:
    RenderTask_sV(Project_sV *project);
    ~RenderTask_sV();

    /**
     * Requests the process to start
     *
     * It is thread safe as it uses #mutex to protect access to #_working variable.
     */
    void requestWork();
    /**
     *  Requests the process to abort
     *
     * It is thread safe as it uses #mutex to protect access to #_abort variable.
     */
    void abort();

    void setRenderTarget(AbstractRenderTarget_sV *renderTarget);
    void setTimeRange(qreal start, qreal end);
    void setTimeRange(QString start, QString end);

    QDir getRenderDirectory();

    /// Rendered frames per second
    Fps_sV fps() { return m_prefs.fps(); }
    /// Output frame resolution
    QSize resolution();

    RenderPreferences_sV& renderPreferences() { return m_prefs; }
    
    void setupProgress(QString desc, int taskSize);
    void updateProgress(int value);
    void stepProgress(int step=1);
    void updateMessage(QString desc);
    
private:
    /**
     *  true when Worker is doing work
     */
    bool _working;
    /**
     * Protects access to #_abort
     */
    QMutex mutex;
    
    Project_sV *m_project;
    RenderPreferences_sV m_prefs; ///< \todo Set preferences

    AbstractRenderTarget_sV *m_renderTarget;

    qreal m_timeStart;
    qreal m_timeEnd;

    QElapsedTimer m_stopwatch;
    qint64 m_renderTimeElapsed;

    bool m_stopRendering; //  Process is aborted when true
    qreal m_nextFrameTime;

    qreal m_prevTime;

	int currentProgress;
	
signals:
    /**
     * This signal is emitted when the Worker request to Work
     * requestWork()
     */
    void workFlowRequested();
  
    /**
     *  signal rendering progression
     */
    void signalItemDesc(QString desc);
    void signalTaskProgress(int value);
    
    /**
     * This signal is emitted when process is finished (or being aborted)
     */
    void signalRenderingContinued();
    void signalRenderingStopped(QString renderTime);
    void signalRenderingFinished(QString renderTime);
    void signalRenderingAborted(QString reason);
       
    void signalNewTask(QString desc, int taskSize);
    
public slots:
    void slotContinueRendering();
    void slotStopRendering();

};

#endif // RENDERTASK_SV_H

