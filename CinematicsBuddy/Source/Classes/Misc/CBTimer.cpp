#include "CBTimer.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

using namespace std::chrono;

CBTimer::CBTimer(const std::string& InName)
    : StartTime(steady_clock::now()), Name(InName) {}

CBTimer::~CBTimer()
{
    if(!bStopped)
    {
	    Stop();
    }
}

void CBTimer::Stop()
{
    bStopped = true;
    float Runtime = duration_cast<duration<float>>(steady_clock::now() - StartTime).count();

    //Log milliseconds
	if(Runtime < 1.f)
    {
		GlobalCvarManager->log("[" + std::to_string(Runtime * 1000.f) + "ms] " + Name);
        return;
    }
	
    //Log minutes
    if(Runtime > 60.f)
    {
		GlobalCvarManager->log("[" + std::to_string(Runtime / 60.f) + "min] " + Name);
        return;
    }
	
    //Log seconds
    GlobalCvarManager->log("[" + std::to_string(Runtime) + "s] " + Name);
}
