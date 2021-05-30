#include "InputsManager.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "UI/UIManager.h"

InputsManager::InputsManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    UI->AddElement({bInvertPitch,    CVAR_INVERT_PITCH,        "Invert Pitch (Controller)", "Inverts pitch values for the controller"                   });
    UI->AddElement({RollBinding,     CVAR_ROLL_BINDING,        "Toggle Roll Binding",       "Modifier to swap an input axis with roll"                  });
    UI->AddElement({FOVBinding,      CVAR_FOV_BINDING,         "Toggle FOV Binding",        "Modifier to swap an input axis with FOV"                   });
    UI->AddElement({RollSwapChoice,  CVAR_ROLL_SWAP,           "Roll Input Swap",           "Which axis to swap with Roll"                              });
    UI->AddElement({FOVSwapChoice,   CVAR_FOV_SWAP,            "FOV Input Swap",            "Which axis to swap with FOV"                               });
    UI->AddElement({bFreeze,         CVAR_CAM_FREEZE,          "Freeze",                    "Block inputs to camera"                                    });
    UI->AddElement({bFreezeExternal, CVAR_CAM_FREEZE_EXTERNAL, "##FreezeExternal",          "Sync freeze with other plugins", -1000001, -1000001, false });

    //Add options to dropdown menus
    SetBindingOptions();
    SetInputSwapOptions();

    //Bind addOnValueChanged functions to the input bindings
    ON_CVAR_CHANGED(CVAR_CAM_FREEZE_EXTERNAL, InputsManager::OnFreezeExternalChanged);
    ON_CVAR_CHANGED(CVAR_ROLL_BINDING, InputsManager::CacheRollBinding);
    ON_CVAR_CHANGED(CVAR_FOV_BINDING, InputsManager::CacheFOVBinding);
    ON_CVAR_CHANGED(CVAR_CAM_FREEZE, InputsManager::OnFreezeChanged);
    ON_CVAR_CHANGED(CVAR_ROLL_SWAP, InputsManager::CacheRollSwap);
    ON_CVAR_CHANGED(CVAR_FOV_SWAP, InputsManager::CacheFOVSwap);
    CacheRollBinding();
    CacheFOVBinding();
    CacheRollSwap();
    CacheFOVSwap();
}

// GET AND MODIFY INPUTS //
void InputsManager::PlayerInputTick(float Delta)
{
    PlayerControllerWrapper Controller = GlobalGameWrapper->GetPlayerController();
	if(Controller.IsNull()) return;

    if(*bFreeze)
    {
        NullifyStoredInputs();
    }
    else
    {
        GetInputs(Controller);
    }

    NullifyGameInputs(Controller);
}

void InputsManager::ResetInputs(bool bResetMomentum)
{
    NullifyStoredInputs();

    if(bResetMomentum)
    {
        GlobalCvarManager->executeCommand(NOTIFIER_CAM_RESET, false);
    }
}

void InputsManager::GetInputs(PlayerControllerWrapper Controller)
{
    bRoll = GlobalGameWrapper->IsKeyPressed(RollBindingIndex);
    bFOV  = GlobalGameWrapper->IsKeyPressed(FOVBindingIndex);

    //Retrieve all the state values
    bUsingGamepad = Controller.GetbUsingGamepad();

    //Retrieve the inputs
    Forward = Controller.GetAForward();
    Right   = Controller.GetAStrafe();
    Up      = Controller.GetAUp();
    Pitch   = bUsingGamepad ? Controller.GetALookUp() : Controller.GetALookUp() * .0025f;
    Yaw     = bUsingGamepad ? Controller.GetATurn()   : Controller.GetATurn()   * .0025f;
    Roll    = 0.f;
    FOV     = 0.f;
    if(bUsingGamepad)
    {
        if(bRoll)
        {
            DoSwap(RollSwap, Roll);
        }
        if(bFOV)
        {
            DoSwap(FOVSwap, FOV);
        }
        if(*bInvertPitch)
        {
            Pitch *= -1.f;
        }
    }
    else
    {
        //When pressing keyboard roll buttons, roll is +- 83.3333 which is the same as 250 / 3
        //Compress it to -1 to 1 range to match controller inputs
        constexpr float LookRollRate = 250.f / 3.f;
        Roll = Controller.GetALookRoll() / LookRollRate;

        //Sometimes with keyboard the roll input will go slightly above or below 1 and -1
        //Clamp it to -1 to 1 range
        if(abs(Roll) > 1.f)
        {
            Roll /= abs(Roll);
        }
    }
}

void InputsManager::NullifyGameInputs(PlayerControllerWrapper Controller)
{
    Controller.SetAForward(0.f);
    Controller.SetAStrafe(0.f);
    Controller.SetAUp(0.f);
    Controller.SetALookUp(0.f);
    Controller.SetATurn(0.f);
    Controller.SetALookRoll(0.f);
}

void InputsManager::NullifyStoredInputs()
{
    Forward = 0.f;
    Right   = 0.f;
    Up      = 0.f;
    Pitch   = 0.f;
    Yaw     = 0.f;
    Roll    = 0.f;
    FOV     = 0.f;
}

