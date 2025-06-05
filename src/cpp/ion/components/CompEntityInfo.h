#ifndef COMPENTITYINFO_H
#define COMPENTITYINFO_H

namespace ion
{
class CompEntityInfo
{
  public:
    int entityType = 0;
    int variation = 0;

    CompEntityInfo(int entityType) : entityType(entityType)
    {
    }

    CompEntityInfo(int entityType, int variation) : entityType(entityType), variation(variation)
    {
    }
};
} // namespace ion

#endif