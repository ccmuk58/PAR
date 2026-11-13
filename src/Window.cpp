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

    glfwGetCursorPos(m_window, &m_cursorX, &m_cursorY);

    return true;
}

void Window::PollEvents()
{
    glfwPollEvents();
    UpdateMouseDelta();
}

void Window::WaitEvents()
{
    glfwWaitEvents();
    UpdateMouseDelta();
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_window) != 0;
}

void Window::RequestClose()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

bool Window::IsMinimized() const
{
    return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0;
}

double Window::Time() const
{
    return glfwGetTime();
}   

bool Window::IsKeyDown(Key key) const
{
    int glfwKey = GLFW_KEY_UNKNOWN;
    switch (key)
    {
    case Key::W: glfwKey = GLFW_KEY_W; break;
    case Key::A: glfwKey = GLFW_KEY_A; break;
    case Key::S: glfwKey = GLFW_KEY_S; break;
    case Key::D: glfwKey = GLFW_KEY_D; break;
    case Key::Space: glfwKey = GLFW_KEY_SPACE; break;
    case Key::LeftShift: glfwKey = GLFW_KEY_LEFT_SHIFT; break;
    case Key::Escape: glfwKey = GLFW_KEY_ESCAPE; break;
    }
    if (glfwKey == GLFW_KEY_UNKNOWN)
        return false;

    return glfwGetKey(m_window, glfwKey) == GLFW_PRESS;
}

bool Window::IsMouseButtonDown(MouseButton button) const
{
    const int glfwButton = (button == MouseButton::Left) ? GLFW_MOUSE_BUTTON_LEFT : GLFW_MOUSE_BUTTON_RIGHT;
    return glfwGetMouseButton(m_window, glfwButton) == GLFW_PRESS;
}

void Window::SetCursorCaptured(bool captured)
{
    if (captured == m_cursorCaptured)
        return;
    m_cursorCaptured = captured;

    glfwSetInputMode(m_window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, captured ? GLFW_TRUE : GLFW_FALSE);

    glfwGetCursorPos(m_window, &m_cursorX, &m_cursorY);
    m_mouseDelta = {};
}

void Window::UpdateMouseDelta()
{
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(m_window, &x, &y);
    m_mouseDelta = { static_cast<float>(x - m_cursorX), static_cast<float>(y - m_cursorY) };
    m_cursorX = x;
    m_cursorY = y;
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
