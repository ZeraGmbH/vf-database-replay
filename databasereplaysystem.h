#ifndef DATABASEREPLAYCONTROLLER_H
#define DATABASEREPLAYCONTROLLER_H

#include <ve_eventsystem.h>
#include <QSqlDatabase>
#include <QDateTime>
#include <QTimer>
#include <QSqlQuery>
#include <QQueue>

class ECSDataset;

class DatabaseReplaySystem : public VeinEvent::EventSystem
{
  Q_OBJECT
public:
  explicit DatabaseReplaySystem(QObject *t_parent = 0);

  // EventSystem interface
public:
  bool processEvent(QEvent *t_event) override;

signals:
  void finished();

public slots:
  void startReplay();
  bool setDatabaseFile(const QString &t_dbFilePath);
  void setTickrate(int t_tickrate);
  void setLoop(bool t_loop);
  void dataTimerFinished();

private:
  QSqlDatabase m_database;
  QSqlQuery m_dataQuery;
  QSqlQuery m_initQuery;
  int m_tickRate;
  bool m_loopData=false;
  QDateTime m_timeIndex;
  QTimer m_dataTimer;
  QQueue<ECSDataset *> m_ecsData;
};

#endif // DATABASEREPLAYCONTROLLER_H
