#include "AnimationImporter.h"
#include "SupportFiles/MacrosStructsEnums.h"

void AnimationImporter::InitCvars()
{
    MAKE_CVAR_BIND_STRING(ImportFileName, CVAR_IMPORT_FILE_NAME, "Set the import file name", true);
}
