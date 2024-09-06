#pragma once

#include <string>
#include <vector>

namespace Cosmos
{
	// returns the binary directory path
	std::string GetBinDir();

	// returns the assets folder path
	std::string GetAssetsDir();

	// sets a new path to the assets dir
	void SetAssetsDir(std::string path);

	// returns the path of a sub-directory item that starts at the asset directory
	std::string GetAssetSubDir(std::string subpath, bool removeExtension = false);
}