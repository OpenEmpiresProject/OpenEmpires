#include "DensityGrid.h"

using namespace core;

void DensityGrid::init(uint32_t width, uint32_t height)
{
    m_map.resize(width, height);
}

void DensityGrid::incrementDensity(const Feet& pos)
{
    auto densityGridPos = pos / (Constants::FEET_PER_TILE / Constants::DENSITY_GRID_RESOLUTION);
    auto x = static_cast<int>(densityGridPos.x);
    auto y = static_cast<int>(densityGridPos.y);

    if (m_map.isValidPos(x, y))
    {
        /*  Gaussian blur like kernel
            2 4 2
            4 4 4
            2 4 2
        */
        m_map(x, y) += m_centerWeight;

        if (m_map.isValidPos(x + 1, y))
            m_map(x + 1, y) += m_centerWeight;

        if (m_map.isValidPos(x - 1, y))
            m_map(x - 1, y) += m_centerWeight;

        if (m_map.isValidPos(x, y + 1))
            m_map(x, y + 1) += m_centerWeight;

        if (m_map.isValidPos(x, y - 1))
            m_map(x, y - 1) += m_centerWeight;

        if (m_map.isValidPos(x + 1, y + 1))
            m_map(x + 1, y + 1) += m_centerWeight / 2;

        if (m_map.isValidPos(x - 1, y - 1))
            m_map(x - 1, y - 1) += m_centerWeight / 2;

        if (m_map.isValidPos(x + 1, y - 1))
            m_map(x + 1, y - 1) += m_centerWeight / 2;

        if (m_map.isValidPos(x - 1, y + 1))
            m_map(x - 1, y + 1) += m_centerWeight / 2;
    }
}

int DensityGrid::getDensity(const Feet& pos)
{
    auto densityGridPos = pos / (Constants::FEET_PER_TILE / Constants::DENSITY_GRID_RESOLUTION);
    return m_map.at(static_cast<size_t>(densityGridPos.x), static_cast<size_t>(densityGridPos.y));
}

void DensityGrid::clear()
{
    m_map.fill(0);
}

/*
 *   Get density value with saturation. Always return value between 0 and 1.
 */
float DensityGrid::getDensitySaturated(const Feet& pos)
{
    float density = getDensity(pos);
    return density / (density + m_densitySaturationParameter);
}

size_t DensityGrid::width() const
{
    return m_map.width();
}

size_t DensityGrid::height() const
{
    return m_map.height();
}
