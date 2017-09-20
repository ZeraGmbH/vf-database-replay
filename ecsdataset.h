#ifndef ECSDATASET_H
#define ECSDATASET_H

#include <QDateTime>
#include <QString>
#include <QVariant>

class ECSDataset
{
public:
  ECSDataset(QDateTime t_timestamp, int t_entityId, const QString &t_componentName, const QString &t_recordName, QVariant t_value);

  QDateTime getTimestamp() const
  {
    return m_timestamp;
  }

  int getEntityId() const
  {
    return m_entityId;
  }

  QString getComponentName() const
  {
    return m_componentName;
  }

  QString getRecordName() const
  {
    return m_recordName;
  }

  QVariant getValue() const
  {
    return m_value;
  }

private:
  const QDateTime m_timestamp;
  const int m_entityId;
  const QString m_componentName;
  const QString m_recordName;
  const QVariant m_value;
};

#endif // ECSDATASET_H
