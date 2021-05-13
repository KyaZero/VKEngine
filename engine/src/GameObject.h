#pragma once

#include <Graphics/Model.h>
#include <memory>

namespace vke
{
	struct RigidBody2DComponent 
	{
	   glm::vec2 velocity;
	   float mass{1.0f};
	};

	struct Transform2DComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{ 1.0f, 1.0f };
		float rotation = 0.0f;

		glm::mat2 mat2()
		{
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);

			glm::mat2 rotMat{ {c, s}, {-s, c} };
			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };
			return rotMat * scaleMat;
		}
	};

	class GameObject
	{
	public:
		using id_t = unsigned int;

		static GameObject CreateGameObject()
		{
			static id_t currentId = 0;
			return GameObject(currentId++);
		}

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		id_t GetId() const { return m_Id; }

		std::shared_ptr<Model> model;
		glm::vec3 color;

		Transform2DComponent transform;
		RigidBody2DComponent rigidBody;

	private:
		GameObject(id_t objId) : m_Id(objId), model(), color() {}

		id_t m_Id;
	};
}