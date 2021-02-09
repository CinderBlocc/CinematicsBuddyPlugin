#pragma once
#include <memory>

class AnimationBuffer;
class AnimationExporter;

class AnimationRecorder
{
public:
    AnimationRecorder(std::shared_ptr<AnimationExporter> InExporter, std::shared_ptr<AnimationBuffer> InBuffer);

    void TickRecording();

private:
    AnimationRecorder() = delete;

    std::shared_ptr<AnimationExporter> Exporter;
    std::shared_ptr<AnimationBuffer> Buffer;
};
