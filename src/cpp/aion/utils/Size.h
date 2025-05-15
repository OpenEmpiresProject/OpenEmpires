#ifndef SIZE_H
#define SIZE_H

namespace aion
{
struct Size
{
    int width;
    int height;

    Size(int w, int h) : width(w), height(h)
    {
    }
};
} // namespace aion

#endif