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

    Model::Model(zvlk::Device* device, const std::string name, std::shared_ptr<zvlk::Frame> frame) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, name.c_str(), "/tmp/")) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices({});

        for (tinyobj::material_t& mat : materials) {
            zvlk::Material* newMaterial = new zvlk::Material(device, frame, mat.name,
                    glm::vec4(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.0f),
                    glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f),
                    glm::vec4(mat.specular[0], mat.specular[1], mat.specular[2], 1.0f),
                    mat.shininess, mat.diffuse_texname);
            this->materials.push_back(newMaterial);
            this->modelParts[newMaterial] = {};
        }

        uint32_t lastIndexOffset = 0;
        int indiceIndex = 0;
        int lastMaterial = -1;
        //TODO sorting by materials
        for (const auto& shape : shapes) {
            const tinyobj::mesh_t& mesh = shape.mesh;

            int meshIndiceIndex=0;
            for (const auto& index : mesh.indices) {
                Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
                
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t> (this->vertices.size());
                    this->vertices.push_back(vertex);
                }

                if (lastMaterial == -1) {
                    lastMaterial = mesh.material_ids[meshIndiceIndex/3];
                } else if (lastMaterial != mesh.material_ids[meshIndiceIndex/3]) {
                    this->modelParts[this->materials[lastMaterial]].push_back({indiceIndex - lastIndexOffset, lastIndexOffset});

                    lastMaterial = mesh.material_ids[meshIndiceIndex/3];
                    lastIndexOffset = indiceIndex;
                }

                this->indices.push_back(uniqueVertices[vertex]);
                indiceIndex++;
                meshIndiceIndex++;
            }

        }
        this->modelParts[this->materials[lastMaterial]].push_back({indiceIndex - lastIndexOffset, lastIndexOffset});

        vk::DeviceSize vertexBufferSize = sizeof (this->vertices[0]) * this->vertices.size();
        vk::DeviceSize indicesBufferSize = sizeof (this->indices[0]) * this->indices.size();
        vk::DeviceSize maxStagingBufferSize = std::max(vertexBufferSize, indicesBufferSize);

        device->createVertexBuffer(vertexBufferSize, this->vertexBuffer);
        device->createIndexBuffer(indicesBufferSize, this->indexBuffer);

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device->createStagingBuffer(maxStagingBufferSize, stagingBuffer, stagingBufferMemory);

        vk::MemoryAllocateInfo vertexAllocationInfo = device->getMemoryAllocateInfo(this->vertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::MemoryAllocateInfo indexAllocationInfo = device->getMemoryAllocateInfo(this->indexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        device->createDeviceMemory(vertexAllocationInfo, this->vertexBufferMemory);
        device->createDeviceMemory(indexAllocationInfo, this->indexBufferMemory);

        device->copyMemory(vertexBufferSize, vertices.data(), stagingBufferMemory);
        device->bindBuffer(vertexBuffer, this->vertexBufferMemory, static_cast<vk::DeviceSize> (0));
        device->copyBuffer(stagingBuffer, vertexBuffer, vertexBufferSize);

        device->copyMemory(indicesBufferSize, indices.data(), stagingBufferMemory);
        device->bindBuffer(indexBuffer, this->indexBufferMemory, static_cast<vk::DeviceSize> (0));
        device->copyBuffer(stagingBuffer, indexBuffer, indicesBufferSize);

        device->freeMemory(stagingBuffer, stagingBufferMemory);
        this->device = device;
    }

    Model::~Model() {
        for (auto material : materials) {
            delete material;
        }
        this->device->freeMemory(this->indexBuffer, this->indexBufferMemory);
        this->device->freeMemory(this->vertexBuffer, this->vertexBufferMemory);
    }
}

