#include "InputsManager.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "UI/UIManager.h"

InputsManager::InputsManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    UI->AddElement(UIElement(bRollReplacesPitch, CVAR_ROLL_REPLACES_PITCH, "Roll replaces pitch instead of yaw", "Roll binding replaces pitch instead of yaw"));

    //MAKE_CVAR_BIND_TO_STRING(bRollReplacesPitch, CVAR_ROLL_REPLACES_PITCH, "Roll binding replaces pitch instead of yaw", true);
}

void InputsManager::PlayerInputTick(float Delta, bool bRoll)
{
    PlayerControllerWrapper Controller = GlobalGameWrapper->GetPlayerController();
	if(Controller.IsNull()) return;

    GetInputs(Controller, bRoll);
    NullifyInputs(Controller);
}

void InputsManager::GetInputs(PlayerControllerWrapper Controller, bool bRoll)
{
    //Retrieve all the state values
    bUsingGamepad = Controller.GetbUsingGamepad();

    //Retrieve the inputs
    Forward = Controller.GetAForward();
    Right = Controller.GetAStrafe();
    Up = Controller.GetAUp();
    Pitch = Controller.GetALookUp();
    Yaw = Controller.GetATurn();
    Roll = 0.f;
    if(bUsingGamepad)
    {
        if(bRoll)
        {
            if(*bRollReplacesPitch)
            {
                Roll = Pitch;
                Pitch = 0.f;
            }
            else
            {
                Roll = Yaw;
                Yaw = 0.f;
            }
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

    HandleTest(); // TESTS - REMOVE WHEN DONE //
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


// TESTS - REMOVE WHEN DONE //
void InputsManager::HandleTest()
{
    if(bTestIsRunning)
    {
        using namespace std::chrono;
        float TestTime = duration_cast<duration<float>>(steady_clock::now() - TestStartTime).count();
        if(TestTime >= 0.f && TestTime < 5.f)
        {
            Forward = -1.f;
        }
        else if(TestTime >= 5.f && TestTime < 10.f)
        {
            Forward = 0.f;
        }
        else if(TestTime >= 10.f && TestTime < 15.f)
        {
            Forward = 1.f;
        }
        else if(TestTime >= 15.f && TestTime < 20.f)
        {
            Forward = -1.f;
        }
        else if(TestTime >= 20.f)
        {
            bTestIsRunning = false;
        }
    }
}
