#include "databasereplaysystem.h"
#include <QSqlError>
#include <QSqlResult>
#include <QDebug>
#include <QDataStream>

#include <ve_commandevent.h>
#include <vcmp_entitydata.h>
#include <vcmp_componentdata.h>


#include "ecsdataset.h"

DatabaseReplaySystem::DatabaseReplaySystem(QObject *t_parent) : VeinEvent::EventSystem(t_parent)
{
  m_database = QSqlDatabase::addDatabase("QSQLITE");
  connect(&m_dataTimer, &QTimer::timeout, this, &DatabaseReplaySystem::dataTimerFinished);
}

void DatabaseReplaySystem::processEvent(QEvent *t_event)
{
  if(t_event->type() == VeinEvent::CommandEvent::getQEventType())
  {
    VeinEvent::CommandEvent *cEvent = nullptr;
    cEvent = static_cast<VeinEvent::CommandEvent *>(t_event);
    Q_ASSERT(cEvent != nullptr);

    if(cEvent->eventSubtype() == VeinEvent::CommandEvent::EventSubtype::TRANSACTION)
    {
      cEvent->setEventSubtype(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION);
    }
  }
}

void DatabaseReplaySystem::startReplay()
{
  if(m_database.isOpen() && m_dataQuery.isActive())
  {
    m_dataTimer.start();
  }
  else
  {
    qDebug() << "DB:" << m_database.isOpen() << "QUERY:" << m_dataQuery.isValid();
  }
}

bool DatabaseReplaySystem::setDatabaseFile(const QString &t_dbFilePath)
{
  bool retVal = false;
  m_database.setDatabaseName(t_dbFilePath);
  QSqlError dbError;
  if (!m_database.open())
  {
    dbError = m_database.lastError();
    qWarning() << "Error opening database:" << dbError.text();
  }

  if(dbError.type() != QSqlError::NoError)
  {
    qWarning() << "Database error:" << m_database.lastError().text();
  }
  else
  {
    QSqlQuery schemaVersionQuery(m_database);

    if(schemaVersionQuery.exec("pragma schema_version;") == true) //check if the file is valid (empty or a valid database)
    {
      schemaVersionQuery.first();
      if(schemaVersionQuery.value(0) == 0) //if there is no database schema or if the file does not exist, then this will create the database and initialize the schema
      {
        qWarning() << "Database schema is not supported";
      }
      else
      {
        int currentEntityId = -1;
        QMultiHash<int, QString> initialECSData;
        QSqlError queryError;

        m_dataQuery = QSqlQuery(m_database);
        m_initQuery = QSqlQuery(m_database);

        m_initQuery.prepare("SELECT DISTINCT entity_id, component_name FROM valuemap NATURAL JOIN components ORDER BY 1;");
        m_initQuery.exec();
        queryError = m_initQuery.lastError();
        if(queryError.type() != QSqlError::NoError)
        {
          qWarning() << "Error in initQuery:" << queryError.text();
        }


        while(m_initQuery.next())
        {
          const int tmpEntityId = m_initQuery.value(0).toInt();
          if(currentEntityId != tmpEntityId)
          {
            qDebug() << "Added entity" << tmpEntityId;
            //add new entity
            currentEntityId = tmpEntityId;
            VeinComponent::EntityData *entData = new VeinComponent::EntityData();
            entData->setCommand(VeinComponent::EntityData::Command::ECMD_ADD);
            entData->setEntityId(currentEntityId);

            emit sigSendEvent(new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, entData));
          }
          initialECSData.insert(currentEntityId, m_initQuery.value(1).toString());
        }
        m_initQuery.finish();

        VeinComponent::ComponentData *initialData=nullptr;
        for(int loopEntityId : initialECSData.uniqueKeys())
        {
          for(const QString &compName : initialECSData.values(loopEntityId))
          {
            initialData = new VeinComponent::ComponentData();
            initialData->setEntityId(loopEntityId);
            initialData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
            initialData->setComponentName(compName);
            initialData->setNewValue(QVariant());
            initialData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
            initialData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
            emit sigSendEvent(new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, initialData));
          }
        }


        //all values
        m_dataQuery.prepare("SELECT value_timestamp, record_name, entity_id, component_name, component_value FROM valuemap NATURAL JOIN recordmapping NATURAL JOIN records NATURAL JOIN components ORDER BY 1;");
        m_dataQuery.exec();
        queryError = m_dataQuery.lastError();
        if(queryError.type() != QSqlError::NoError)
        {
          qWarning() << "Error in dataQuery:" << queryError.text();
        }


      }
    }
    schemaVersionQuery.finish();
  }

  return retVal;
}

void DatabaseReplaySystem::setTickDelay(int t_tickrate)
{
  m_dataTimer.setInterval(t_tickrate);
  m_tickRate = t_tickrate;
}

void DatabaseReplaySystem::setLoop(bool t_loop)
{
  m_loopData = t_loop;
}

void DatabaseReplaySystem::dataTimerFinished()
{
  //do not read all data at once since the possible database size could be 40GB or even more
  //qDebug() << "initial queue size:" << m_ecsData.size();
  bool hasNext;
  while (m_ecsData.size() < 1000 && (hasNext = m_dataQuery.next()))
  {
    const QDateTime timestamp = QDateTime::fromString(m_dataQuery.value(0).toString(), Qt::ISODate); //value_timestamp
    const QString recordName = m_dataQuery.value(1).toString(); //record_name
    const int entityId = m_dataQuery.value(2).toInt(); //entity_id
    const QString componentName = m_dataQuery.value(3).toString(); //component_name
    QVariant componentValue;

    //data is encoded via QDataStream and stored as QByteArray BLOB
    QByteArray tmpValue = m_dataQuery.value(4).toByteArray();
    Q_ASSERT(tmpValue.isEmpty() == false);
    QDataStream tmpValueStream(&tmpValue, QIODevice::ReadOnly);
    tmpValueStream.setVersion(QDataStream::Qt_5_0);
    tmpValueStream >> componentValue; //reads a QVariant

    m_ecsData.enqueue(new ECSDataset(timestamp, entityId, componentName, recordName, componentValue));
  }
  //init timestamp to first value
  if(Q_UNLIKELY(m_timeIndex.isNull()))
  {
    m_timeIndex = m_ecsData.head()->getTimestamp();
  }
  //increment time index by tickrate
  m_timeIndex = m_timeIndex.addMSecs(m_tickRate);
  int old = m_ecsData.size();
  while(m_ecsData.isEmpty() == false && m_ecsData.head()->getTimestamp().msecsTo(m_timeIndex) > m_tickRate)
  {
    ECSDataset *tmpData = m_ecsData.dequeue();

    VeinComponent::ComponentData *cData = new VeinComponent::ComponentData(tmpData->getEntityId());
    cData->setComponentName(tmpData->getComponentName());
    cData->setNewValue(tmpData->getValue());
    cData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
    cData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
    sigSendEvent(new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, cData));

    delete tmpData;
  }
  qDebug() << "replayed values:" << old - m_ecsData.size();
  if(Q_UNLIKELY(hasNext == false) && Q_UNLIKELY(m_ecsData.isEmpty()))
  {
    if(m_loopData == false)
    {
      qDebug() << "finished database";
      m_dataQuery.finish();
      m_dataTimer.stop();
    }
    else
    {
      //reset dataQuery
      m_dataQuery.first();
      //reset timeIndex
      m_timeIndex = QDateTime();
    }
  }
}
