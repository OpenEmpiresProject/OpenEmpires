#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <chrono>
#include <deque>
#include <numeric>

namespace aion
{

class FPSCounter
{
  public:
    FPSCounter(size_t maxSamples = 100)
        : maxSamples_(maxSamples), lastTime_(std::chrono::steady_clock::now()),
          lastStableTime_(lastTime_)
    {
    }

    void frame()
    {
        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - lastTime_).count();
        lastTime_ = now;

        if (delta > 0.0f)
        {
            fpsSamples_.push_back(1.0f / delta);
            if (fpsSamples_.size() > maxSamples_)
            {
                fpsSamples_.pop_front();
            }
        }
    }

    float getAverageFPS()
    {
        if (isStable())
        {
            stableFPS_ = getFPS();
        }
        return stableFPS_;
    }

    float getFPS() const
    {
        if (fpsSamples_.empty())
            return 0.0f;
        float sum = std::accumulate(fpsSamples_.begin(), fpsSamples_.end(), 0.0f);
        return sum / fpsSamples_.size();
    }

    bool isStable()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastStableTime_).count() >= 1.0f)
        {
            lastStableTime_ = now;
            return true;
        }
        return false;
    }

  private:
    size_t maxSamples_;
    std::deque<float> fpsSamples_;
    std::chrono::steady_clock::time_point lastTime_;
    std::chrono::steady_clock::time_point lastStableTime_;
    int stableFPS_ = 0;
};

} // namespace aion
#endif // FPS_COUNTER_H
