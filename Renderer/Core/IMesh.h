#pragma once

#include "IContext.h"
#include "Material.h"
#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>
#include <string>

namespace Cosmos::Renderer
{
	class IMesh
	{
	public:

		// creates a mesh
		static Shared<IMesh> Create();

		// constructor
		IMesh() = default;

		// destructor
		~IMesh() = default;

		// returns a reference to the file path
		inline std::string& GetPathRef() { return mPath; }

		// returns a reference to the mesh material
		inline Material& GetMaterialRef() { return mMaterial; }

		// returns if mesh was parsed and loaded into the programs memory
		inline bool IsLoaded() { return mLoaded; }

		// returns if mesh is currently selected
		inline bool IsSelected() { return mSelected; }

		// sets the mesh as selected/unselected
		inline void SetSelected(bool value) { mSelected = value; }

	public:

		// updates the mesh frame-logic
		virtual void OnUpdate(float timestep) = 0;

		// renders the mesh
		virtual void OnRender(const glm::mat4& transform, uint64_t id, IContext::Stage stage) = 0;

	public:

		// returns if mesh is currently being transfered
		virtual bool IsTransfering() = 0;

		// loads the mesh from it's filepath
		virtual void LoadFromFile(std::string path, float scale = 1.0f) = 0;

		// refreshes mesh configuration, applying any changes made
		virtual void Refresh() = 0;

	protected:

		// general info
		std::string mName = {};
		std::string mPath = {};
		Material mMaterial;
		bool mLoaded = false;
		bool mSelected = false;

		// boundaries data (untested)
		//glm::mat4 mAABB = glm::mat4(1.0f);
		//glm::vec3 mMin = glm::vec3(FLT_MAX);
		//glm::vec3 mMax = glm::vec3(-FLT_MAX);
	};
}