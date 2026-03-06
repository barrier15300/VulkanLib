#include "pch.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class HelloTriangleApplication {
public:

	void run() {
		initWindow();
		initVulkan();
		if (!initflag) {
			return;
		}
		mainLoop();
		cleanup();
	}

private:

#ifdef NDEBUG
	static constexpr bool enableValidationLayers = false;
#else
	static constexpr bool enableValidationLayers = true;
#endif

	template<class T>
	using to_array = vk::ArrayProxyNoTemporaries<T>;

	static constexpr uint32_t WIDTH = 800;
	static constexpr uint32_t HEIGHT = 600;
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	bool initflag = true;
	GLFWwindow* window = nullptr;
	vk::SurfaceKHR surface;

	vk::detail::DispatchLoaderDynamic dispatcher;
	vk::Instance instance;
	vk::DebugUtilsMessengerEXT debugUtilsMessenger;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::ImageView> swapChainImageViews;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;
	vk::RenderPass renderPass;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;
	std::vector<vk::Framebuffer> swapChainFramebuffers;
	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> commandBuffer;
	std::vector<vk::Semaphore> imageAvailableSemaphore;
	std::vector<vk::Semaphore> renderFinishedSemaphore;
	std::vector<vk::Fence> inFlightFence;
	uint32_t currentFrame = 0;

	Timer timer;
	glm::vec3 pos;

	bool framebufferResized = false;
	static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		
		static vk::VertexInputBindingDescription getBindingDescription() {
			auto bindingDescription = vk::VertexInputBindingDescription(
				0,
				sizeof(Vertex),
				vk::VertexInputRate::eVertex
			);
			return bindingDescription;
		}
		static auto getAttributeDescriptions() {
			auto attributeDescriptions = std::array{
				vk::VertexInputAttributeDescription(
					0,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(Vertex, pos)
				),
				vk::VertexInputAttributeDescription(
					1,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(Vertex, color)
				),
				vk::VertexInputAttributeDescription(
					2,
					0,
					vk::Format::eR32G32Sfloat,
					offsetof(Vertex, texCoord)
				)
			};

			return attributeDescriptions;
		}
	};
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};
	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;
	vk::Buffer indexBuffer;
	vk::DeviceMemory indexBufferMemory;

	std::vector<vk::Buffer> uniformBuffers;
	std::vector<vk::DeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;

	vk::Image textureImage;
	vk::DeviceMemory textureImageMemory;
	vk::ImageView textureImageView;
	vk::Sampler textureSampler;

	vk::Image depthImage;
	vk::DeviceMemory depthImageMemory;
	vk::ImageView depthImageView;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, &frameBufferResizeCallBack);
	}
	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicPipeline();
		createCommandPool();
		createDepthResources();
		createFramebuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}
	void cleanup() {
		cleanupSwapChain();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			device.destroy(imageAvailableSemaphore[i]);
			device.destroy(renderFinishedSemaphore[i]);
			device.destroy(inFlightFence[i]);
			device.destroy(uniformBuffers[i]);
			device.free(uniformBuffersMemory[i]);
		}
		device.destroy(textureSampler);
		device.destroy(textureImageView);
		device.destroy(textureImage);
		device.free(textureImageMemory);
		device.destroy(vertexBuffer);
		device.free(vertexBufferMemory);
		device.destroy(indexBuffer);
		device.free(indexBufferMemory);
		device.destroy(commandPool);
		device.destroy(graphicsPipeline);
		device.destroy(pipelineLayout);
		device.destroy(descriptorPool);
		device.destroy(descriptorSetLayout);
		device.destroy(renderPass);
		device.destroy();
		if (enableValidationLayers) {
			instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger, nullptr, dispatcher);
		}
		instance.destroySurfaceKHR(surface);
		instance.destroy();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createInstance() {

		if (enableValidationLayers && !checkValidationLayerSupport()) {
			initflag = false;
			return;
		}

		auto appInfo = vk::ApplicationInfo("Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
		auto createInfo = vk::InstanceCreateInfo({}, &appInfo);

		vk::DebugUtilsMessengerCreateInfoEXT debugInfo;

		if (enableValidationLayers) {
			createInfo.setPEnabledLayerNames(validationLayers);
			debugInfo = getDebugUtilsMessengerCreateInfo();
			createInfo.setPNext(&debugInfo);
		}

		auto extensions = getRequiredExtensions();
		createInfo.setPEnabledExtensionNames(extensions);

		instance = vk::createInstance(createInfo);
		
		dispatcher.init();
		dispatcher.init(instance);
	}
	void setupDebugMessenger() {
		if (!enableValidationLayers) { return; }

		auto createInfo = getDebugUtilsMessengerCreateInfo();

		debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dispatcher);
	}
	void createSurface() {
		VkSurfaceKHR sur;
		if (glfwCreateWindowSurface(instance, window, nullptr, &sur) != VK_SUCCESS) {
			initflag = false;
			return;
		}
		surface = sur;
	}
	void pickPhysicalDevice() {
		auto devices = instance.enumeratePhysicalDevices();

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (!physicalDevice) {
			initflag = false;
			return;
		}
	}
	void createLogicalDevice() {
		auto indices = findQueueFamilies(physicalDevice);

		float queuepriority = 1.0;
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos; 
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		for (auto queueFamily : uniqueQueueFamilies) {
			auto queueCreateInfo = vk::DeviceQueueCreateInfo({}, queueFamily, to_array<const float>(queuepriority));
			queueCreateInfos.push_back(queueCreateInfo);
		}

		auto deviceFeatures = physicalDevice.getFeatures();

		auto createInfo = vk::DeviceCreateInfo({}, queueCreateInfos, {}, deviceExtensions, &deviceFeatures);

		device = physicalDevice.createDevice(createInfo);

		graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
		presentQueue = device.getQueue(indices.presentFamily.value(), 0);
	}
	void createSwapChain() {
		auto swapChainSupport = querySwapChainSupport(physicalDevice);

		auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		auto presentMode = chooseSwapSurfacePresentMode(swapChainSupport.presentModes);
		auto extent	 = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		const uint32_t maxImageCount = swapChainSupport.capabilities.maxImageCount;

		if (maxImageCount > 0 && imageCount > maxImageCount) {
			imageCount = maxImageCount;
		}
		
		auto indices = findQueueFamilies(physicalDevice);
		auto queueFamilyIndicesReference = vk::ArrayProxyNoTemporaries<const uint32_t>();
		uint32_t queueFamilyIndices[] = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()	
		};

		vk::SharingMode sharingMode = vk::SharingMode::eExclusive;

		if (indices.graphicsFamily != indices.presentFamily) {
			sharingMode = vk::SharingMode::eConcurrent;
			queueFamilyIndicesReference = queueFamilyIndices;
		}

		auto createInfo = vk::SwapchainCreateInfoKHR(
			{},
			surface,
			imageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			sharingMode,
			queueFamilyIndicesReference,
			swapChainSupport.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode,
			true,
			{}
		);

		swapChain = device.createSwapchainKHR(createInfo);
		swapChainImages = device.getSwapchainImagesKHR(swapChain);
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; auto& image : swapChainImages) {
			swapChainImageViews[i] = createImageView(image, swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
			++i;
		}
	}
	void createRenderPass() {
		auto colorAttachment = vk::AttachmentDescription(
			{},
			swapChainImageFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		);
		auto depthAttachment = vk::AttachmentDescription(
			{},
			findDepthFormat(),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		);

		auto colorAttachmentRef = vk::AttachmentReference(
			0,
			vk::ImageLayout::eColorAttachmentOptimal
		);
		auto depthAttachmentRef = vk::AttachmentReference(
			1,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		);

		auto subpass = vk::SubpassDescription(
			{},
			vk::PipelineBindPoint::eGraphics,
			{},
			colorAttachmentRef,
			{},
			&depthAttachmentRef
		);

		auto dependency = vk::SubpassDependency(
			vk::SubpassExternal,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput |
			vk::PipelineStageFlagBits::eLateFragmentTests,

			vk::PipelineStageFlagBits::eColorAttachmentOutput |
			vk::PipelineStageFlagBits::eEarlyFragmentTests,

			vk::AccessFlagBits::eDepthStencilAttachmentWrite,

			vk::AccessFlagBits::eColorAttachmentWrite |
			vk::AccessFlagBits::eDepthStencilAttachmentWrite
		);

		auto attachments = std::array{
			colorAttachment,
			depthAttachment
		};

		auto renderPassInfo = vk::RenderPassCreateInfo(
			{},
			attachments,
			subpass,
			dependency
		);

		renderPass = device.createRenderPass(renderPassInfo);
	}
	void createDescriptorSetLayout() {
		auto uboLayoutBinding = vk::DescriptorSetLayoutBinding(
			0,
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eVertex,
			nullptr
		);

		auto samplerBinding = vk::DescriptorSetLayoutBinding(
			1,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			nullptr
		);

		auto bindings = std::array{uboLayoutBinding, samplerBinding};

		auto layoutInfo = vk::DescriptorSetLayoutCreateInfo(
			{},
			bindings
		);

		descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
	}
	void createGraphicPipeline() {

		auto vertShaderModule = createShaderModule(readShaderCode("shaders/vert.spv"));
		auto fragShaderModule = createShaderModule(readShaderCode("shaders/frag.spv"));

		auto shaderStages = std::array{
			vk::PipelineShaderStageCreateInfo(
				{},
				vk::ShaderStageFlagBits::eVertex,
				vertShaderModule,
				"main"
			),
			vk::PipelineShaderStageCreateInfo(
				{},
				vk::ShaderStageFlagBits::eFragment,
				fragShaderModule,
				"main"
			),
		};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescriptions();

		auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
			{},
			bindingDescription,
			attributeDescription
		);

		auto dynamicStates = std::array{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		auto dynamicState = vk::PipelineDynamicStateCreateInfo(
			{},
			dynamicStates
		);

		auto inputAssenbly = vk::PipelineInputAssemblyStateCreateInfo(
			{},
			vk::PrimitiveTopology::eTriangleList,
			false
		);

		auto viewport = vk::Viewport(
			0.0f,
			0.0f,
			(float)swapChainExtent.width,
			(float)swapChainExtent.height,
			0.0f,
			1.0f
		);

		auto scissor = vk::Rect2D(
			{0, 0},
			swapChainExtent
		);

		auto viewportState = vk::PipelineViewportStateCreateInfo(
			{},
			viewport,
			scissor
		);

		auto rasterizer = vk::PipelineRasterizationStateCreateInfo(
			{},
			false,
			false,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise,
			false,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		);

		auto multisampling = vk::PipelineMultisampleStateCreateInfo(
			{},
			vk::SampleCountFlagBits::e1,
			false,
			1.0f,
			nullptr,
			false,
			false
		);

		auto depthStencil = vk::PipelineDepthStencilStateCreateInfo(
			{},
			true,
			true,
			vk::CompareOp::eLess,
			false,
			false,
			{},
			{},
			0.0f,
			1.0f
		);

		auto colorComponentFlags = vk::ColorComponentFlags();
		colorComponentFlags |= vk::ColorComponentFlagBits::eR;
		colorComponentFlags |= vk::ColorComponentFlagBits::eG;
		colorComponentFlags |= vk::ColorComponentFlagBits::eB;
		colorComponentFlags |= vk::ColorComponentFlagBits::eA;

		auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState(
			false,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			colorComponentFlags
		);

		auto colorBlending = vk::PipelineColorBlendStateCreateInfo(
			{},
			false,
			vk::LogicOp::eCopy,
			colorBlendAttachment,
			{0.0f,0.0f,0.0f,0.0f}
		);

		auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo(
			{},
			descriptorSetLayout
		);

		pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

		auto pipelineInfo = vk::GraphicsPipelineCreateInfo(
			{},
			shaderStages,
			&vertexInputInfo,
			&inputAssenbly,
			nullptr,
			&viewportState,
			&rasterizer,
			&multisampling,
			&depthStencil,
			&colorBlending,
			&dynamicState,
			pipelineLayout,
			renderPass,
			0,
			{},
			-1
		);

		auto pipelines = device.createGraphicsPipelines({}, pipelineInfo);

		if (!pipelines.has_value()) {
			initflag = false;
			return;
		}

		graphicsPipeline = (*pipelines).front();

		device.destroy(vertShaderModule);
		device.destroy(fragShaderModule);
	}
	void createCommandPool() {
		auto queueFamilyIndices = findQueueFamilies(physicalDevice);

		auto poolInfo = vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			queueFamilyIndices.graphicsFamily.value()
		);

		commandPool = device.createCommandPool(poolInfo);
	}
	void createDepthResources() {
		vk::Format depthFormat = findDepthFormat();

		std::tie(depthImage, depthImageMemory) = createImage(
			swapChainExtent.width, swapChainExtent.height,
			depthFormat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);

		transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}
	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		
		for (size_t i = 0; auto& imageView : swapChainImageViews) {
			auto attachments = std::array{
				imageView,
				depthImageView
			};

			auto framebufferInfo = vk::FramebufferCreateInfo(
				{},
				renderPass,
				attachments,
				swapChainExtent.width,
				swapChainExtent.height,
				1
			);

			swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);

			++i;
		}
	}
	void createTextureImage() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		std::tie(stagingBuffer, stagingBufferMemory) = createBuffer(
			imageSize,
			
			vk::BufferUsageFlagBits::eTransferSrc, 

			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		void* data = device.mapMemory(stagingBufferMemory, {}, imageSize, {});
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		device.unmapMemory(stagingBufferMemory);

		stbi_image_free(pixels);
		
		auto format = vk::Format::eR8G8B8A8Srgb;

		std::tie(textureImage, textureImageMemory) = createImage(
			texWidth,
			texHeight,
			format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		transitionImageLayout(textureImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		transitionImageLayout(textureImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		device.destroy(stagingBuffer);
		device.free(stagingBufferMemory);
	}
	void createTextureImageView() {
		textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
	}
	void createTextureSampler() {
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

		auto samplerInfo = vk::SamplerCreateInfo(
			{},
			vk::Filter::eLinear,
			vk::Filter::eLinear,
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			0.0f,
			true,
			properties.limits.maxSamplerAnisotropy,
			false,
			vk::CompareOp::eAlways,
			0.0f,
			0.0f,
			vk::BorderColor::eIntOpaqueBlack,
			false
		);

		textureSampler = device.createSampler(samplerInfo);
	}
	void createVertexBuffer() {
		vk::DeviceSize bufferSize = sizeof(decltype(vertices)::value_type) * vertices.size();

		auto [stagingBuffer, stagingBufferMemory] = createBuffer(
			bufferSize,

			vk::BufferUsageFlagBits::eTransferSrc,

			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		void* data = device.mapMemory(stagingBufferMemory, 0, bufferSize, {});
		memcpy(data, vertices.data(), (size_t)bufferSize);
		device.unmapMemory(stagingBufferMemory);

		std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(
			bufferSize,

			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eVertexBuffer,

			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		device.destroy(stagingBuffer);
		device.free(stagingBufferMemory);
	}
	void createIndexBuffer() {
		vk::DeviceSize bufferSize = sizeof(decltype(indices)::value_type) * indices.size();

		auto [stagingBuffer, stagingBufferMemory] = createBuffer(
			bufferSize,

			vk::BufferUsageFlagBits::eTransferSrc,

			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		void* data = device.mapMemory(stagingBufferMemory, 0, bufferSize, {});
		memcpy(data, indices.data(), (size_t)bufferSize);
		device.unmapMemory(stagingBufferMemory);

		std::tie(indexBuffer, indexBufferMemory) = createBuffer(
			bufferSize,

			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eIndexBuffer,

			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		device.destroy(stagingBuffer);
		device.free(stagingBufferMemory);
	}
	void createUniformBuffers() {
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			std::tie(uniformBuffers[i], uniformBuffersMemory[i]) = createBuffer(
				bufferSize,

				vk::BufferUsageFlagBits::eUniformBuffer,
				
				vk::MemoryPropertyFlagBits::eHostVisible |
				vk::MemoryPropertyFlagBits::eHostCoherent
			);
			uniformBuffersMapped[i] = device.mapMemory(uniformBuffersMemory[i], 0, bufferSize, {});
		}
	}
	void createDescriptorPool() {

		auto poolSizes = std::array{
			vk::DescriptorPoolSize(
				vk::DescriptorType::eUniformBuffer,
				static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
			),
			vk::DescriptorPoolSize(
				vk::DescriptorType::eCombinedImageSampler,
				static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
			)
		};

		auto poolInfo = vk::DescriptorPoolCreateInfo(
			{},
			static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
			poolSizes
		);

		descriptorPool = device.createDescriptorPool(poolInfo);
	}
	void createDescriptorSets() {
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

		auto allocInfo = vk::DescriptorSetAllocateInfo(
			descriptorPool,
			layouts
		);

		descriptorSets = device.allocateDescriptorSets(allocInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			auto bufferInfo = vk::DescriptorBufferInfo(
				uniformBuffers[i],
				0,
				sizeof(UniformBufferObject)
			);

			auto imageInfo = vk::DescriptorImageInfo(
				textureSampler,
				textureImageView,
				vk::ImageLayout::eShaderReadOnlyOptimal
			);

			auto descriptorWrites = std::array{
				vk::WriteDescriptorSet(
					descriptorSets[i],
					0,
					0,
					vk::DescriptorType::eUniformBuffer,
					{},
					bufferInfo,
					{}
				),
				vk::WriteDescriptorSet(
					descriptorSets[i],
					1,
					0,
					vk::DescriptorType::eCombinedImageSampler,
					imageInfo,
					{},
					{}
				)
			};

			device.updateDescriptorSets(descriptorWrites, {});
		}
	}
	void createCommandBuffers() {
		auto allocInfo = vk::CommandBufferAllocateInfo(
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			MAX_FRAMES_IN_FLIGHT
		);

		commandBuffer = device.allocateCommandBuffers(allocInfo);
	}
	void createSyncObjects() {
		imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

		auto semaphoreInfo = vk::SemaphoreCreateInfo();
		auto fenceInfo = vk::FenceCreateInfo(
			vk::FenceCreateFlagBits::eSignaled
		);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			imageAvailableSemaphore[i] = device.createSemaphore(semaphoreInfo);
			renderFinishedSemaphore[i] = device.createSemaphore(semaphoreInfo);
			inFlightFence[i] = device.createFence(fenceInfo);
		}
	}

	void cleanupSwapChain() {
		device.waitIdle();

		device.destroy(depthImage);
		device.destroy(depthImageView);
		device.free(depthImageMemory);
		for (auto& framebuffer : swapChainFramebuffers) {
			device.destroy(framebuffer);
		}
		for (auto& imageView : swapChainImageViews) {
			device.destroy(imageView);
		}
		device.destroy(swapChain);
	}
	void recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createDepthResources();
		createFramebuffers();
	}

	bool checkValidationLayerSupport() {
		auto availableLayers = vk::enumerateInstanceLayerProperties();

		for (auto layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (std::string_view(layerName) == layerProperties.layerName) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
	std::vector<const char*> getRequiredExtensions() {
		uint32_t extensionCount = 0;
		const char** pExtensions = nullptr;

		pExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

		auto extensions = std::vector<const char*>(pExtensions, pExtensions + extensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
	vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo() {

		auto severityflags = vk::DebugUtilsMessageSeverityFlagsEXT();
		severityflags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
		severityflags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		severityflags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

		auto messageTypes = vk::DebugUtilsMessageTypeFlagsEXT();
		messageTypes |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
		messageTypes |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
		messageTypes |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		return vk::DebugUtilsMessengerCreateInfoEXT({}, severityflags, messageTypes, &debugCallback, nullptr);
	}

	bool isDeviceSuitable(vk::PhysicalDevice device) {
		auto indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}
	bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : device.enumerateDeviceExtensionProperties()) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) {
		QueueFamilyIndices indices;

		auto queueFamilies = device.getQueueFamilyProperties();

		for (int i = 0; const auto& queueFamily : queueFamilies) {
			auto presentSupport = device.getSurfaceSupportKHR(i, surface);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			++i;
		}

		return indices;
	}
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};
	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) {
		SwapChainSupportDetails details;

		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		details.formats = device.getSurfaceFormatsKHR(surface);
		details.presentModes = device.getSurfacePresentModesKHR(surface);

		return details;
	}
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {

		for (const auto& availableForamt : availableFormats) {
			if (availableForamt.format == vk::Format::eB8G8R8A8Srgb && availableForamt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableForamt;
			}
		}

		return availableFormats[0];
	}
	vk::PresentModeKHR chooseSwapSurfacePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
		
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				return availablePresentMode;
			}
		}
		
		return vk::PresentModeKHR::eFifo;
	}
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	static std::vector<uint32_t> readShaderCode(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		
		if (!file.is_open()) {
			throw std::runtime_error("doesn't open file.");
		}

		size_t size = (size_t)file.tellg();
		std::vector<uint32_t> buffer((size - 1) / sizeof(uint32_t) + 1);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), size);

		file.close();

		return buffer;
	}
	vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code) {
		auto createInfo = vk::ShaderModuleCreateInfo(
			{},
			code
		);

		return device.createShaderModule(createInfo);
	}

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
		auto beginInfo = vk::CommandBufferBeginInfo(
			{},
			nullptr
		);

		commandBuffer.begin(beginInfo); {
			auto clearValues = std::array{
				vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)),
				vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0.0f))
			};

			auto renderPassInfo = vk::RenderPassBeginInfo(
				renderPass,
				swapChainFramebuffers[imageIndex],
				{{0,0}, swapChainExtent},
				clearValues
			);

			commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
			{
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

				auto viewport = vk::Viewport(
					0.0f,
					0.0f,
					(float)swapChainExtent.width,
					(float)swapChainExtent.height,
					0.0f,
					1.0f
				);
				commandBuffer.setViewport(0, viewport);

				auto scissor = vk::Rect2D({0,0}, swapChainExtent);
				commandBuffer.setScissor(0, scissor);

				vk::Buffer vertexBuffers[] = {vertexBuffer};
				vk::DeviceSize offsets[] = {0};
				commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
				commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets[currentFrame], {});
				commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

			} commandBuffer.endRenderPass();
		} commandBuffer.end();
	}

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties;
		memProperties = physicalDevice.getMemoryProperties();

		for (size_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
	auto createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) -> std::pair<vk::Buffer, vk::DeviceMemory> {
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;

		auto bufferInfo = vk::BufferCreateInfo(
			{},
			size,
			usage,
			vk::SharingMode::eExclusive
		);

		buffer = device.createBuffer(bufferInfo);

		auto memRequirements = vk::MemoryRequirements();
		memRequirements = device.getBufferMemoryRequirements(buffer);

		auto allocInfo = vk::MemoryAllocateInfo(
			memRequirements.size,
			findMemoryType(memRequirements.memoryTypeBits,
				properties
			)
		);

		bufferMemory = device.allocateMemory(allocInfo);
		device.bindBufferMemory(buffer, bufferMemory, 0);

		return {buffer, bufferMemory};
	}
	vk::CommandBuffer beginSingleTimeCommands() {
		auto allocInfo = vk::CommandBufferAllocateInfo(
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			1
		);

		vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo).front();

		auto beginInfo = vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}
	void endSignleTimeCommands(vk::CommandBuffer commandBuffer) {
		commandBuffer.end();

		auto submitInfo = vk::SubmitInfo(
			{},
			{},
			commandBuffer
		);

		graphicsQueue.submit(submitInfo);
		graphicsQueue.waitIdle();

		device.free(commandPool, commandBuffer);
	}
	auto copyBuffer(vk::Buffer srcBuffer, vk::Buffer destBuffer, vk::DeviceSize size) -> void {
		auto commandBuffer = beginSingleTimeCommands();

		auto copyRegion = vk::BufferCopy(
			0,
			0,
			size
		);

		commandBuffer.copyBuffer(srcBuffer, destBuffer, copyRegion);

		endSignleTimeCommands(commandBuffer);
	}

	auto createImage(
		uint32_t width, uint32_t height,
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties
	) -> std::pair<vk::Image, vk::DeviceMemory> {
		vk::Image image;
		vk::DeviceMemory imageMemory;

		auto imageInfo = vk::ImageCreateInfo(
			{},
			vk::ImageType::e2D,
			format,
			vk::Extent3D(width, height, 1),
			1,
			1,
			vk::SampleCountFlagBits::e1,
			tiling,
			usage,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined
		);

		image = device.createImage(imageInfo);

		auto memRequirements = vk::MemoryRequirements();
		memRequirements = device.getImageMemoryRequirements(image);

		auto allocInfo = vk::MemoryAllocateInfo(
			memRequirements.size,
			findMemoryType(memRequirements.memoryTypeBits, properties)
		);

		imageMemory = device.allocateMemory(allocInfo);

		device.bindImageMemory(image, imageMemory, 0);

		return {image, imageMemory};
	}
	void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
		auto commandBuffer = beginSingleTimeCommands();

		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags destStage;
		vk::AccessFlags srcFlags;
		vk::AccessFlags destFlags;
		vk::ImageAspectFlags aspectFlags;

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;

			if (hasStencilCompornent(format)) {
				aspectFlags |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			srcFlags = {};
			destFlags = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			srcFlags = vk::AccessFlagBits::eTransferWrite;
			destFlags = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			destStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			srcFlags = {};
			destFlags =
				vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		auto barrier = vk::ImageMemoryBarrier(
			srcFlags,
			destFlags,
			oldLayout,
			newLayout,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			image,
			vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)
		);


		commandBuffer.pipelineBarrier(
			srcStage, destStage,
			{},
			{},
			{},
			barrier
		);

		endSignleTimeCommands(commandBuffer);
	}
	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
		auto commandBuffer = beginSingleTimeCommands();

		auto region = vk::BufferImageCopy(
			0,
			0,
			0,
			vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
			vk::Offset3D{0,0,0},
			vk::Extent3D{width, height, 1}
		);

		commandBuffer.copyBufferToImage(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			region
		);

		endSignleTimeCommands(commandBuffer);
	}
	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
		auto viewInfo = vk::ImageViewCreateInfo(
			{},
			image,
			vk::ImageViewType::e2D,
			format,
			vk::ComponentMapping(
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			),
			vk::ImageSubresourceRange(
				aspectFlags,
				0,
				1,
				0,
				1
			)
		);

		vk::ImageView imageview;
		imageview = device.createImageView(viewInfo);

		return imageview;
	}

	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		for (auto format : candidates) {
			auto props = physicalDevice.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}
	vk::Format findDepthFormat() {
		return findSupportedFormat(
			{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}
	bool hasStencilCompornent(vk::Format format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void mainLoop() {
		auto extensions = vk::enumerateInstanceExtensionProperties();

		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}

		timer.Start();

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				break;
			}
		}
	}
	void drawFrame() {
		constexpr uint64_t timeout = (uint64_t)(-1);
		device.waitForFences(inFlightFence[currentFrame], true, timeout);
		device.resetFences(inFlightFence[currentFrame]);

		uint32_t imageIndex;
		auto result = device.acquireNextImageKHR(swapChain, timeout, imageAvailableSemaphore[currentFrame], {}, &imageIndex);

		if (result == vk::Result::eErrorOutOfDateKHR ||
			result == vk::Result::eSuboptimalKHR || 
			framebufferResized) {
			framebufferResized = true;
			recreateSwapChain();
		}
		else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		device.resetFences(inFlightFence[currentFrame]);
		commandBuffer[currentFrame].reset({});

		updateUniformBuffer(currentFrame);
		recordCommandBuffer(commandBuffer[currentFrame], imageIndex);

		vk::Semaphore waitSemaphores[] = {
			imageAvailableSemaphore[currentFrame]
		};
		vk::PipelineStageFlags waitStages[] = {
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};
		vk::Semaphore signalSemaphores[] = {
			renderFinishedSemaphore[currentFrame]
		};

		auto submitInfo = vk::SubmitInfo(
			waitSemaphores,
			waitStages,
			commandBuffer[currentFrame],
			signalSemaphores
		);

		graphicsQueue.submit(submitInfo, inFlightFence[currentFrame]);

		auto presentInfo = vk::PresentInfoKHR(
			signalSemaphores,
			swapChain,
			imageIndex,
			{}
		);

		presentQueue.presentKHR(presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void updateUniformBuffer(uint32_t currentImage) {
		float time = timer.GetElapsed().Second<float>();

		UniformBufferObject ubo{};

		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f) + pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}

	static vk::Bool32 debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		
		switch (messageSeverity) {
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			std::cerr << "[Verbose]: ";
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			std::cerr << "[Info]: ";
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			std::cerr << "[Warning]: ";
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			std::cerr << "[Error]: ";
			break;
		}

		std::cerr << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}