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

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
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

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t> (this->vertices.size());
                    this->vertices.push_back(vertex);
                }

                this->indices.push_back(uniqueVertices[vertex]);
            }
        }

        vk::DeviceSize bufferSize = sizeof (this->vertices[0]) * this->vertices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        device->copyMemory(bufferSize, vertices.data(), stagingBuffer, stagingBufferMemory);

        device->createBuffer(bufferSize, 
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                this->vertexBuffer, this->vertexBufferMemory);

        device->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
        device->freeMemory(stagingBuffer, stagingBufferMemory);

        bufferSize = sizeof (indices[0]) * indices.size();
        device->copyMemory(bufferSize, indices.data(), stagingBuffer, stagingBufferMemory);

        device->createBuffer(bufferSize, 
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                this->indexBuffer, this->indexBufferMemory);

        device->copyBuffer(stagingBuffer, indexBuffer, bufferSize);
        device->freeMemory(stagingBuffer, stagingBufferMemory);

        this->device = device;
    }

    Model::~Model() {
        this->device->freeMemory(this->indexBuffer, this->indexBufferMemory);
        this->device->freeMemory(this->vertexBuffer, this->vertexBufferMemory);
    }
}

