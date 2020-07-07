#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <zip.h>
#include <fstream>

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "Texture.h"
#include "Model.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "TransformationMatrices.h"
#include "Engine.h"
#include "Camera.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const char* MODEL_PATH = "/tmp/ldm12.obj";

class TrainApplication : public zvlk::WindowCallback, zvlk::DeviceAssessment, zvlk::EngineCallback {
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
    zvlk::Camera* camera;
    int lastKey = 0;


    bool framebufferResized = false;

    void init() {
        this->window = new zvlk::Window(800, 600, std::string("Vulkan"), dynamic_cast<WindowCallback*> (this));

        this->vulkan = new zvlk::Vulkan(enableValidationLayers, std::string("Triangle app"));
        this->vulkan->addSurface(this->window);
        this->device = this->vulkan->getDevice(this);

        this->frame = this->vulkan->initializeDeviceForGraphics(this->device);
        this->frame->attachWindow(this->window);

        this->texture = new zvlk::Texture(this->device, "viking_room.png");

        this->model = new zvlk::Model(this->device, MODEL_PATH);

        this->vertexShader = new zvlk::VertexShader(this->device->getGraphicsDevice(), "vert.spv");
        this->fragmentShader = new zvlk::FragmentShader(this->device->getGraphicsDevice(), "frag.spv");

        this->camera = new zvlk::Camera(device, frame, glm::vec3(500.0f, 500.0f, 500.0f),
                glm::vec3(0.0f, 0.0f, 0.0f), 45.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 2500.0f);
        this->transformationMatrices = new zvlk::TransformationMatrices(this->device, this->frame);
        this->transformationMatrices->rotate(90, glm::vec3(1.0f, 0.0f, 0.0f))
                .translate(glm::vec3(0.0f, -250.0f, 0.0f));
        this->engine = new zvlk::Engine(this->frame, this->device);
        this->engine->setCamera(this->camera);
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
        if (this->lastKey == GLFW_KEY_A) {
            this->camera->rotateEye(1.0f);
        } else if (this->lastKey == GLFW_KEY_D) {
            this->camera->rotateEye(-1.0f);
        } else if (this->lastKey == GLFW_KEY_W) {
            this->transformationMatrices->translate(glm::vec3(0.0f, -1.0f, 0.0f));
        } else if (this->lastKey == GLFW_KEY_S) {
            this->transformationMatrices->translate(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        static_cast<zvlk::UniformBuffer*> (this->transformationMatrices)->update(frameIndex);
        static_cast<zvlk::UniformBuffer*> (this->camera)->update(frameIndex);
    }

    void recreateSwapChain() {
        this->window->waitResize();

        this->device->getGraphicsDevice().waitIdle();

        this->frame->destroy();
        this->engine->clean();

        this->frame->create(this->device, this->vulkan->getSurface());
        this->engine->compile();

        this->framebufferResized = false;
    }

    void mainLoop() {
        auto startTime = std::chrono::high_resolution_clock::now();
        uint32_t frames = 0;

        while (!window->isClosed() && lastKey != GLFW_KEY_Q) {
            glfwPollEvents();
            vk::Bool32 needSwapChainRecreate = !this->engine->execute(this->framebufferResized);

            if (needSwapChainRecreate) {
                this->recreateSwapChain();
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            frames = frames + 1;

            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << static_cast<float> (frames) / time << " FPS";
            glfwSetWindowTitle(this->window->getWindow(), ss.str().data());

            if (frames == 100) {
                frames = 0;
                startTime = std::chrono::high_resolution_clock::now();
            }
        }

        device->getGraphicsDevice().waitIdle();
    }

    void cleanup() {
        delete this->engine;

        delete this->vertexShader;
        delete this->fragmentShader;

        delete this->camera;
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

    void key(int key, int action, int mods) {
        this->lastKey = key;

        if (key == GLFW_KEY_F && action == GLFW_PRESS) {
            window->toggleFullscreen();
        } else if ((key == GLFW_KEY_A || key == GLFW_KEY_D || key == GLFW_KEY_W || key == GLFW_KEY_S)
                && action == GLFW_RELEASE) {
            this->lastKey = 0;
        }

    }

};

int inflateModel(std::string source, std::string dest) {
    int errorp;
    zip_t* zipSource = zip_open((source + ".zip").data(), ZIP_RDONLY, &errorp);
    if (zipSource == NULL) {
        return errorp;
    }

    for (auto ext :{".obj", ".mtl"}) {
        zip_file_t* sourceFile = zip_fopen(zipSource, (source + ext).data(), ZIP_FL_UNCHANGED);

        zip_stat_t stat;
        if (zip_stat(zipSource, (source + ext).data(), ZIP_FL_UNCHANGED, &stat) == -1) {
            return -1;
        }

        char* buffer = new char[stat.size];
        if (zip_fread(sourceFile, buffer, stat.size) == -1) {
            return -1;
        }

        std::ofstream output((dest + source + ext).data(), std::ios::out | std::ios::binary);
        output.write(buffer, stat.size);
        output.close();

        delete[] buffer;
    }
    return 0;
}

int main() {
    inflateModel("ldm12", "/tmp/");

    TrainApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}