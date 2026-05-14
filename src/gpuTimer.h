#ifndef GPU_TIMER_H
#define GPU_TIMER_H

struct FrameStats
{
    double gpuFrameTimeMs = 0.0;
    double gpuFps = 0.0;
    bool hasGpuTiming = false;
};

class GpuFrameTimer
{
public:
    GpuFrameTimer() = default;
    ~GpuFrameTimer();

    GpuFrameTimer(const GpuFrameTimer&) = delete;
    GpuFrameTimer& operator=(const GpuFrameTimer&) = delete;

    void initialize();
    void destroy();

    void beginFrame();
    void endFrame(double deltaTime);

    const FrameStats& stats() const;

private:
    static constexpr int QueryBufferCount = 2;
    static constexpr double StatsUpdateIntervalSeconds = 0.5;

    unsigned int queryIds_[QueryBufferCount] = {};
    int writeIndex_ = 0;
    int readIndex_ = 1;

    bool initialized_ = false;
    bool queryIssued_[QueryBufferCount] = {};

    double sampleAccumMs_ = 0.0;
    double statsTimerSeconds_ = 0.0;
    int sampleCount_ = 0;

    FrameStats stats_;
};

#endif
