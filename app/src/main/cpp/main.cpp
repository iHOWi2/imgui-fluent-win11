// Android (NativeActivity + EGL/GLES3) host for the Fluent demo. Landscape, touch input.
#include "fluent.h"
#include "demo_menu.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android_native_app_glue.h>

static EGLDisplay g_Display = EGL_NO_DISPLAY;
static EGLSurface g_Surface = EGL_NO_SURFACE;
static EGLContext g_Context = EGL_NO_CONTEXT;
static struct android_app* g_App = nullptr;
static bool g_Init = false;
static Demo::State g_State;
static bool g_Open = true;

static int GetUnicodeChar(int action, int keyCode, int metaState) {
    JavaVM* vm = g_App->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return 0;
    jclass cls = env->FindClass("android/view/KeyEvent");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(II)V");
    jobject ev = env->NewObject(cls, ctor, action, keyCode);
    jmethodID get = env->GetMethodID(cls, "getUnicodeChar", "(I)I");
    int ch = env->CallIntMethod(ev, get, metaState);
    env->DeleteLocalRef(ev);
    env->DeleteLocalRef(cls);
    return ch;
}

static void ShowSoftKeyboard() {
    JavaVM* vm = g_App->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = g_App->activity->clazz;
    jclass actCls = env->GetObjectClass(activity);
    jmethodID getSvc = env->GetMethodID(actCls, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring name = env->NewStringUTF("input_method");
    jobject imm = env->CallObjectMethod(activity, getSvc, name);
    jclass immCls = env->GetObjectClass(imm);
    jmethodID show = env->GetMethodID(immCls, "showSoftInput", "(Landroid/view/View;I)Z");
    jmethodID getWin = env->GetMethodID(actCls, "getWindow", "()Landroid/view/Window;");
    jobject win = env->CallObjectMethod(activity, getWin);
    jclass winCls = env->GetObjectClass(win);
    jmethodID getDecor = env->GetMethodID(winCls, "getDecorView", "()Landroid/view/View;");
    jobject decor = env->CallObjectMethod(win, getDecor);
    env->CallBooleanMethod(imm, show, decor, 0);
}

static void InitAll(struct android_app* app) {
    if (g_Init) return;
    g_App = app;
    ANativeWindow_acquire(app->window);

    g_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_Display, nullptr, nullptr);
    const EGLint cfgAttr[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                               EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
    EGLConfig cfg; EGLint n = 0;
    eglChooseConfig(g_Display, cfgAttr, &cfg, 1, &n);
    EGLint fmt; eglGetConfigAttrib(g_Display, cfg, EGL_NATIVE_VISUAL_ID, &fmt);
    ANativeWindow_setBuffersGeometry(app->window, 0, 0, fmt);
    const EGLint ctxAttr[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_Context = eglCreateContext(g_Display, cfg, EGL_NO_CONTEXT, ctxAttr);
    g_Surface = eglCreateWindowSurface(g_Display, cfg, app->window, nullptr);
    eglMakeCurrent(g_Display, g_Surface, g_Surface, g_Context);
    eglSwapInterval(g_Display, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    int density = app->config ? AConfiguration_getDensity(app->config) : 320;
    if (density <= 0 || density > 1000) density = 320;
    Fluent::Init(density / 160.f);
    g_Init = true;
}

static void ShutdownAll() {
    if (!g_Init) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    if (g_Display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_Context != EGL_NO_CONTEXT) eglDestroyContext(g_Display, g_Context);
        if (g_Surface != EGL_NO_SURFACE) eglDestroySurface(g_Display, g_Surface);
        eglTerminate(g_Display);
    }
    g_Display = EGL_NO_DISPLAY; g_Context = EGL_NO_CONTEXT; g_Surface = EGL_NO_SURFACE;
    ANativeWindow_release(g_App->window);
    g_Init = false;
}

static void OnAppCmd(struct android_app* app, int32_t cmd) {
    if (cmd == APP_CMD_INIT_WINDOW) { if (app->window) InitAll(app); }
    else if (cmd == APP_CMD_TERM_WINDOW) ShutdownAll();
}

static int32_t OnInput(struct android_app*, AInputEvent* e) {
    if (g_Init && AInputEvent_getType(e) == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getAction(e) == AKEY_EVENT_ACTION_DOWN) {
        int ch = GetUnicodeChar(AKEY_EVENT_ACTION_DOWN, AKeyEvent_getKeyCode(e), AKeyEvent_getMetaState(e));
        if (ch > 0x1F && ch != 0x7F) ImGui::GetIO().AddInputCharacter((unsigned)ch);
    }
    return ImGui_ImplAndroid_HandleInputEvent(e);
}

static void Frame() {
    if (!g_Init) return;
    ImGuiIO& io = ImGui::GetIO();
    static bool wasText = false;
    if (io.WantTextInput && !wasText) ShowSoftKeyboard();
    wasText = io.WantTextInput;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    Fluent::NewFrame();
    Demo::Render(g_State, &g_Open);
    if (!g_Open) g_Open = true;          // a phone has no desktop to close to
    ImGui::Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    const ImVec4 bg = Fluent::Ctx().themeT > 0.5f ? ImVec4(0.80f, 0.86f, 0.95f, 1.f) : ImVec4(0.06f, 0.07f, 0.10f, 1.f);
    glClearColor(bg.x, bg.y, bg.z, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(g_Display, g_Surface);
}

void android_main(struct android_app* app) {
    app->onAppCmd = OnAppCmd;
    app->onInputEvent = OnInput;
    while (true) {
        int events; struct android_poll_source* src;
        while (ALooper_pollOnce(g_Init ? 0 : -1, nullptr, &events, (void**)&src) >= 0) {
            if (src) src->process(app, src);
            if (app->destroyRequested != 0) { ShutdownAll(); return; }
        }
        Frame();
    }
}
