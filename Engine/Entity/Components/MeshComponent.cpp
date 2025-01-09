#include "MeshComponent.h"

#include "Entity/Entity.h"
#include "IDComponent.h"

#include <Renderer/Core/IMesh.h>
#include <Renderer/Core/ITexture.h>

namespace Cosmos::Engine
{
	void MeshComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<MeshComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			auto& component = entity->GetComponent<MeshComponent>();
			
			if(component.mesh != nullptr) {
				if(component.mesh->IsLoaded()) {
					dataFile[uuid]["Mesh"]["Path"].SetString(component.mesh->GetPathRef());
					dataFile[uuid]["Mesh"]["Albedo"].SetString(component.mesh->GetMaterialRef().GetAlbedoTextureRef()->GetPathRef());
				}
			}
		}
	}

	void MeshComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if(dataFile.Exists("Mesh")) {
			entity->AddComponent<MeshComponent>();
			auto& component = entity->GetComponent<MeshComponent>();

			component.mesh = Renderer::IMesh::Create();
			component.mesh->LoadFromFile(dataFile["Mesh"]["Path"].GetString());
			component.mesh->GetMaterialRef().GetAlbedoTextureRef().reset();
			component.mesh->GetMaterialRef().GetAlbedoTextureRef() = Renderer::ITexture2D::Create(dataFile["Mesh"]["Albedo"].GetString());
			component.mesh->Refresh();
		}
	}
}
