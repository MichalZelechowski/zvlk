/* 
 * File:   Model.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 17 czerwca 2020, 16:52
 */

#ifndef MODEL_H
#define MODEL_H

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>

#include "Device.h"

namespace zvlk {

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
        glm::vec3 normal;

        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription(0, sizeof (Vertex), vk::VertexInputRate::eVertex);
            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions = {
                vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
                vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
                vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)),
                vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal))
            };

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
        }
    };

    class Model {
    public:
        Model() = delete;
        Model(const Model& orig) = delete;
        Model(zvlk::Device* device, const std::string name);
        virtual ~Model();

        inline vk::Buffer getVertexBuffer() {
            return this->vertexBuffer[0];
        };

        inline vk::Buffer getIndexBuffer() {
            return this->indexBuffer[0];
        };

        inline uint32_t getNumberOfIndices() {
            return this->indices.size();
        }
    private:
        zvlk::Device* device;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<vk::Buffer> vertexBuffer;
        std::vector<vk::DeviceMemory> vertexBufferMemory;
        std::vector<vk::Buffer> indexBuffer;
        std::vector<vk::DeviceMemory> indexBufferMemory;
    };
}

namespace std {

    template<> struct hash<zvlk::Vertex> {

        size_t operator()(zvlk::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                    (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1) ^
                    (hash<glm::vec3>()(vertex.normal) << 2);
        }
    };
}

#endif /* MODEL_H */

