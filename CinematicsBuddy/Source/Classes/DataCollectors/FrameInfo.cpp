#include "FrameInfo.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SimpleJSON/json.hpp"

using namespace std::chrono;

FrameInfo FrameInfo::Get()
{
    FrameInfo Output;

    ReplayServerWrapper ReplayServer = GlobalGameWrapper->GetGameEventAsReplay();
    ServerWrapper ActualServer = GlobalGameWrapper->GetCurrentGameState();
    CameraWrapper Camera = GlobalGameWrapper->GetCamera();
    
    Output.ReplayID = "NULL";
    if(GlobalGameWrapper->IsInReplay() && !ReplayServer.IsNull())
	{
		ReplayWrapper Replay = ReplayServer.GetReplay();
		if(Replay.memory_address != NULL)
        {
            UnrealStringWrapper ReplayID = Replay.GetId();
			Output.ReplayID = ReplayID.IsNull() ? "NULL" : ReplayID.ToString();
        }
	}

    Output.TimeData = TimeInfo::Get(ReplayServer);
    Output.CameraData = CameraInfo::Get(Camera);
    Output.BallData = BallInfo::Get(ActualServer);
    if(!ActualServer.IsNull())
    {
        ArrayWrapper<CarWrapper> Cars = ActualServer.GetCars();
        for(int i = 0; i < Cars.Count(); ++i)
        {
            CarWrapper ThisCar = Cars.Get(i);
            if(ThisCar.IsNull()) { continue; }

            Output.CarData.push_back(CarInfo::Get(ThisCar));
            Output.CarsSeenData.push_back(CarSeen::Get(ThisCar));
        }
    }

    return Output;
}

std::string FrameInfo::Print(const TimeInfo& FirstFrame, const std::vector<CarSeen>& AllCarsSeen) const
{
    //FirstFrame is so that the time can be trimmed to the start time
    //  get the difference between this frame's CaptureTime and FirstFrame's CaptureTime

    //AllCarsSeen should be provided by the AnimationExporter when writing to file
    //  That way the header identifies each unique car and gives it a number by index in this vector
    //  so each frame can use that number instead of the full UniqueIDWrapper ID

    json::JSON Output = json::Object();

    return "";
}
