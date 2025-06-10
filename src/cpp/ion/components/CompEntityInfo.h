#ifndef COMPENTITYINFO_H
#define COMPENTITYINFO_H

namespace ion
{
class CompEntityInfo
{
  public:
    int entityType = 0;
    int entitySubType = 0;
    int variation = 0;
    bool isDestroyed = false;

    CompEntityInfo(int entityType) : entityType(entityType)
    {
    }

    CompEntityInfo(int entityType, int entitySubType, int variation) : entityType(entityType), variation(variation),  entitySubType(entitySubType)
    {
    }
};
} // namespace ion

#endif