void InputsManager::OnFreezeChanged()
{
    //If freeze has been turned on, stop all momentum
    ResetInputs(true);

    //Don't notify other plugins about the change if it was modified externally
    if(bWasFreezeExternallyChanged)
    {
        bWasFreezeExternallyChanged = false;
        return;
    }

    //Sync camera freeze with CameraLock plugin
    CVarWrapper CameraLockSync = GlobalCvarManager->getCvar("CameraLock_Enable_EXTERNAL");
    if(!CameraLockSync.IsNull())
    {
        CameraLockSync.setValue(*bFreeze);
    }
}

void InputsManager::OnFreezeExternalChanged()
{
    //To avoid infinitely changing values back and forth, only have external plugins set this cvar
    bWasFreezeExternallyChanged = true;
    GlobalCvarManager->getCvar(CVAR_CAM_FREEZE).setValue(*bFreezeExternal);
}


// HANDLE INPUT SWAPPING //
void InputsManager::SetBindingOptions()
{
    UIElement::DropdownOptionsType BindingsList;
    BindingsList.emplace_back(NO_SELECTION, NO_SELECTION);
    BindingsList.emplace_back("Left thumbstick press", "XboxTypeS_LeftThumbStick");
    BindingsList.emplace_back("Right thumbstick press", "XboxTypeS_RightThumbStick");
    BindingsList.emplace_back("DPad up", "XboxTypeS_DPad_Up");
    BindingsList.emplace_back("DPad left", "XboxTypeS_DPad_Left");
    BindingsList.emplace_back("DPad right", "XboxTypeS_DPad_Right");
    BindingsList.emplace_back("DPad down", "XboxTypeS_DPad_Down");
    BindingsList.emplace_back("Back button", "XboxTypeS_Back");
    BindingsList.emplace_back("Start button", "XboxTypeS_Start");
    BindingsList.emplace_back("Xbox Y - PS4 Triangle", "XboxTypeS_Y");
    BindingsList.emplace_back("Xbox X - PS4 Square", "XboxTypeS_X");
    BindingsList.emplace_back("Xbox B - PS4 Circle", "XboxTypeS_B");
    BindingsList.emplace_back("Xbox A - PS4 X", "XboxTypeS_A");
    BindingsList.emplace_back("Xbox LB - PS4 L1", "XboxTypeS_LeftShoulder");
    BindingsList.emplace_back("Xbox RB - PS4 R1", "XboxTypeS_RightShoulder");
    BindingsList.emplace_back("Xbox LT - PS4 L2", "XboxTypeS_LeftTrigger");
    BindingsList.emplace_back("Xbox RT - PS4 R2", "XboxTypeS_RightTrigger");
    BindingsList.emplace_back("Left thumbstick X axis", "XboxTypeS_LeftX");
    BindingsList.emplace_back("Left thumbstick Y axis", "XboxTypeS_LeftY");
    BindingsList.emplace_back("Right thumbstick X axis", "XboxTypeS_RightX");
    BindingsList.emplace_back("Right thumbstick Y axis", "XboxTypeS_RightY");

    UI->EditElement(CVAR_ROLL_BINDING).AddDropdownOptions(BindingsList);
    UI->EditElement(CVAR_FOV_BINDING).AddDropdownOptions(BindingsList);
}

void InputsManager::SetInputSwapOptions()
{
    UIElement::DropdownOptionsType OptionsList;

    OptionsList.emplace_back(NO_SELECTION, NO_SELECTION);
    OptionsList.emplace_back("Forward", "Forward");
    OptionsList.emplace_back("Right",   "Right");
    OptionsList.emplace_back("Up",      "Up");
    OptionsList.emplace_back("Pitch",   "Pitch");
    OptionsList.emplace_back("Yaw",     "Yaw");

    UI->EditElement(CVAR_ROLL_SWAP).AddDropdownOptions(OptionsList);
    UI->EditElement(CVAR_FOV_SWAP).AddDropdownOptions(OptionsList);
}

void InputsManager::CacheRollBinding()
{
    RollBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*RollBinding);
}

void InputsManager::CacheFOVBinding()
{
    FOVBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*FOVBinding);
}

void InputsManager::CacheRollSwap()
{
    RollSwap = GetSwapType(*RollSwapChoice);
}

void InputsManager::CacheFOVSwap()
{
    FOVSwap = GetSwapType(*FOVSwapChoice);
}

void InputsManager::DoSwap(EInputSwapType SwapType, float& ValueToSwap)
{
    switch(SwapType)
    {
        case EInputSwapType::S_Forward:
        {
            ValueToSwap = Forward;
            Forward = 0.f;
            break;
        }
        case EInputSwapType::S_Right:
        {
            ValueToSwap = Right;
            Right = 0.f;
            break;
        }
        case EInputSwapType::S_Up:
        {
            ValueToSwap = Up;
            Up = 0.f;
            break;
        }
        case EInputSwapType::S_Pitch:
        {
            ValueToSwap = Pitch;
            Pitch = 0.f;
            break;
        }
        case EInputSwapType::S_Yaw:
        {
            ValueToSwap = Yaw;
            Yaw = 0.f;
            break;
        }
    }
}

EInputSwapType InputsManager::GetSwapType(const std::string& InTypeString)
{
    if(InTypeString == "Forward") { return EInputSwapType::S_Forward; }
    if(InTypeString == "Right")   { return EInputSwapType::S_Right;   }
    if(InTypeString == "Up")      { return EInputSwapType::S_Up;      }
    if(InTypeString == "Pitch")   { return EInputSwapType::S_Pitch;   }
    if(InTypeString == "Yaw")     { return EInputSwapType::S_Yaw;     }

    return EInputSwapType::S_NONE;
}
