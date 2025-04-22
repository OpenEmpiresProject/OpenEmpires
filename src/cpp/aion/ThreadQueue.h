#ifndef THREADQUEUE_H
#define THREADQUEUE_H

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
class ThreadMessage
{
  public:
    enum class Type
    {
        NONE = 0,
        RENDER,
        INPUT,
    };

    ThreadMessage(Type type = Type::NONE) : type(type)
    {
        commandBuffer.reserve(100);
    }

    const Type type = Type::NONE;
    std::vector<void*> commandBuffer;
};

using ThreadQueue = moodycamel::ReaderWriterQueue<ThreadMessage*>;

} // namespace aion
//
#endif