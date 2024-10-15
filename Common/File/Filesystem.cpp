#include "Filesystem.h"

#include "Debug/Logger.h"
#include "Util/Algorithm.h"

#include <fstream>
#include <filesystem>

namespace Cosmos
{
	std::string GetBinDir()
	{
		std::string binDir = std::filesystem::current_path().string();
		Cosmos::replace(binDir.begin(), binDir.end(), '\\', '/');

		return binDir;
	}

		// default assets dir
#if defined _WIN32
	static std::string s_AssetsDir = "../Data/";
#else 
	static std::string s_AssetsDir = "Data/";
#endif

	std::string GetAssetsDir()
	{
		// assuming the folder we're in is Editor or Game
		// this allows us to load stuff on editor or game relatively, it is hacky but allow
		// files to be saved without full path directories
		return std::string{ s_AssetsDir };
	}

	void SetAssetsDir(std::string path)
	{
		if (std::filesystem::is_directory(path)) {
			s_AssetsDir = path;
		}
	}

	std::string GetAssetSubDir(std::string subpath, bool removeExtension)
	{
		std::string assets = GetAssetsDir();
		assets.append(subpath);

		if (removeExtension) {
			assets.erase(assets.begin() + assets.find_last_of('.'), assets.end());
		}

		return assets;
	}
}

