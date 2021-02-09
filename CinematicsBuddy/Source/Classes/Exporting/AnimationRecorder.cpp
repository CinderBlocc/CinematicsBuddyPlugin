#include "AnimationRecorder.h"
#include "AnimationExporter.h"
#include "AnimationBuffer.h"
#include "DataCollectors/FrameInfo.h"

AnimationRecorder::AnimationRecorder(std::shared_ptr<AnimationExporter> InExporter, std::shared_ptr<AnimationBuffer> InBuffer)
    : Exporter(InExporter), Buffer(InBuffer) {}

void AnimationRecorder::TickRecording()
{
    if(!Exporter || !Buffer)
    {
        return;
    }

    if(Exporter->GetbIsRecording() || Buffer->GetbIsRecording())
    {
        const FrameInfo& ThisFrame = FrameInfo::Get();

        if(Exporter->GetbIsRecording())
        {
            Exporter->AddData(ThisFrame);
        }

        if(Buffer->GetbIsRecording())
        {
            Buffer->AddData(ThisFrame);
            Buffer->CleanOutdatedData();
        }
    }
}
