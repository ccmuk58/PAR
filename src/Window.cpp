#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <cstdio>
#include <utility>

Window::~Window()
{
    if (m_window)
    {
        glfwSetWindowUserPointer(m_window, nullptr);
        glfwDestroyWindow(m_window);
    }

    glfwTerminate();
}

bool Window::Init(int width, int height, const char* title)
{
    glfwSetErrorCallback([](int code, const char* desc) {
        std::fprintf(stderr, "[GLFW %d] %s\n", code, desc);
    });

    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
        return false;

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int w, int h) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_onResize)
            self->m_onResize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    });

    return true;
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::WaitEvents()
{
    glfwWaitEvents();
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_window) != 0;
}

bool Window::IsMinimized() const
{
    return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0;
}

double Window::Time() const
{
    return glfwGetTime();
}

HWND Window::Hwnd() const
{
    return glfwGetWin32Window(m_window);
}

uint32_t Window::Width() const
{
    int width = 0;
    glfwGetFramebufferSize(m_window, &width, nullptr);
    return static_cast<uint32_t>(width);
}

uint32_t Window::Height() const
{
    int height = 0;
    glfwGetFramebufferSize(m_window, nullptr, &height);
    return static_cast<uint32_t>(height);
}

void Window::SetResizeCallback(std::function<void(uint32_t, uint32_t)> callback)
{
    m_onResize = std::move(callback);
}
