#include "FrameInfoCollectors.h"
#include "SupportFiles/CBUtils.h"
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

float TimeInfo::GetTimeDifference(const TimeInfo& FirstFrame) const
{
    return duration_cast<duration<float>>(TimeCaptured - FirstFrame.TimeCaptured).count();
}

json::JSON TimeInfo::ConvertToJSON(const TimeInfo& FirstFrame) const
{
    json::JSON Output = json::Object();

    Output["T"] = CBUtils::PrintFloat(GetTimeDifference(FirstFrame), 4);
    Output["RF"] = ReplayFrame;

    return Output;
}

json::JSON TimeInfo::CreateExampleJSON()
{
    json::JSON Output = json::Object();

    Output["(T) Time"] = "(float) Duration since first frame in seconds";
    Output["(RF) Replay Frame"] = "(int) Replay timestamp";

    return Output;
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

json::JSON CameraInfo::ConvertToJSON() const
{
    json::JSON Output = json::Object();

    Output["L"] = CBUtils::PrintVector(Location);
    Output["R"] = CBUtils::PrintQuat(Rotation);
    Output["F"] = CBUtils::PrintFloat(FOV);
    
    return Output;
}

json::JSON CameraInfo::CreateExampleJSON()
{
    json::JSON Output = json::Object();

    Output["(L) Location"] = "(Vector) X, Y, Z";
    Output["(R) Rotation"] = "(Quat) W, X, Y, Z";
    Output["(F) FOV"] = "(float - degrees horizontal) Field of view";
    
    return Output;
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

json::JSON BallInfo::ConvertToJSON() const
{
    json::JSON Output = json::Object();

    Output["L"] = CBUtils::PrintVector(Location);
    Output["R"] = CBUtils::PrintQuat(Rotation);
    
    return Output;
}

json::JSON BallInfo::CreateExampleJSON()
{
    json::JSON Output = json::Object();

    Output["(L) Location"] = "(Vector) X, Y, Z";
    Output["(R) Rotation"] = "(Quat) W, X, Y, Z";
    
    return Output;
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

json::JSON WheelInfo::ConvertToJSON(int WheelIndex) const
{
    json::JSON Output = json::Object();

    Output[std::to_string(WheelIndex)] = CBUtils::PrintFloat(SteerAmount) + "," + CBUtils::PrintFloat(SuspensionDistance) + "," + CBUtils::PrintFloat(SpinSpeed);
    
    return Output;
}

json::JSON WheelInfo::CreateExampleJSON()
{
    json::JSON Output = json::Object();

    Output["(0-3) Wheel index"] = "(float - radians) SteerAmount, (float - degrees of axle rotation?) SuspensionDistance, (float - radians per second) SpinSpeed";
    
    return Output;
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
            Output.ID = Car.GetPRI().GetUniqueIdWrapper().GetIdString();
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

std::string CarInfo::GetCarSeenIndex(const std::vector<struct CarSeen>& AllCarsSeen) const
{
    if(!AllCarsSeen.empty())
    {
        int Index = 0;
        for(const auto& ThisCarSeen : AllCarsSeen)
        {
            if(ID == ThisCarSeen.ID)
            {
                return std::to_string(Index);
            }

            ++Index;
        }
    }

    return ID;
}

json::JSON CarInfo::ConvertToJSON() const
{
    json::JSON Output = json::Object();

    Output["B"] = bIsBoosting;
    Output["L"] = CBUtils::PrintVector(Location);
    Output["R"] = CBUtils::PrintQuat(Rotation);
    for(int i = 0; i < 4; ++i)
    {
        Output["W"][i] = Wheels[i].ConvertToJSON(i);
    }

    return Output;
}

json::JSON CarInfo::CreateExampleJSON()
{
    json::JSON Output = json::Object();

    Output["(B) IsBoosting"] = "(bool) If boost effects are playing";
    Output["(L) Location"] = "(Vector) X, Y, Z";
    Output["(R) Rotation"] = "(Quat) W, X, Y, Z";
    Output["(W) Wheels Array"][0] = WheelInfo::CreateExampleJSON();

    return Output;
}


// CAR SEEN //
CarSeen CarSeen::Get(CarWrapper Car)
{
    CarSeen Output;

    //Assign defaults
    Output.bIsNull = true;
    Output.Body = -1;
    Output.FrontWheelRadius = 0.f;
    Output.BackWheelRadius = 0.f;

    //Fill data
    if(!Car.IsNull())
    {
        Output.bIsNull = false;
        if(!Car.GetPRI().IsNull())
        {
            Output.ID = Car.GetPRI().GetUniqueIdWrapper().GetIdString();
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

json::JSON CarSeen::ConvertToJSON() const
{
    json::JSON Output = json::Object();

    Output["ID"] = ID;
    Output["Body"] = std::to_string(Body);
    Output["Front Wheel Radius"] = CBUtils::PrintFloat(FrontWheelRadius);
    Output["Back Wheel Radius"] = CBUtils::PrintFloat(BackWheelRadius);

    return Output;
}
