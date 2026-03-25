#ifndef CORE_LINEUNITFORMATION_H
#define CORE_LINEUNITFORMATION_H
#include "BaseUnitFormation.h"
#include "utils/Size.h"

namespace core
{
class LineUnitFormation : public BaseUnitFormation,
                          public std::enable_shared_from_this<LineUnitFormation>
{
  public:
    LineUnitFormation();
    ~LineUnitFormation();

    void createFormation(const std::vector<uint32_t>& unitEntityIds, const Feet& target) override;
    void deleteFormation() override;
    void move(const Feet& newPos) override;
    bool isInsideFormation(const Feet& point) const override;

    // Line formation allows units collision circles to overlap slightly,
    // hence less than 1.0f.
    const float SPACING_FACTOR = 1.0f;
    const int MAX_FORMATION_WIDTH = 5;

  private:
    Size m_dimentions;
};
} // namespace core

#endif // CORE_LINEUNITFORMATION_H
