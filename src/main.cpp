#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Renderer.h"
#include <cstdio>

// 윈도우 크기 변경 시 호출되는 콜백
static void OnFramebufferResize(GLFWwindow* window, int width, int height)
{
    auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer)
        renderer->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

int main()
{
    glfwSetErrorCallback([](int code, const char* desc) {
        std::fprintf(stderr, "[GLFW %d] %s\n", code, desc);
    });

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "PAR", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    {   
        Renderer renderer;

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        if (renderer.Init(glfwGetWin32Window(window),
            static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight)))
        {
            glfwSetWindowUserPointer(window, &renderer);
            glfwSetFramebufferSizeCallback(window, OnFramebufferResize);

            // ---------------- 메인 루프 ----------------
            while (!glfwWindowShouldClose(window))
            {
                // OS 이벤트 처리
                glfwPollEvents();

                if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                {
                    glfwWaitEvents();
                    continue;
                }

                // renderer.Render();
                renderer.Render(static_cast<float>(glfwGetTime()));
                renderer.Present(true);
            }

            glfwSetWindowUserPointer(window, nullptr);
        } 
    } // Renderer 소멸

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
