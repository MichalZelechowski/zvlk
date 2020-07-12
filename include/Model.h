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
#include "Frame.h"
#include "Material.h"

namespace zvlk {

    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec3 normal;

        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription(0, sizeof (Vertex), vk::VertexInputRate::eVertex);
            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {
                vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
                vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)),
                vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal))
            };

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return position == other.position && texCoord == other.texCoord && normal == other.normal;
        }
    };

    class Model {
    public:
        Model() = delete;
        Model(const Model& orig) = delete;
        Model(zvlk::Device* device, const std::string name, zvlk::Frame* frame);
        virtual ~Model();

        inline std::vector<std::string> getPartNames() {
            return names;
        }
        
        inline vk::Buffer getVertexBuffer(std::string name) {
            return this->vertexBuffer[name];
        };

        inline vk::Buffer getIndexBuffer(std::string name) {
            return this->indexBuffer[name];
        };

        inline uint32_t getNumberOfIndices(std::string name) {
            return this->indices[name].size();
        }
        
        inline zvlk::Material* getMaterial(uint32_t index) {
            return this->materialMapping[this->names[index]];
        }
        
    private:
        zvlk::Device* device;
        std::vector<zvlk::Material*> materials;
        std::vector<std::string> names;
        std::unordered_map<std::string, Material*> materialMapping;
        std::unordered_map<std::string, std::vector<Vertex>> vertices;
        std::unordered_map<std::string, std::vector<uint32_t>> indices;
        std::unordered_map<std::string, vk::Buffer> vertexBuffer;
        std::unordered_map<std::string, vk::Buffer> indexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        vk::DeviceMemory indexBufferMemory;
    };
}

namespace std {

    template<> struct hash<zvlk::Vertex> {

        size_t operator()(zvlk::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1) ^
                    (hash<glm::vec3>()(vertex.normal) << 2)));
        }
    };
}

#endif /* MODEL_H */

