/* 
 * File:   Model.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 17 czerwca 2020, 16:52
 */

#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace zvlk {

    Model::Model(zvlk::Device* device, const std::string name) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, name.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<std::string, std::unordered_map<Vertex, uint32_t>> uniqueVertices({});
        
        glm::vec3 black = {0.2f, 0.2f, 0.2f};
        glm::vec3 red = {1.0f, 0.0f, 0.0f};
        glm::vec3 white = {1.0f, 1.0f, 1.0f};
        glm::vec3 grey = {0.7f, 0.7f, 0.7f};
        glm::vec3 yellow = {0.8f, 0.8f, 0.2f};
        std::unordered_map<std::string, glm::vec3> colors({
            {"Brake_arms_001", black},
            {"Brake_pipes_001", red},
            {"Cabin_001", black},
            {"Crank_001", red},
            {"Diesel_motor_001", black},
            {"Driving_wheels_1_001", red},
            {"Driving_whees2_001", red},
            {"Frame_001", red},
            {"Loco_coupler_bar_001", black},
            {"Loco_coupler_bar_002", black},
            {"Loco_coupler_hook_001", black},
            {"Loco_coupler_hook_002", black},
            {"Wagon_coupler_link1_002", black},
            {"Wagon_coupler_link2_003", black},
            {"Loco_coupler_link2_003", black},
            {"Loco_coupler_link_002", black},
            {"Loco_coupler_link_001", black},
            {"Loco_coupler_link2_001", black},
            {"Plates_001", white},
            {"Plates_003", white},
            {"Side_rod_001", red},
            {"Side_rod_003", red},
            {"Tracks_001", grey},
            {"Wagon_body_001", yellow},
            {"Wagon_coupler_hook_001", black},
            {"Wagon_coupler_link1_001", black},
            {"Wagon_coupler_bar_001", black},
            {"Wagon_coupler_link2_001", black},
            {"Wagon_coupler_bar_002", black},
            {"Wagon_door1_001", yellow},
            {"Wagon_door2_001", yellow},
            {"Wagon_door2_002", yellow},
            {"Wagon_door1_003", yellow},
            {"Wagon_wheel_001", grey},
            {"Wagon_wheel_002", grey},
        });
        

        for (const auto& shape : shapes) {
            uniqueVertices[shape.name] = std::unordered_map<Vertex, uint32_t>();
            this->indices[shape.name] = std::vector<uint32_t>();
            this->vertices[shape.name] = std::vector<Vertex>();
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = colors[shape.name];
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

                if (uniqueVertices[shape.name].count(vertex) == 0) {
                    uniqueVertices[shape.name][vertex] = static_cast<uint32_t> (this->vertices[shape.name].size());
                    this->vertices[shape.name].push_back(vertex);
                }

                this->indices[shape.name].push_back(uniqueVertices[shape.name][vertex]);
            }

            this->vertexBuffer[shape.name] = vk::Buffer();
            this->vertexBufferMemory[shape.name] = vk::DeviceMemory();
            this->indexBuffer[shape.name] = vk::Buffer();
            this->indexBufferMemory[shape.name] = vk::DeviceMemory();

            vk::DeviceSize bufferSize = sizeof (this->vertices[shape.name][0]) * this->vertices[shape.name].size();

            vk::Buffer stagingBuffer;
            vk::DeviceMemory stagingBufferMemory;

            device->copyMemory(bufferSize, vertices[shape.name].data(), stagingBuffer, stagingBufferMemory);

            device->createBuffer(bufferSize,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                    this->vertexBuffer[shape.name], this->vertexBufferMemory[shape.name]);

            device->copyBuffer(stagingBuffer, vertexBuffer[shape.name], bufferSize);
            device->freeMemory(stagingBuffer, stagingBufferMemory);

            bufferSize = sizeof (indices[shape.name][0]) * indices[shape.name].size();
            device->copyMemory(bufferSize, indices[shape.name].data(), stagingBuffer, stagingBufferMemory);

            device->createBuffer(bufferSize,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                    this->indexBuffer[shape.name], this->indexBufferMemory[shape.name]);

            device->copyBuffer(stagingBuffer, indexBuffer[shape.name], bufferSize);
            device->freeMemory(stagingBuffer, stagingBufferMemory);
        }


        this->device = device;
    }

    Model::~Model() {
        std::vector<std::string> names = this->getPartNames();
        for (auto& name : names) {
            this->device->freeMemory(this->indexBuffer[name], this->indexBufferMemory[name]);
            this->device->freeMemory(this->vertexBuffer[name], this->vertexBufferMemory[name]);
        }
    }
}

