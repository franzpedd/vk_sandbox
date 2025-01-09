#include "Script.h"

namespace Cosmos::Engine
{
    Script::Script(std::string name)
        : mName(name)
    {
    }

    bool Script::LoadScript(const std::string& path)
	{
        if (mSharedLib.Load(path)) {
            return false;
        }

        mScriptEntrypoint = mSharedLib.Register();

        if (mScriptEntrypoint) {
            return true;
        }

        mSharedLib.Unload();
        return false;
	}
}