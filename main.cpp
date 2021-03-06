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

class BallApplication : public zvlk::WindowCallback, zvlk::DeviceAssessment, zvlk::EngineCallback {
public:

    void run() {
        init();
        mainLoop();
        cleanup();
    }

private:
    std::shared_ptr<zvlk::Window> window;
    std::unique_ptr<zvlk::Vulkan> vulkan;
    std::shared_ptr<zvlk::Frame> frame;
    zvlk::Device *device;
    zvlk::Model* room;
    zvlk::Model* ball;
    zvlk::VertexShader *vertexShader;
    zvlk::FragmentShader *fragmentShader;
    zvlk::TransformationMatrices *transformationMatrices;
    zvlk::TransformationMatrices *ballTransformationMatrices;
    zvlk::Engine* engine;
    zvlk::Camera* camera;
    std::set<int> lastKeys;

    bool framebufferResized = false;
    float ballFallingSpeed = 0.0f;
    float ballDistance = 0.0f;

    void init() {
        this->window = std::shared_ptr<zvlk::Window>(new zvlk::Window(800, 600, std::string("Vulkan"), dynamic_cast<WindowCallback*> (this)));

        this->vulkan = std::unique_ptr<zvlk::Vulkan>(new zvlk::Vulkan(enableValidationLayers, std::string("Triangle app")));
        this->vulkan->addSurface(this->window);
        this->device = this->vulkan->getDevice(this);

        this->frame = this->vulkan->initializeDeviceForGraphics(this->device);
        this->frame->attachWindow(this->window);

        this->room = new zvlk::Model(this->device, "/tmp/room.obj", this->frame);
        this->ball = new zvlk::Model(this->device, "/tmp/ball.obj", this->frame);

        this->vertexShader = new zvlk::VertexShader(this->device->getGraphicsDevice(), "vert.spv");
        this->fragmentShader = new zvlk::FragmentShader(this->device->getGraphicsDevice(), "frag.spv");

        this->camera = new zvlk::Camera(device, frame, glm::vec3(10.0f, 10.0f, 10.0f),
                glm::vec3(0.0f, 0.0f, 0.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.1f, 2500.0f);
        this->transformationMatrices = new zvlk::TransformationMatrices(this->device, this->frame);
        this->transformationMatrices->scale(glm::vec3(10, 10, 10));
        this->ballTransformationMatrices = new zvlk::TransformationMatrices(this->device, this->frame);

        this->engine = new zvlk::Engine(this->frame, this->device);
        this->engine->setCamera(this->camera);
        this->engine->attachLight(new zvlk::Light({10.0f, 10.0f, 10.0f},
        {
            1.0f, 1.0f, 1.0f, 1.0f
        }, 0.0f));
        this->engine->enableShaders(*this->vertexShader, *this->fragmentShader);
        this->engine->draw(*this->room, *this->transformationMatrices);
        this->engine->draw(*this->ball, *this->ballTransformationMatrices);
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
        if (this->lastKeys.count(GLFW_KEY_A)) {
            this->camera->rotateEye(-1.0f);
        } else if (this->lastKeys.count(GLFW_KEY_D)) {
            this->camera->rotateEye(1.0f);
        } else if (this->lastKeys.count(GLFW_KEY_W)) {
        } else if (this->lastKeys.count(GLFW_KEY_S)) {
        }

        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        if (ballDistance <= 9.0f) {
            ballFallingSpeed += 9.81f * time;
            float ballDistanceDelta = 100 * 0.5f * ballFallingSpeed * time;
            ballDistance += ballDistanceDelta;
            if (ballDistance > 9.0f) {
                ballDistanceDelta -= ballDistance - 9.0f;
                ballDistance = 9.0f;
                ballFallingSpeed *= -1*0.95f;
            }
            this->ballTransformationMatrices->translate(glm::vec3(0.0f, -ballDistanceDelta, 0.0f));
        }


        startTime = currentTime;

        static_cast<zvlk::UniformBuffer*> (this->transformationMatrices)->update(frameIndex);
        static_cast<zvlk::UniformBuffer*> (this->ballTransformationMatrices)->update(frameIndex);
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

        while (!window->isClosed() && lastKeys.count(GLFW_KEY_Q) == 0) {
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
        delete this->ballTransformationMatrices;
        delete this->room;
        delete this->ball;
    }

    void resize(int width, int height) {
        this->framebufferResized = true;
    }

    void key(int key, int action, int mods) {
        this->lastKeys.insert(key);

        if (key == GLFW_KEY_F && action == GLFW_PRESS) {
            window->toggleFullscreen();
        } else if ((key == GLFW_KEY_A || key == GLFW_KEY_D || key == GLFW_KEY_W || key == GLFW_KEY_S)
                && action == GLFW_RELEASE) {
            this->lastKeys.erase(key);
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
    inflateModel("ball", "/tmp/");
    inflateModel("room", "/tmp/");

    BallApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}