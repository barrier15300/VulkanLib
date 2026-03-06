#pragma once

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")

#pragma warning(disable:26812)
#pragma warning(disable:26451)
#pragma warning(disable:26495)
#include <fbxsdk.h>
#pragma warning(default:26812)
#pragma warning(default:26451)
#pragma warning(default:26495)

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "Timer.h"
