# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("nativeguitest")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(${PROJECT_NAME} SHARED
        main.cpp
        AndroidOut.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        Utility.cpp)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Configure libraries CMake uses to link your target library.
target_link_libraries(${PROJECT_NAME}
        # The game activity
        game-activity::game-activity_static

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log)