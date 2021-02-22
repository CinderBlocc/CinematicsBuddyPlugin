#include "InputsManager.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "UI/UIManager.h"

InputsManager::InputsManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    UI->AddElement({m_RollBinding,    CVAR_ROLL_BINDING, "Toggle roll binding",       "Modifier to swap an input axis with roll" });
    UI->AddElement({m_FOVBinding,     CVAR_FOV_BINDING,  "Toggle FOV binding",        "Modifier to swap an input axis with FOV"  });
    UI->AddElement({m_RollSwapChoice, CVAR_ROLL_SWAP,    "Roll Input Swap",           "Which axis to swap with Roll"  });
    UI->AddElement({m_FOVSwapChoice,  CVAR_FOV_SWAP,     "FOV Input Swap",            "Which axis to swap with FOV"  });
    UI->AddElement({m_bInvertPitch,   CVAR_INVERT_PITCH, "Invert pitch (controller)", "Inverts pitch values for the controller"  });

    //Add options to dropdown menus
    SetBindingOptions();
    SetInputSwapOptions();

    //Bind addOnValueChanged functions to the input bindings
    ON_CVAR_CHANGED(CVAR_ROLL_BINDING, InputsManager::CacheRollBinding);
    ON_CVAR_CHANGED(CVAR_FOV_BINDING, InputsManager::CacheFOVBinding);
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

    GetInputs(Controller);
    NullifyInputs(Controller);
}

void InputsManager::GetInputs(PlayerControllerWrapper Controller)
{
    m_bRoll = GlobalGameWrapper->IsKeyPressed(m_RollBindingIndex);
    m_bFOV  = GlobalGameWrapper->IsKeyPressed(m_FOVBindingIndex);

    //Retrieve all the state values
    m_bUsingGamepad = Controller.GetbUsingGamepad();

    //Retrieve the inputs
    m_Forward = Controller.GetAForward();
    m_Right   = Controller.GetAStrafe();
    m_Up      = Controller.GetAUp();
    m_Pitch   = Controller.GetALookUp();
    m_Yaw     = Controller.GetATurn();
    m_Roll    = 0.f;
    m_FOV     = 0.f;
    if(m_bUsingGamepad)
    {
        if(m_bRoll)
        {
            DoSwap(m_RollSwap, m_Roll);
        }
        if(m_bFOV)
        {
            DoSwap(m_FOVSwap, m_FOV);
        }
        if(*m_bInvertPitch)
        {
            m_Pitch *= -1.f;
        }
    }
    else
    {
        //When pressing keyboard roll buttons, roll is +- 83.3333 which is the same as 250 / 3
        //Compress it to -1 to 1 range to match controller inputs
        constexpr float LookRollRate = 250.f / 3.f;
        m_Roll = Controller.GetALookRoll() / LookRollRate;

        //Sometimes with keyboard the roll input will go slightly above or below 1 and -1
        //Clamp it to -1 to 1 range
        if(abs(m_Roll) > 1.f)
        {
            m_Roll /= abs(m_Roll);
        }
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
    m_RollBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*m_RollBinding);
}

void InputsManager::CacheFOVBinding()
{
    m_FOVBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*m_FOVBinding);
}

void InputsManager::CacheRollSwap()
{
    m_RollSwap = GetSwapType(*m_RollSwapChoice);
}

void InputsManager::CacheFOVSwap()
{
    m_FOVSwap = GetSwapType(*m_FOVSwapChoice);
}

void InputsManager::DoSwap(EInputSwapType SwapType, float& ValueToSwap)
{
    switch(SwapType)
    {
        case EInputSwapType::S_Forward:
        {
            ValueToSwap = m_Forward;
            m_Forward = 0.f;
            break;
        }
        case EInputSwapType::S_Right:
        {
            ValueToSwap = m_Right;
            m_Right = 0.f;
            break;
        }
        case EInputSwapType::S_Up:
        {
            ValueToSwap = m_Up;
            m_Up = 0.f;
            break;
        }
        case EInputSwapType::S_Pitch:
        {
            ValueToSwap = m_Pitch;
            m_Pitch = 0.f;
            break;
        }
        case EInputSwapType::S_Yaw:
        {
            ValueToSwap = m_Yaw;
            m_Yaw = 0.f;
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
