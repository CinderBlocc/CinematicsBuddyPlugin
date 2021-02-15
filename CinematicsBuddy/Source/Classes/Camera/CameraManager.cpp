#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"

CameraManager::CameraManager()
    :
Inputs(std::make_shared<InputsManager>()),
bUseOverrides(false),
bRoll(false),
MovementSpeed(1.f),
MovementAccel(1.f),
RotationAccel(1.f),
MouseSensitivity(10.f),
GamepadSensitivity(20.f),
FOVRotationScale(.9f) {}

void CameraManager::PlayerInputTick(float Delta, bool InbRoll)
{
    if(!bUseOverrides)
    {
        return;
    }

    //Get the inputs and then nullify them before they reach the game
    bRoll = InbRoll;
	Inputs->PlayerInputTick(Delta, bRoll);

    //Apply inputs to camera
    UpdateCameraTransformation(Delta);
}

void CameraManager::UpdateCameraTransformation(float Delta)
{
    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(!TheCamera.IsNull())
    {
        UpdateCameraMatrix(TheCamera);
        UpdateVelocity(Delta);
        UpdateAngularVelocity(Delta);
        UpdatePosition(Delta, TheCamera);
        UpdateRotation(Delta, TheCamera);
    }
}

void CameraManager::UpdateCameraMatrix(CameraWrapper TheCamera)
{
    Quat CameraQuat = RotatorToQuat(TheCamera.GetRotation());
    Forward = RotateVectorWithQuat(Vector(1, 0, 0), CameraQuat);
    Right   = RotateVectorWithQuat(Vector(0, 1, 0), CameraQuat);
    Up      = RotateVectorWithQuat(Vector(0, 0, 1), CameraQuat);
}

void CameraManager::UpdateVelocity(float Delta)
{
    //Calculate the max speed
    constexpr float BaseSpeed = 2000.f;
    float MaxSpeed = BaseSpeed * MovementSpeed;

    //Get useful percentages
    float SpeedPerc   = Velocity.magnitude() / MaxSpeed;
    float ForwardPerc = Inputs->GetForward();
    float RightPerc   = Inputs->GetRight();
    float UpPerc      = Inputs->GetUp();

    //#TODO: Braking force should be applied per vector component, and should be the inverse of that component's input amount
    //i.e. If forward input is 0.25, forward braking should be 0.75

    //#TODO: Calculate an impulse amount based on delta?
    //As Velocity approaches MaxVelocity, reduce impulse strength to ease into max speed?
    //float BrakeImpulse = MaxSpeed * Delta * (1 - SpeedPerc) * Acceleration;

    //Get relative input velocity direction. NOTE: Won't be normalized
    //#TODO: Normalize this? Might be problematic in slowing down forward momentum while holding right, etc
    Vector AccelForward = Forward * ForwardPerc;
    Vector AccelRight   = Right   * RightPerc;
    Vector AccelUp      = Up      * UpPerc;
    Vector AccelVelocity = AccelForward + AccelRight + AccelUp;
    //Vector BrakeForward = Forward * (1 - ForwardPerc) * -1.f;
    //Vector BrakeRight   = Right   * (1 - RightPerc)   * -1.f;
    //Vector BrakeUp      = Up      * (1 - UpPerc)      * -1.f;
    //Vector BrakeVelocity = BrakeForward + BrakeRight + BrakeUp;

    //Apply impulse strength to directional vector
    float AccelImpulse = MaxSpeed * Delta * (1 - SpeedPerc) * MovementAccel;
    AccelVelocity *= AccelImpulse;
    //BrakeVelocity *= BrakeImpulse;

    //#TODO: Add this new velocity to the original velocity, don't directly set Velocity = Additional
    Velocity += AccelVelocity;

    //Apply brake
    float BrakeAmount = min(Delta * 10.f, 1.f);
    if(abs(AccelVelocity.magnitude()) < 0.001f)
    {
        Velocity -= Velocity * BrakeAmount;
    }
    //Velocity += BrakeVelocity;


    //Calculate a brake force to automatically slow down the camera
    //Vector BrakeForce = Velocity * -1.f * Delta * Acceleration;
    //Velocity += BrakeForce;
}

void CameraManager::UpdateAngularVelocity(float Delta)
{
    //#TODO: Take FOVRotationScale into account here, along with camera FOV
}

void CameraManager::UpdatePosition(float Delta, CameraWrapper TheCamera)
{
    Vector NewLocation = TheCamera.GetLocation() + Velocity * Delta;
    TheCamera.SetLocation(NewLocation);
}

void CameraManager::UpdateRotation(float Delta, CameraWrapper TheCamera)
{
    
}


// TESTS - REMOVE WHEN DONE //
void CameraManager::DebugRender(CanvasWrapper Canvas)
{
    if(!bUseOverrides)
    {
        return;
    }
    
    //Create RenderStrings and fill it with some values
    std::vector<std::string> RenderStrings;
    RenderStrings.push_back("bRoll: "              + std::to_string(bRoll));
    RenderStrings.push_back("MovementSpeed: "      + std::to_string(MovementSpeed));
    RenderStrings.push_back("MovementAccel: "      + std::to_string(MovementAccel));
    RenderStrings.push_back("RotationAccel: "      + std::to_string(RotationAccel));
    RenderStrings.push_back("MouseSensitivity: "   + std::to_string(MouseSensitivity));
    RenderStrings.push_back("GamepadSensitivity: " + std::to_string(GamepadSensitivity));
    RenderStrings.push_back("FOVRotationScale: "   + std::to_string(FOVRotationScale));

    //Get values from InputsManager
    Inputs->DebugRender(Canvas, RenderStrings);

    RenderStrings.emplace_back("");
    RenderStrings.push_back("Velocity: " + CBUtils::PrintFloat(Velocity.magnitude(), 6));
    RenderStrings.push_back("Velocity Components: " + CBUtils::PrintVector(Velocity, 4));
    RenderStrings.push_back("AngularVelocity: " + CBUtils::PrintVector(AngularVelocity, 6));


    //Draw black box behind text
    Canvas.SetPosition(Vector2{50,50});
    Canvas.SetColor(LinearColor{0,0,0,200});
    Canvas.FillBox(Vector2{300,400});

    //Draw text
	Vector2 base = {75,75};
    Canvas.SetColor(LinearColor{0, 255, 0, 255});
    for(const auto& Str : RenderStrings)
    {
        Canvas.SetPosition(base);
        Canvas.DrawString(Str);
        base.Y += 20;
    }
}
