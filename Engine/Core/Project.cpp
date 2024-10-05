#include "Project.h"

#include "Scene.h"
#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>

namespace Cosmos::Engine
{
	Project::Project(const ProjectSettings settings)
		: mProjectSettings(settings)
	{

	}

	Project::~Project()
	{

	}

	ProjectSettings ProjectSettings::Read(std::string path)
	{
		ProjectSettings settings = {};
		Datafile file;

		if (!Datafile::Read(file, path)) {
			COSMOS_LOG(Logger::Error, "Could not load custom project settings path, returning default project");
			return settings;
		}

		// parse
		Datafile data = file["Project Settings"];
		if (data.Exists("width")) settings.width = (uint32_t)data["width"].GetInt();
		if (data.Exists("height")) settings.height = (uint32_t)data["height"].GetInt();
		if (data.Exists("fullscreen")) settings.fullscreen = data["fullscreen"].GetInt() == 1 ? true : false;
		//
		if (data.Exists("validations")) settings.validations = data["validations"].GetInt() == 1 ? true : false;
		if (data.Exists("vulkanversion")) settings.vulkanversion = data["vulkanversion"].GetString();
		if (data.Exists("msaa")) settings.msaa = (uint32_t)data["msaa"].GetInt();
		//
		if (data.Exists("enginename")) settings.enginename = data["enginename"].GetString();
		if (data.Exists("datapath")) settings.datapath = data["datapath"].GetString();
		if (data.Exists("version")) settings.version = data["version"].GetString();
		//
		if (data.Exists("language")) settings.language = data["language"].GetString();
		if (data.Exists("gamename")) settings.gamename = data["gamename"].GetString();
		if (data.Exists("builddate")) settings.builddate = data["builddate"].GetString();
		if (data.Exists("initialscene")) {
			if (data["initialscene"].GetString().compare("Default") == 0) {
				settings.initialscene = Scene::CreateDefaultScene();
			}

			else {
				std::string initialScenePath = GetAssetSubDir("Scene");
				initialScenePath.append("/");
				initialScenePath.append(data["initialscene"].GetString());
				initialScenePath.append(".scene");
				Datafile::Read(settings.initialscene, initialScenePath);
			}
		}

		return settings;
	}

	void ProjectSettings::Write(ProjectSettings settings, std::string path)
	{
		Datafile data;
		//
		data["Project Settings"]["width"].SetInt(settings.width);
		data["Project Settings"]["height"].SetInt(settings.height);
		data["Project Settings"]["fullscreen"].SetInt((int32_t)settings.fullscreen);
		//
		data["Project Settings"]["validations"].SetInt((int32_t)settings.validations);
		data["Project Settings"]["vulkanversion"].SetString(settings.vulkanversion);
		data["Project Settings"]["msaa"].SetInt(settings.msaa);
		//
		data["Project Settings"]["enginename"].SetString(settings.enginename);
		data["Project Settings"]["datapath"].SetString(settings.datapath);
		data["Project Settings"]["version"].SetString(settings.version);
		//
		data["Project Settings"]["language"].SetString(settings.language);
		data["Project Settings"]["gamename"].SetString(settings.gamename);
		data["Project Settings"]["builddate"].SetString(settings.builddate);
		data["Project Settings"]["initialscene"].SetString(settings.initialscene["Name"].GetString());
		//
		Datafile::Write(data, path);
	}
}