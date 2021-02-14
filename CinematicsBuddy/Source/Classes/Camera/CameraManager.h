#pragma once
#include <memory>

class CanvasWrapper;
class InputsManager;

class CameraManager
{
public:
    CameraManager();

    void PlayerInputTick(float Delta);
    void ResetValues();

    void SetbUseOverrides(bool bNewValue) { bUseOverrides = bNewValue; }

    // TESTS - REMOVE WHEN DONE //
    void DebugRender(CanvasWrapper Canvas);

private:
    bool bUseOverrides;
    std::shared_ptr<InputsManager> Inputs;
};
