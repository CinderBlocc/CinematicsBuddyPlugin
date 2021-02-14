#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"

CameraManager::CameraManager()
    : bUseOverrides(false), Inputs(std::make_shared<InputsManager>()) {}

void CameraManager::PlayerInputTick(float Delta)
{
	Inputs->PlayerInputTick(Delta);
}

void CameraManager::DebugRender(CanvasWrapper Canvas)
{
    if(bUseOverrides)
    {
        Inputs->DebugRender(Canvas);
    }
}

void CameraManager::ResetValues()
{
    Inputs->ResetValues();
}
