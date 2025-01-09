#include "Project.h"

#include <Common/Debug/Logger.h>

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

		// project settings
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
		if (data.Exists("initialscene")) settings.initialscene = data["initialscene"].GetString();

		// editor settings
		data = file["Editor Settings"];
		if (data.Exists("gizmo-snapping")) settings.gizmo_snapping = (uint32_t)data["gizmo-snapping"].GetInt() == 1 ? true : false;
		if (data.Exists("gizmo-snapping-value")) settings.gizmo_snapping_value = (float)data["gizmo-snapping-value"].GetDouble();
		if (data.Exists("grid-visible")) settings.grid_visible = data["grid-visible"].GetInt() == 1 ? true : false;

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
		data["Project Settings"]["initialscene"].SetString(settings.initialscene);
		//

		data["Editor Settings"]["gizmo-snapping"].SetInt(settings.gizmo_snapping);
		data["Editor Settings"]["gizmo-snapping-value"].SetDouble(settings.gizmo_snapping_value);
		data["Editor Settings"]["grid-visible"].SetInt((int32_t)settings.fullscreen);

		Datafile::Write(data, path);
	}
}