#include "FrameInfoCollectors.h"
#include "SimpleJSON/json.hpp"

using namespace std::chrono;

// TIME INFO //
TimeInfo TimeInfo::Get(ReplayServerWrapper Replay)
{
    TimeInfo Output;
    
    //Assign defaults
    Output.TimeCaptured = steady_clock::now();
    Output.ReplayFrame = -1;
    
    //Fill data
    if(!Replay.IsNull())
    {
        Output.ReplayFrame = Replay.GetCurrentReplayFrame();
    }

    return Output;
}

void TimeInfo::AddToJSON(json::JSON& JSONState, const TimeInfo& FirstFrame)
{
    //FirstFrameTime is so that the time can be trimmed to the start time
    //  get the difference between this frame's CaptureTime and FirstFrame's CaptureTime
}


// CAMERA INFO //
CameraInfo CameraInfo::Get(CameraWrapper Camera)
{
    CameraInfo Output;
    
    //Assign defaults
    Output.bIsNull = true;
    Output.FOV = 90.f;

    //Fill data
    if(!Camera.IsNull())
    {
        Output.bIsNull = false;
        Output.Location = Camera.GetLocation();
        Output.Rotation = RotatorToQuat(Camera.GetRotation());
        Output.FOV = Camera.GetFOV();
    }

    return Output;
}

void CameraInfo::AddToJSON(json::JSON& JSONState)
{

}


// BALL INFO //
BallInfo BallInfo::Get(ServerWrapper Server)
{
    BallInfo Output;
    
    //Assign defaults
    Output.bIsNull = true;

    //Fill data
    BallWrapper Ball = Server.IsNull() ? BallWrapper(NULL) : Server.GetBall();
    if(!Ball.IsNull())
    {
        Output.bIsNull = false;
        Output.Location = Ball.GetLocation();
        Output.Rotation = RotatorToQuat(Ball.GetRotation());
    }

    return Output;
}

void BallInfo::AddToJSON(json::JSON& JSONState)
{

}


// WHEEL INFO //
WheelInfo WheelInfo::Get(WheelWrapper Wheel)
{
    WheelInfo Output;
    
    //Assign defaults
    Output.bIsNull = true;
    Output.SteerAmount = 0.f;
    Output.SuspensionDistance = 0.f;
    Output.SpinSpeed = 0.f;

    //Fill data
    if(Wheel.memory_address != NULL)
    {
        Output.bIsNull = false;
        Output.SteerAmount = Wheel.GetSteer2();
        Output.SuspensionDistance = Wheel.GetSuspensionDistance();
        Output.SpinSpeed = Wheel.GetSpinSpeed();
    }

    return Output;
}

void WheelInfo::AddToJSON(json::JSON& JSONState)
{

}


// CAR INFO //
CarInfo CarInfo::Get(CarWrapper Car)
{
    CarInfo Output;

    //Assign defaults
    Output.bIsNull = true;
    Output.bIsBoosting = false;
    for(auto& Wheel : Output.Wheels)
    {
        Wheel = WheelInfo::Get(WheelWrapper(NULL));
    }

    //Fill data
    if(!Car.IsNull())
    {
        Output.bIsNull = false;
        if(!Car.GetPRI().IsNull())
        {
            Output.ID = Car.GetPRI().GetUniqueIdWrapper();
        }
        Output.bIsBoosting = Car.IsBoostCheap();
        Output.Location = Car.GetLocation();
        Output.Rotation = RotatorToQuat(Car.GetRotation());
        if(Car.GetVehicleSim().memory_address != NULL)
        {
            ArrayWrapper<WheelWrapper> CarWheels = Car.GetVehicleSim().GetWheels();
            int MaxIters = min(CarWheels.Count(), 4);
            for(int i = 0; i < MaxIters; ++i)
            {
                Output.Wheels[i] = WheelInfo::Get(CarWheels.Get(i));
            }
        }
    }

    return Output;
}

void CarInfo::AddToJSON(json::JSON& JSONState, const std::vector<struct CarSeen>& AllCarsSeen)
{
    //AllCarsSeen should be provided by the AnimationExporter when writing to file
    //  That way the header identifies each unique car and gives it a number by index in this vector
    //  so each frame can use that number instead of the full UniqueIDWrapper ID
}


// CAR SEEN //
CarSeen CarSeen::Get(CarWrapper Car)
{
    CarSeen Output;

    //Assign defaults
    Output.bIsNull = true;
    Output.TimeSeen = steady_clock::now();
    Output.Body = -1;
    Output.FrontWheelRadius = 0.f;
    Output.BackWheelRadius = 0.f;

    //Fill data
    if(!Car.IsNull())
    {
        Output.bIsNull = false;
        if(!Car.GetPRI().IsNull())
        {
            Output.ID = Car.GetPRI().GetUniqueIdWrapper();
        }
        Output.Body = Car.GetLoadoutBody();
        if(Car.GetVehicleSim().memory_address != NULL)
        {
            ArrayWrapper<WheelWrapper> CarWheels = Car.GetVehicleSim().GetWheels();
            if(CarWheels.Count() >= 3)
            {
                Output.FrontWheelRadius = CarWheels.Get(0).memory_address == NULL ? 0.f : CarWheels.Get(0).GetWheelRadius();
                Output.BackWheelRadius = CarWheels.Get(2).memory_address == NULL ? 0.f : CarWheels.Get(2).GetWheelRadius();
            }
        }
    }

    return Output;
}
