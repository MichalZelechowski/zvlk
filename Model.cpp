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

        tinyobj::shape_t shape = shapes[0];
        //        for (const auto& shape : shapes) {
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
            vertex.normal = { 
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t> (this->vertices.size());
                this->vertices.push_back(vertex);
            }

            this->indices.push_back(uniqueVertices[vertex]);
        }
        //        }
        
        this->vertexBuffer.push_back(vk::Buffer());
        this->vertexBufferMemory.push_back(vk::DeviceMemory());
        this->indexBuffer.push_back(vk::Buffer());
        this->indexBufferMemory.push_back(vk::DeviceMemory());

        vk::DeviceSize bufferSize = sizeof (this->vertices[0]) * this->vertices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        device->copyMemory(bufferSize, vertices.data(), stagingBuffer, stagingBufferMemory);

        device->createBuffer(bufferSize,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                this->vertexBuffer[0], this->vertexBufferMemory[0]);

        device->copyBuffer(stagingBuffer, vertexBuffer[0], bufferSize);
        device->freeMemory(stagingBuffer, stagingBufferMemory);

        bufferSize = sizeof (indices[0]) * indices.size();
        device->copyMemory(bufferSize, indices.data(), stagingBuffer, stagingBufferMemory);

        device->createBuffer(bufferSize,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                this->indexBuffer[0], this->indexBufferMemory[0]);

        device->copyBuffer(stagingBuffer, indexBuffer[0], bufferSize);
        device->freeMemory(stagingBuffer, stagingBufferMemory);

        this->device = device;
    }

    Model::~Model() {
        for (size_t i = 0; i<this->indexBuffer.size(); i++) {
            this->device->freeMemory(this->indexBuffer[i], this->indexBufferMemory[i]);
            this->device->freeMemory(this->vertexBuffer[i], this->vertexBufferMemory[i]);
        }
    }
}

