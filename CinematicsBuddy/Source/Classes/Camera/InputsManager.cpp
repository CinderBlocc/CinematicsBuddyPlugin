#include "InputsManager.h"
#include "SupportFiles/MacrosStructsEnums.h"

/*

    Instead of averaging forward/back inputs, use those inputs to generate an acceleration/brake along forward vector up to a certain velocity?
        - Take delta into account. Add more velocity
        - Velocity would be entirely local. If player is looking up and down while decelerating with no inputs, they'll move in a rollercoaster
        - Reduce the strength of the input the closer the velocity is to max to ease into final speed?
        - Acceleration should be a matter of duration (i.e. 1 second to max speed), so force applied should be tied to max speed

*/

InputsManager::InputsManager()
    :
bDirty(false),
bUsingGamepad(false),
bRoll(false),
MouseSensitivity(0.f),
Acceleration(0.f),
Speed(0.f),
Forward(0.f),
Strafe(0.f),
Up(0.f),
Pitch(0.f),//LookUp
Yaw(0.f),//Turn
Roll(0.f),//Keyboard: LookRoll | Gamepad: bRoll + Turn (Yaw)
LookRightScale(0.f),
LookUpScale(0.f),
GamepadLookScale(0.f) {}

void InputsManager::PlayerInputTick(float Delta)
{
    PlayerControllerWrapper Controller = GlobalGameWrapper->GetPlayerController();
	if(Controller.IsNull()) return;

    GetInputs(Controller);
    NullifyInputs(Controller);
    UpdateVelocity(Delta);
    UpdateAngularVelocity(Delta);
}

void InputsManager::GetInputs(PlayerControllerWrapper Controller)
{
    //Retrieve all the useful inputs
    bUsingGamepad = Controller.GetbUsingGamepad();
    bRoll = Controller.GetbRoll();
    Forward = Controller.GetAForward();
    Strafe = Controller.GetAStrafe();
    Up = Controller.GetAUp();
    Pitch = Controller.GetALookUp();
    Yaw = Controller.GetATurn();
    if(bUsingGamepad)
    {
        Roll = Yaw;
        Yaw = 0.f;
    }
    else
    {
        Roll = Controller.GetALookRoll();
    }
    
    //These values don't change per tick. Only update if they are out of date

    //#TODO: Get rid of these state variables grabbed from the controller? Just use custom cvars
    // Keep MouseSensitivity though, because that's set in game? Might as well make a cvar for that too since this acceleration method will be different from the game's method
    if(bDirty)
    {
        MouseSensitivity = Controller.GetMouseSensitivity();
        Acceleration = Controller.GetSpectatorCameraAccel();
        Speed = Controller.GetSpectatorCameraSpeed();
        LookRightScale = Controller.GetLookRightScale();
        LookUpScale = Controller.GetLookUpScale();
        GamepadLookScale = Controller.GetGamepadLookScale();

        bDirty = false;
    }
}

void InputsManager::NullifyInputs(PlayerControllerWrapper Controller)
{
    //Reset the inputs that cause movement or rotation
    Controller.SetAForward(0.f);
    Controller.SetAStrafe(0.f);
    Controller.SetAUp(0.f);
    Controller.SetALookUp(0.f);
    Controller.SetATurn(0.f);
    Controller.SetALookRoll(0.f);
}

void InputsManager::UpdateVelocity(float Delta)
{

}

void InputsManager::UpdateAngularVelocity(float Delta)
{
    
}

void InputsManager::DebugRender(CanvasWrapper Canvas)
{
    PlayerControllerWrapper Controller = GlobalGameWrapper->GetPlayerController();
    if(Controller.IsNull()) { return; }

    std::vector<std::string> RenderStrings;

    RenderStrings.push_back("bUsingGamepad: " + std::to_string(bUsingGamepad));
    RenderStrings.push_back("bRoll: " + std::to_string(bRoll));
    RenderStrings.push_back("MouseSensitivity: " + std::to_string(MouseSensitivity));
    RenderStrings.push_back("Acceleration: " + std::to_string(Acceleration));
    RenderStrings.push_back("Speed: " + std::to_string(Speed));
    RenderStrings.push_back("Forward: " + std::to_string(Forward));
    RenderStrings.push_back("Strafe: " + std::to_string(Strafe));
    RenderStrings.push_back("Up: " + std::to_string(Up));
    RenderStrings.push_back("Pitch: " + std::to_string(Pitch));
    RenderStrings.push_back("Yaw: " + std::to_string(Yaw));
    RenderStrings.push_back("Roll: " + std::to_string(Roll));
    RenderStrings.push_back("LookRightScale: " + std::to_string(LookRightScale));
    RenderStrings.push_back("LookUpScale: " + std::to_string(LookUpScale));
    RenderStrings.push_back("GamepadLookScale: " + std::to_string(GamepadLookScale));

	Vector2 base = {50,50};
    Canvas.SetColor(LinearColor{0, 255, 0, 255});
    for(const auto& Str : RenderStrings)
    {
        Canvas.SetPosition(base);
        Canvas.DrawString(Str);
        base.Y += 20;
    }
}

void InputsManager::ResetValues()
{
    // #TODO: Reset acceleration and speed values. Or just remove this if you're switching state variables to cvars instead of controller variables

    //Reset values to default and leave player controller alone
	PlayerControllerWrapper Controller = GlobalGameWrapper->GetPlayerController();
	if(Controller.IsNull()) return;
	Controller.SetSpectatorCameraAccel(4000.f);
	Controller.SetSpectatorCameraSpeed(2000.f);
}
