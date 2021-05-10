#include "Model.h"

vke::VkeModel::VkeModel(VkeDevice& device, const std::vector<Vertex>& vertices) : m_Device(device)
{
	CreateVertexBuffers(vertices);
}

vke::VkeModel::~VkeModel()
{
	vkDestroyBuffer(m_Device.Device(), m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device.Device(), m_VertexBufferMemory, nullptr);
}

void vke::VkeModel::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { m_VertexBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void vke::VkeModel::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
}

void vke::VkeModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	m_VertexCount = static_cast<uint32_t>(vertices.size());
	ASSERT_LOG(m_VertexCount >= 3, "Vertex count must be at least 3!");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
	m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VertexBuffer, m_VertexBufferMemory);

	void* data;
	vkMapMemory(m_Device.Device(), m_VertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Device.Device(), m_VertexBufferMemory);
}

std::vector<VkVertexInputBindingDescription> vke::VkeModel::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> vke::VkeModel::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = 0;
	return attributeDescriptions;
}