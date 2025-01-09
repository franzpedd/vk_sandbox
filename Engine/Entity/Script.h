#pragma once

#include <Platform/Core/SharedLib.h>
#include <string>

namespace Cosmos::Engine
{
	class Script
	{
	public:

		// constructor
		Script(std::string name = "My Script");

		// destructor
		~Script() = default;

		// returns a reference to the script's name
		inline std::string& GetNameRef() { return mName; }

		// returns a reference to the script's path
		inline std::string& GetPathRef() { return mPath; }

	public:

		// lodas the script given it's disk path
		bool LoadScript(const std::string& path);

	private:

		std::string mName = {};
		std::string mPath = {};
		Platform::SharedLib mSharedLib;
		Platform::Register_SharedLib_Func* mScriptEntrypoint = nullptr;
	};
}