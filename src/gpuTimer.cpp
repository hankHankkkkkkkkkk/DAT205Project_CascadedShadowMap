#include "gpuTimer.h"

#include <glad/glad.h>

#include <algorithm>

GpuFrameTimer::~GpuFrameTimer()
{
    destroy();
}

void GpuFrameTimer::initialize()
{
    if (initialized_)
    {
        return;
    }

    glGenQueries(QueryBufferCount, queryIds_);
    initialized_ = true;
}

void GpuFrameTimer::destroy()
{
    if (!initialized_)
    {
        return;
    }

    glDeleteQueries(QueryBufferCount, queryIds_);
    queryIds_[0] = 0;
    queryIds_[1] = 0;
    queryIssued_[0] = false;
    queryIssued_[1] = false;
    initialized_ = false;
}

void GpuFrameTimer::beginFrame()
{
    if (!initialized_)
    {
        return;
    }

    glBeginQuery(GL_TIME_ELAPSED, queryIds_[writeIndex_]);
    queryIssued_[writeIndex_] = true;
}

void GpuFrameTimer::endFrame(double deltaTime)
{
    if (!initialized_)
    {
        return;
    }

    glEndQuery(GL_TIME_ELAPSED);

    // Read the previous buffer only if it is ready, so performance monitoring does not stall the frame.
    if (queryIssued_[readIndex_])
    {
        int resultAvailable = 0;
        glGetQueryObjectiv(queryIds_[readIndex_], GL_QUERY_RESULT_AVAILABLE, &resultAvailable);

        if (resultAvailable)
        {
            unsigned long long elapsedNs = 0;
            glGetQueryObjectui64v(queryIds_[readIndex_], GL_QUERY_RESULT, &elapsedNs);

            sampleAccumMs_ += static_cast<double>(elapsedNs) / 1000000.0;
            ++sampleCount_;
        }
    }

    statsTimerSeconds_ += deltaTime;
    if (statsTimerSeconds_ >= StatsUpdateIntervalSeconds)
    {
        if (sampleCount_ > 0)
        {
            stats_.gpuFrameTimeMs = sampleAccumMs_ / static_cast<double>(sampleCount_);
            stats_.gpuFps = stats_.gpuFrameTimeMs > 0.0 ? 1000.0 / stats_.gpuFrameTimeMs : 0.0;
            stats_.hasGpuTiming = true;
        }

        sampleAccumMs_ = 0.0;
        sampleCount_ = 0;
        statsTimerSeconds_ = 0.0;
    }

    std::swap(writeIndex_, readIndex_);
}

const FrameStats& GpuFrameTimer::stats() const
{
    return stats_;
}
