#ifndef SIZE_H
#define SIZE_H

namespace ion
{
struct Size
{
    int width = 0;
    int height = 0;

    Size(int w, int h) : width(w), height(h)
    {
    }
};
} // namespace ion

#endif