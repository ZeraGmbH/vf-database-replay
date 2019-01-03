#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QDebug>
#include <QMimeDatabase>

#include <ve_eventhandler.h>
#include <ve_eventsystem.h>
#include <vs_veinhash.h>
#include <vn_introspectionsystem.h>
#include <vn_networksystem.h>
#include <vn_tcpsystem.h>

#include "databasereplaysystem.h"

bool checkDatabaseParam(const QString &t_dbParam)
{
  bool retVal = false;
  if(t_dbParam.isEmpty())
  {
    qWarning() << "invalid parameter f" << t_dbParam;
  }
  else
  {
    QFileInfo dbFInfo;
    dbFInfo.setFile(t_dbParam);

    if(dbFInfo.exists() == false)
    {
      qWarning() << "Database file does not exist:" << t_dbParam;
    }
    else
    {
      QMimeDatabase mimeDB;
      const QString mimeName = mimeDB.mimeTypeForFile(dbFInfo, QMimeDatabase::MatchContent).name();
      if(mimeName == "application/x-sqlite3" || mimeName == "application/vnd.sqlite3")
      {
        retVal = true;
        qDebug() << "Database file:"  << t_dbParam << QString("%1 MB").arg(dbFInfo.size()/1024.0/1024.0);
      }
      else
      {
        qWarning() << "Database filetype not supported:" << mimeName;
      }
    }
  }

  return retVal;
}

bool checkTickDelayParam(const QString &t_tickDelay)
{
  bool retVal = false;
  bool tickDelayOk = false;
  const int tickDelay = t_tickDelay.toInt(&tickDelayOk);
  if(tickDelayOk == false || tickDelay < 10 || tickDelay > 1000)
  {
    qWarning() << "Invalid parameter t" << t_tickDelay;
  }
  else
  {
    retVal = true;
    qDebug() << "tick delay:" << tickDelay;
  }

  return retVal;
}

int main(int argc, char *argv[])
{
  QStringList loggingFilters = QStringList() << QString("%1.debug=false").arg(VEIN_EVENT().categoryName()) <<
                                                QString("%1.debug=false").arg(VEIN_NET_VERBOSE().categoryName()) <<
//                                                QString("%1.debug=false").arg(VEIN_NET_INTRO_VERBOSE().categoryName()) << //< Introspection logging is still enabled
                                                QString("%1.debug=false").arg(VEIN_NET_TCP_VERBOSE().categoryName()) <<
                                                QString("%1.debug=false").arg(VEIN_STORAGE_HASH_VERBOSE().categoryName());


  QLoggingCategory::setFilterRules(loggingFilters.join("\n"));

  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("vf-database-replay");
  QCoreApplication::setApplicationVersion("0.1");

  QCommandLineParser parser;
  parser.setApplicationDescription("Reads specially formated SQLite databases and replays the data from the value");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption databaseOption("f",
                                    QCoreApplication::translate("main", "SQLite 3 database file to read from"),
                                    QCoreApplication::translate("main", "database file"));


  QCommandLineOption tickDelayOption("t",
                                    QCoreApplication::translate("main", "Delay between updates as integer ms value\n10 <= tick delay <= 1000"),
                                    QCoreApplication::translate("main", "tick delay"));

  QCommandLineOption loopOption("l", QCoreApplication::translate("main", "Loop over data until interrupted"));
  parser.addOption(loopOption);

  parser.addOption(databaseOption);
  parser.addOption(tickDelayOption);
  parser.process(app);

  if(checkDatabaseParam(parser.value(databaseOption)) == false)
  {
    //return with exit 1
    parser.showHelp(1);
  }

  if(checkTickDelayParam(parser.value(tickDelayOption)) == false)
  {
    //return with exit 1
    parser.showHelp(1);
  }

  VeinEvent::EventHandler evHandler;
  DatabaseReplaySystem *replaySystem = new DatabaseReplaySystem(&app);
  VeinStorage::VeinHash *storSystem = new VeinStorage::VeinHash(&app);
  VeinNet::IntrospectionSystem *introspectionSystem = new VeinNet::IntrospectionSystem(&app);
  VeinNet::NetworkSystem *netSystem = new VeinNet::NetworkSystem(&app);
  VeinNet::TcpSystem *tcpSystem = new VeinNet::TcpSystem(&app);

  netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

  QList<VeinEvent::EventSystem*> subSystems;
  subSystems.append(replaySystem);
  subSystems.append(storSystem);
  subSystems.append(introspectionSystem);
  subSystems.append(netSystem);
  subSystems.append(tcpSystem);

  evHandler.setSubsystems(subSystems);

  replaySystem->setDatabaseFile(parser.value(databaseOption));
  replaySystem->setTickDelay(parser.value(tickDelayOption).toInt());
  replaySystem->startReplay();
  replaySystem->setLoop(parser.isSet(loopOption));


  if(tcpSystem->startServer(12000) == false)
  {
    app.exit(-EADDRINUSE);
  }

  return app.exec();
}
