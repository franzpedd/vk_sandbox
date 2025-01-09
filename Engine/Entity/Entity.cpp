#include "Entity.h"

#include "Components/AllComponents.h"

namespace Cosmos::Engine
{
	Entity::Entity(Scene* scene, entt::entity handle)
		: mScene(scene), mHandle(handle)
	{
	}

	void Entity::Serialize(Datafile& datafile)
	{
		IDComponent::Serialize(this, datafile);
		EditorComponent::Serialize(this, datafile);
		NameComponent::Serialize(this, datafile);
		TransformComponent::Serialize(this, datafile);
		MeshComponent::Serialize(this, datafile);
	}
}