
/**
 * @file viewer.cpp
 * @brief Robot Status Viewer (ImGui)
 */

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <memory>
#include <algorithm/algorithm_teleop.hpp>

// GLFW Error Callback
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Locomo Robot Viewer", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // State
    std::unique_ptr<locomo::algorithm::AlgorithmBase> teleop = std::make_unique<locomo::algorithm::AlgorithmTeleop>();
    float current_vx = 0.0f;
    float current_wz = 0.0f;
    float battery_voltage = 12.0f; // Dummy value

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Window: Robot Status
        {
            ImGui::Begin("Robot Status");
            ImGui::Text("Connection: Connected");
            ImGui::Separator();

            ImGui::Text("Battery Voltage: %.2f V", battery_voltage);

            ImGui::Separator();
            ImGui::Text("Command History");
            ImGui::Text("vx: %.2f m/s", current_vx);
            ImGui::Text("wz: %.2f rad/s", current_wz);

            // Handle Key Input for Teleop (Simple example)
            if (ImGui::IsKeyPressed(ImGuiKey_W)) { current_vx = 0.5f; }
            if (ImGui::IsKeyPressed(ImGuiKey_S)) { current_vx = -0.5f; }
            if (ImGui::IsKeyPressed(ImGuiKey_X)) { current_vx = 0.0f; current_wz = 0.0f; }
             // Reset for simple demonstration
             // logic to keep state would go here

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
