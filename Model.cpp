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

        vk::DeviceSize vertexBufferSize = 0;
        vk::DeviceSize indicesBufferSize = 0;
        vk::DeviceSize maxStagingBufferSize = 0;
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

            vertexBufferSize += sizeof (this->vertices[shape.name][0]) * this->vertices[shape.name].size();
            indicesBufferSize += sizeof (this->indices[shape.name][0]) * this->indices[shape.name].size();
            maxStagingBufferSize = std::max(std::max(maxStagingBufferSize, sizeof (this->vertices[shape.name][0]) * this->vertices[shape.name].size()),
                    sizeof (this->indices[shape.name][0]) * this->indices[shape.name].size());
        }

        vk::MemoryAllocateInfo vertexAllocationInfo(0, 0);
        vk::MemoryAllocateInfo indexAllocationInfo(0, 0);
        for (auto& shape : shapes) {
            this->vertexBuffer[shape.name] = vk::Buffer();
            this->indexBuffer[shape.name] = vk::Buffer();

            vk::DeviceSize bufferSize = sizeof (this->vertices[shape.name][0]) * this->vertices[shape.name].size();
            device->createVertexBuffer(bufferSize, this->vertexBuffer[shape.name]);
            bufferSize = sizeof (this->indices[shape.name][0]) * this->indices[shape.name].size();
            device->createIndexBuffer(bufferSize, this->indexBuffer[shape.name]);

            vk::MemoryAllocateInfo ma = device->getMemoryAllocateInfo(this->vertexBuffer[shape.name], vk::MemoryPropertyFlagBits::eDeviceLocal);
            vertexAllocationInfo.allocationSize += ma.allocationSize;
            vertexAllocationInfo.memoryTypeIndex = ma.memoryTypeIndex;
            ma = device->getMemoryAllocateInfo(this->indexBuffer[shape.name], vk::MemoryPropertyFlagBits::eDeviceLocal);
            indexAllocationInfo.allocationSize += ma.allocationSize;
            indexAllocationInfo.memoryTypeIndex = ma.memoryTypeIndex;
        }

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device->createStagingBuffer(maxStagingBufferSize, stagingBuffer, stagingBufferMemory);

        device->createDeviceMemory(vertexAllocationInfo, this->vertexBufferMemory);
        device->createDeviceMemory(indexAllocationInfo, this->indexBufferMemory);

        vk::DeviceSize vertexOffset = 0;
        vk::DeviceSize indicesOffset = 0;
        for (auto& shape : shapes) {
            vk::DeviceSize bufferSize = sizeof (this->vertices[shape.name][0]) * this->vertices[shape.name].size();
            vk::MemoryAllocateInfo ma = device->getMemoryAllocateInfo(this->vertexBuffer[shape.name], vk::MemoryPropertyFlagBits::eDeviceLocal);
            device->copyMemory(bufferSize, vertices[shape.name].data(), stagingBufferMemory);
            device->bindBuffer(vertexBuffer[shape.name], this->vertexBufferMemory, vertexOffset);
            device->copyBuffer(stagingBuffer, vertexBuffer[shape.name], bufferSize);
            vertexOffset += ma.allocationSize;

            bufferSize = sizeof (this->indices[shape.name][0]) * this->indices[shape.name].size();
            ma = device->getMemoryAllocateInfo(this->indexBuffer[shape.name], vk::MemoryPropertyFlagBits::eDeviceLocal);
            device->copyMemory(bufferSize, indices[shape.name].data(), stagingBufferMemory);
            device->bindBuffer(indexBuffer[shape.name], this->indexBufferMemory, indicesOffset);
            device->copyBuffer(stagingBuffer, indexBuffer[shape.name], bufferSize);
            indicesOffset += ma.allocationSize;
        }

        device->freeMemory(stagingBuffer, stagingBufferMemory);
        this->device = device;
    }

    Model::~Model() {
        std::vector<std::string> names = this->getPartNames();
        std::vector<vk::Buffer> iB= {};
        std::vector<vk::Buffer> vB= {};
        for (auto& name : names) {
            iB.push_back(this->indexBuffer[name]);
            vB.push_back(this->vertexBuffer[name]);
        }
        this->device->freeMemory(iB, this->indexBufferMemory);
        this->device->freeMemory(vB, this->vertexBufferMemory);
    }
}

