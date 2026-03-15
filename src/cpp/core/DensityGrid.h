
#ifndef CORE_DENSITYGRID_H
#define CORE_DENSITYGRID_H
#include "Feet.h"
#include "Flat2DArray.h"

namespace core
{
class DensityGrid
{
  public:
    void init(uint32_t width, uint32_t height);
    void clear();
    void incrementDensity(const Feet& pos);
    int getDensity(const Feet& pos);
    float getDensitySaturated(const Feet& pos);
    size_t width() const;
    size_t height() const;

  private:
    Flat2DArray<int> m_map;
    constexpr static int m_centerWeight = 4;
    constexpr static int m_densitySaturationParameter = 4;
};
} // namespace core

#endif // CORE_DENSITYGRID_H
