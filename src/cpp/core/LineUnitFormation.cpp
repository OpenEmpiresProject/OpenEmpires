#include "LineUnitFormation.h"

#include "ServiceRegistry.h"
#include "StateManager.h"
#include "components/CompTransform.h"
#include "logging/Logger.h"

#include <algorithm>

using namespace core;

LineUnitFormation::LineUnitFormation()
{
    // constructor
}

LineUnitFormation::~LineUnitFormation()
{
    // destructor
}

void LineUnitFormation::createFormation(const std::vector<uint32_t>& unitEntityIds,
                                        const Feet& target)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    auto unitCount = unitEntityIds.size();
    if (unitCount == 0)
    {
        spdlog::debug("Invalid formation creation request, no units assigned");
        return;
    }

    int unitCountWidth = unitCount;

    if (unitCount > MAX_FORMATION_WIDTH)
    {
        unitCountWidth = std::max((int) std::ceil(std::sqrt(unitCount)), MAX_FORMATION_WIDTH);
    }
    int unitCountLength = std::ceil((float) unitCount / unitCountWidth);
    int slotCount = unitCountWidth * unitCountLength;

    m_anchor = Feet::zero;
    float totalWeight = 0.0f;
    int maxCollisionRadius = std::numeric_limits<int>::min();
    std::vector<CompTransform*> unitTransforms(unitCount);
    int minVelocity = std::numeric_limits<int>::max();

    for (int i = 0; i < unitCount; ++i)
    {
        auto& transform = stateMan->getComponent<CompTransform>(unitEntityIds[i]);
        maxCollisionRadius = std::max(maxCollisionRadius, transform.collisionRadius);
        unitTransforms[i] = &transform;

        auto w = static_cast<float>(unitTransforms[i]->collisionRadius);
        m_anchor += unitTransforms[i]->position * w;
        totalWeight += w;

        minVelocity = std::min(minVelocity, (int) transform.speed);
    }
    m_anchor /= totalWeight;
    m_speed = minVelocity;
    m_target = target;

    auto formationWidthFeet = SPACING_FACTOR * maxCollisionRadius * 2 * unitCountWidth;
    auto formationLengthFeet = SPACING_FACTOR * maxCollisionRadius * 2 * unitCountLength;

    std::vector<Feet> slotPositions(slotCount);
    // spacing between adjacent slots (feet)
    float cellSpacing = SPACING_FACTOR * static_cast<float>(maxCollisionRadius) * 2.0f;

    // start offsets so formation is centered around (0,0)
    float startX = -formationWidthFeet * 0.5f + cellSpacing * 0.5f;
    float startY = formationLengthFeet * 0.5f - cellSpacing * 0.5f;
    int slotIndex = 0;

    m_forward = (target - m_anchor).normalized();
    Feet right(m_forward.y, -m_forward.x);

    for (int y = 0; y < unitCountLength; ++y)
    {
        for (int x = 0; x < unitCountWidth; ++x)
        {
            float posX = startX + x * cellSpacing;
            float posY = startY - y * cellSpacing;
            Feet slotPos(posX, posY);

            Feet absoluteSlotPos = m_anchor + slotPos;
            Feet rotatedOffset = right * slotPos.x + m_forward * slotPos.y;

            slotPositions[slotIndex] = rotatedOffset;

            ++slotIndex;
        }
    }

    std::vector<bool> unitsTaken(unitCount, false);
    auto thisRef = shared_from_this();
    m_slots.reserve(unitCount);

    for (int slotIndex = 0; slotIndex < slotCount; ++slotIndex)
    {
        auto& slotPos = slotPositions[slotIndex];
        float bestDistanceSq = std::numeric_limits<float>::max();
        int bestUnit = -1;
        Feet absoluteSlotPos = m_anchor + slotPos;

        for (int i = 0; i < unitCount; ++i)
        {
            if (unitsTaken[i])
                continue;

            auto unitTransform = unitTransforms[i];

            float distanceSq = (absoluteSlotPos - unitTransform->position).lengthSquared();
            if (distanceSq < bestDistanceSq)
            {
                bestUnit = i;
                bestDistanceSq = distanceSq;
            }
        }

        if (bestUnit != -1)
        {
            uint32_t unit = unitEntityIds[bestUnit];

            unitsTaken[bestUnit] = true;
            FormationSlot slot(unit, thisRef);
            slot.index = slotIndex;
            slot.offsetFromAnchor = slotPos;

            m_slots.push_back(slot);
        }
    }
}

void LineUnitFormation::move(const Feet& newPos)
{
}

void LineUnitFormation::deleteFormation()
{
    BaseUnitFormation::removeAllSlots();
    m_state = FormationState::CANCELLED;
}

bool LineUnitFormation::isInsideFormation(const Feet& point) const
{
    if (m_slots.size() == 0)
        return false;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& slot : m_slots)
    {
        const auto& o = slot.offsetFromAnchor;
        minX = std::min(minX, o.x);
        maxX = std::max(maxX, o.x);
        minY = std::min(minY, o.y);
        maxY = std::max(maxY, o.y);
    }

    float halfWidth = (maxX - minX) * 0.5f;
    float halfLength = (maxY - minY) * 0.5f;

    auto right = m_forward.rotated(90);

    Feet d = point - m_anchor;

    float localX = d.dot(right);
    float localY = d.dot(m_forward);

    return std::abs(localX) <= halfWidth && std::abs(localY) <= halfLength;
}
