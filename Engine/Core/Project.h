#pragma once

#include <Common/File/Datafile.h>
#include <cstdint>

namespace Cosmos::Engine
{
	struct ProjectSettings
	{
		// window
		uint32_t width = 1366;
		uint32_t height = 768;
		bool fullscreen = false;

		// renderer
		bool validations = true;
		std::string vulkanversion = "1.2.0";
		uint32_t msaa = 2U;

		// engine
		std::string enginename = "Cosmos";
		std::string datapath = "../Data";
		std::string version = "0.0.1";

		// app
		std::string language = "English";
		std::string gamename = "Testing";
		std::string builddate = "Unknown";
		std::string initialscene = "Empty Scene";

		bool gizmo_snapping = true;
		float gizmo_snapping_value = 0.01f;
		bool grid_visible = true;

		// parse/read and returns the settings data from a file
		static ProjectSettings Read(std::string path);

		// save/write the settings of a project into a file
		static void Write(ProjectSettings settings, std::string path);
	};

	class Project
	{
	public:

		// constructor
		Project(const ProjectSettings settings);

		// destructor
		~Project();

		// returns a reference to the project settings
		inline ProjectSettings& GetSettingsRef() { return mProjectSettings; }

	private:

		ProjectSettings mProjectSettings;
	};
}