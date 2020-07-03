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

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, name.c_str(), "/tmp/")) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<std::string, std::unordered_map<Vertex, uint32_t >> uniqueVertices({});

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

                vertex.color = {materials[shape.mesh.material_ids[0]].diffuse[0],
                    materials[shape.mesh.material_ids[0]].diffuse[1],
                    materials[shape.mesh.material_ids[0]].diffuse[2]};
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

