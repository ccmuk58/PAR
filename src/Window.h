#pragma once
#include <windows.h>
#include <cstdint>
#include <functional>

struct GLFWwindow;

// GLFW 키 코드를 헤더 밖으로 노출하지 않기 위한 키 목록
enum class Key
{
    W, A, S, D, Escape,
};

enum class MouseButton
{
    Left, Right,
};

struct MouseDelta
{
    float x = 0.0f;
    float y = 0.0f;
};

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Init(int width, int height, const char* title);

    // ---- 메인 루프용 ----
    void PollEvents();
    void WaitEvents();
    bool ShouldClose() const;
    bool IsMinimized() const;
    double Time() const;

    // ---- 입력 ----
    bool IsKeyDown(Key key) const;
    bool IsMouseButtonDown(MouseButton button) const;
    // true일 때 커서를 숨기고 창에 고정
    void SetCursorCaptured(bool captured);
    bool IsCursorCaptured() const { return m_cursorCaptured; }
    // 직전 PollEvents 동안 마우스가 움직인 양 (픽셀, 아래쪽이 +y)
    MouseDelta MouseMovement() const { return m_mouseDelta; }

    // ---- Device 생성에 필요한 정보 ----
    HWND     Hwnd() const;
    uint32_t Width() const;
    uint32_t Height() const;

    void SetResizeCallback(std::function<void(uint32_t, uint32_t)> callback);

private:
    void UpdateMouseDelta();

    GLFWwindow* m_window = nullptr;
    std::function<void(uint32_t, uint32_t)> m_onResize;

    double     m_cursorX = 0.0;
    double     m_cursorY = 0.0;
    MouseDelta m_mouseDelta;
    bool       m_cursorCaptured = false;
};
