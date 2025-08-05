#ifndef SIZE_H
#define SIZE_H

namespace core
{
struct Size
{
    int width = 0;
    int height = 0;

    Size() = default;

    Size(int w, int h) : width(w), height(h)
    {
    }
};
} // namespace core

#endif