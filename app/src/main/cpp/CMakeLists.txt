cmake_minimum_required(VERSION 3.22)
project(fluentdemo CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# repo root = app/src/main/cpp/../../../..
get_filename_component(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
set(IMGUI "${ROOT}/third_party/imgui")          # fetched by CI (see ci/build.yml)
set(GLUE "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

add_library(native_app_glue STATIC "${GLUE}/android_native_app_glue.c")

add_library(fluentdemo SHARED
    main.cpp
    ${IMGUI}/imgui.cpp
    ${IMGUI}/imgui_draw.cpp
    ${IMGUI}/imgui_tables.cpp
    ${IMGUI}/imgui_widgets.cpp
    ${IMGUI}/backends/imgui_impl_android.cpp
    ${IMGUI}/backends/imgui_impl_opengl3.cpp)

target_include_directories(fluentdemo PRIVATE
    ${IMGUI} ${IMGUI}/backends ${GLUE}
    ${ROOT}/fluent ${ROOT}/demo)
target_compile_definitions(fluentdemo PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
set_target_properties(fluentdemo PROPERTIES LINK_FLAGS "-u ANativeActivity_onCreate")
target_link_libraries(fluentdemo native_app_glue android EGL GLESv3 log)
