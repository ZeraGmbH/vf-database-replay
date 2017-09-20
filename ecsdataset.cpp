#include "ecsdataset.h"

ECSDataset::ECSDataset(QDateTime t_timestamp, int t_entityId, const QString &t_componentName, const QString &t_recordName, QVariant t_value) :
  m_timestamp(t_timestamp),
  m_entityId(t_entityId),
  m_componentName(t_componentName),
  m_recordName(t_recordName),
  m_value(t_value)
{

}
