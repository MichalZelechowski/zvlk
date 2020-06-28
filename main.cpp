#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "Texture.h"
#include "Model.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "TransformationMatrices.h"
#include "Engine.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication : public zvlk::WindowCallback, zvlk::DeviceAssessment, zvlk::EngineCallback {
public:

    void run() {
        init();
        mainLoop();
        cleanup();
    }

private:
    zvlk::Window *window;
    zvlk::Vulkan *vulkan;
    zvlk::Device *device;
    zvlk::Frame *frame;
    zvlk::Texture *texture;
    zvlk::Model* model;
    zvlk::VertexShader *vertexShader;
    zvlk::FragmentShader *fragmentShader;
    zvlk::TransformationMatrices *transformationMatrices;
    zvlk::Engine* engine;


    bool framebufferResized = false;

    void init() {
        this->window = new zvlk::Window(800, 600, std::string("Vulkan"), dynamic_cast<WindowCallback*> (this));

        this->vulkan = new zvlk::Vulkan(enableValidationLayers, std::string("Triangle app"));
        this->vulkan->addSurface(this->window);
        this->device = this->vulkan->getDevice(this);

        this->frame = this->vulkan->initializeDeviceForGraphics(this->device);

        this->texture = new zvlk::Texture(this->device, "viking_room.png");

        this->model = new zvlk::Model(this->device, "viking_room.obj");

        this->vertexShader = new zvlk::VertexShader(this->device->getGraphicsDevice(), "vert.spv");
        this->fragmentShader = new zvlk::FragmentShader(this->device->getGraphicsDevice(), "frag.spv");

        this->transformationMatrices = new zvlk::TransformationMatrices(this->device, this->frame);

        this->engine = new zvlk::Engine(this->frame, this->device);

        this->engine->enableShaders(*this->vertexShader, *this->fragmentShader);
        this->engine->draw(*this->model, *this->transformationMatrices, *this->texture);
        this->engine->compile();

        this->engine->addCallback(this);
    }

    int assess(zvlk::Device* device) {
        const vk::PhysicalDeviceProperties& deviceProperties = device->getProperties();
        const vk::PhysicalDeviceFeatures& deviceFeatures = device->getFeatures();

        int score = 0;
        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader || !deviceFeatures.samplerAnisotropy) {
            return 0;
        }


        if (!this->vulkan->doesDeviceSupportExtensions(device) || !this->vulkan->doesDeviceSupportGraphics(device)) {
            return 0;
        }

        zvlk::SwapChainSupportDetails swapChainSupport = this->vulkan->querySwapChainSupport(device);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if (!swapChainAdequate) {
            return 0;
        }

        return score;
    }

    void update(uint32_t frameIndex) {
        static_cast<zvlk::UniformBuffer*> (this->transformationMatrices)->update(frameIndex);
    }

    //    void recreateSwapChain() {
    //        this->window->waitResize();
    //
    //        vkDeviceWaitIdle(this->device->getGraphicsDevice());
    //
    //        cleanupSwapChain();
    //
    //        createGraphicsPipeline();
    //        createColorResources();
    //        createDepthResources();
    //        createFramebuffers();
    //        createUniformBuffers();
    //        createDescriptorPool();
    //        createDescriptorSets();
    //        createCommandBuffers();
    //    }

    void mainLoop() {
        auto startTime = std::chrono::high_resolution_clock::now();
        uint32_t frames = 0;
        
        while (!window->isClosed()) {
            glfwPollEvents();
            this->engine->execute();
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            frames = frames+1;
            
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << static_cast<float>(frames) / time << " FPS";
            glfwSetWindowTitle(this->window->getWindow(), ss.str().data());
            
            if (frames == 100) {
                frames =0 ;
                startTime = std::chrono::high_resolution_clock::now();
            }
        }

        device->getGraphicsDevice().waitIdle();
    }

    void cleanup() {
        delete this->engine;

        delete this->vertexShader;
        delete this->fragmentShader;

        delete this->transformationMatrices;
        delete this->model;

        delete this->texture;

        delete this->frame;

        delete this->vulkan;

        delete this->window;
    }

    void resize(int width, int height) {
        this->framebufferResized = true;
    }

};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}