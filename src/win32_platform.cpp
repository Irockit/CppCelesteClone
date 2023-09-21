#include "game_lib.h"

#include "platform.h"
#include <glcorearb.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"

static HWND window;

LRESULT CALLBACK windows_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;

    switch (msg){
    case WM_CLOSE:{
        running = false;
        break;
    }
    
    default:{
        result = DefWindowProcA(window, msg, wParam, lParam);
    }
    }
    return result;
}

bool platform_create_window(int width, int height, char* title){
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = title;
    wc.lpfnWndProc = windows_window_callback;

    if(!RegisterClassA(&wc)) return false;

    int dwStyle = WS_OVERLAPPEDWINDOW;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    {
        window = CreateWindowExA(0, title, title, dwStyle, 100, 100, width, height, NULL, NULL, instance, NULL);
        SM_ASSERT_GUARD(window != NULL, false, "Failed to create Windows Window");

        HDC fakeDC = GetDC(window);
        SM_ASSERT_GUARD(fakeDC, false, "Failed to get HDC");

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
        SM_ASSERT_GUARD(pixelFormat, false, "Failed to choose pixel Format");
        SM_ASSERT_GUARD(SetPixelFormat(fakeDC, pixelFormat, &pfd), false, "Failed to set pixel format");

        HGLRC fakeRC = wglCreateContext(fakeDC);
        SM_ASSERT_GUARD(wglMakeCurrent(fakeDC, fakeRC), false, "Failed to make current");
        
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");
        SM_ASSERT_GUARD(wglCreateContextAttribsARB && wglChoosePixelFormatARB, false, "Failed to load Opengl functions");

        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window, fakeDC);
        DestroyWindow(window);
    }

    {
        {
            RECT borderRect = {};
            AdjustWindowRectEx(&borderRect, dwStyle, 0, 0);
            width += borderRect.right - borderRect.left;
            height += borderRect.bottom - borderRect.top;
        }
        window = CreateWindowExA(0, title, title, dwStyle, 100, 100, width, height, NULL, NULL, instance, NULL);
        SM_ASSERT_GUARD(window != NULL, false, "Failed to create Windows Window");
        HDC dc = GetDC(window);
        SM_ASSERT_GUARD(dc, false, "Failed to get HDC");
        const int pixelAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            0
        };
        UINT numPixelFormats;
        int pixelFormat = 0;
        SM_ASSERT_GUARD(wglChoosePixelFormatARB(dc, pixelAttribs, 0, 1, &pixelFormat, &numPixelFormats), false, "Failed to wglChoosePixelFormatARB");

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        SM_ASSERT_GUARD(SetPixelFormat(dc, pixelFormat, &pfd), false, "Failed to SetPixelFormat");

        const int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };
        HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
        SM_ASSERT_GUARD(rc, false, "Failed to create Render Context for OpenGL");
        SM_ASSERT_GUARD(wglMakeCurrent(dc,rc), false, "Failed to wglMakeCurent");

    }

    ShowWindow(window, SW_SHOW);
    return true;
}
void platform_update_window(){
    MSG msg;

    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void* platform_load_gl_function(char* funName){
    PROC proc = wglGetProcAddress(funName);
    if(!proc){
        static HMODULE openglDLL = LoadLibraryA("opengl32.dll");
        proc = GetProcAddress(openglDLL, funName);
        if(!proc){
            SM_ASSERT(false, "Failed to load gl function %s", "glCreateProgram");
            return nullptr;
        }
    }
    return (void*)proc;
}