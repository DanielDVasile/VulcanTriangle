

#include "HeaderFiles\Renderer.h"


#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Error.h"
#include <assert.h>


Renderer::Renderer()
{
	SetUp_layers_extensions();
	SetUp_Debug();
	Init_Renderer();
	Init_Debug();
	Init_Device();
	makeWindow(1280, 720, "Vulkan Program");
	Init_Graphics_Pipeline();
	Init_Frame_Buffer();
	Init_Command_Buffer();
	Init_Semaphores();
	make_present_family();
}

void Renderer::close()
{
	delete window;

	Dest_Semaphores();
	Dest_Frame_Buffer();
	Dest_Command_Buffer();
	Dest_PipeLine();
	Dest_Device();
	Dest_Debug();
	Dest_Renderer();
}

void Renderer::Init_Renderer()
{
	VkApplicationInfo App_Info = {};
	App_Info.apiVersion = VK_MAKE_VERSION(1,0,13);
	App_Info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	App_Info.pApplicationName = "Vulkan Tutorial One";

	VkInstanceCreateInfo ICF = {};
	ICF.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ICF.pApplicationInfo = &App_Info;
	ICF.enabledLayerCount = (uint32_t)instance_layers.size();
	ICF.ppEnabledLayerNames = instance_layers.data();
	ICF.enabledExtensionCount = (uint32_t)instance_extensions.size();
	ICF.ppEnabledExtensionNames = instance_extensions.data();

	auto err = vkCreateInstance(&ICF, nullptr, &instance);

	if (VK_SUCCESS != err) {
		std::cout << "There was an error with creating and instance";
		assert(0 && err);
		exit(-120);
	}
}

void Renderer::Dest_Renderer()
{
	vkDestroyInstance(instance, nullptr);
	instance = 0;
}

void Renderer::Init_Device()
{
	//var for # of gpus
	uint32_t gpuCount = 0;
	//get number of gpus
	vkEnumeratePhysicalDevices(instance, &gpuCount, 0);
	//var list to store the different gpus
	std::vector<VkPhysicalDevice> gpu_list(gpuCount);
	//load the gpus into the list
	vkEnumeratePhysicalDevices(instance, &gpuCount, gpu_list.data());
	//set gpu 0 as primary gpu
	gpu = gpu_list[0];
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

	std::cout << gpu_properties.deviceName << "\n";


	uint32_t fam_count;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu,&fam_count,0);
	std::vector<VkQueueFamilyProperties> fam_property_list(fam_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &fam_count, fam_property_list.data());
	
	uint32_t graphics_fam_index = -1;
	bool found = 0;
	for (uint32_t i = 0; i < fam_count; i++) {
		if (fam_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = 1;
			graphics_fam_index = i;
			graphics_queue_family_index = i;
		}
	}

	if (!found) {
		assert(0 && "Not found");
	}
	
	//priority queue var
	float queue_Pr[] = { 1.0f };
	//Device queue info used for ???
	DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	DeviceInfo.queueFamilyIndex = graphics_fam_index;
	DeviceInfo.queueCount = 1;
	DeviceInfo.pQueuePriorities = queue_Pr;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &DeviceInfo;
	device_create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();
	//create a device var
	CheckError( vkCreateDevice(gpu, &device_create_info, 0, &device) , -28);

	vkGetDeviceQueue(device, graphics_fam_index, 0, &graphics_queue);


	uint32_t layer_count = 0;
	vkEnumerateDeviceLayerProperties(gpu, &layer_count, nullptr);
	std::vector<VkLayerProperties> layers(layer_count);
	vkEnumerateDeviceLayerProperties(gpu, &layer_count, layers.data());
	std::cout << "Instance layers count " << layer_count << "\n";
	for (auto &i : layers) {
		std::cout << " " << i.layerName << "\t\t" << i.description << "\n";
	}
	std::cout << "\n";

	uint32_t extension_count = 0;
	vkEnumerateDeviceLayerProperties(gpu, &extension_count, nullptr);
	std::vector<VkLayerProperties> extension(extension_count);
	vkEnumerateDeviceLayerProperties(gpu, &extension_count, extension.data());
	std::cout << "Device layers count " << extension_count << "\n";
	for (auto &i : extension) {
		std::cout << " " << i.layerName << "\t\t" << i.description << "\n";
	}
	std::cout << "\n";

}

void Renderer::Dest_Device()
{
	vkDestroyDevice(device, 0);
	device = 0;
}

const	VkPhysicalDeviceProperties& Renderer::GetVulkanPhysicalDeviceProperties() const
{
	return gpu_properties;
}

void Renderer::make_present_family()
{
	uint32_t fam_count;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &fam_count, 0);
	std::vector<VkQueueFamilyProperties> fam_property_list(fam_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &fam_count, fam_property_list.data());

	uint32_t present_fam_index;
	for (uint32_t i = 0; i < fam_count; i++) {

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, window->get_surface(), &presentSupport);

		if (presentSupport) {
			present_fam_index = i;
		}

	}
	vkGetDeviceQueue(device, present_fam_index, 0, &present_queue);
}

VkCommandBuffer* Renderer::get_pcommand_buffer_at(uint32_t num)
{
	return &command_buffers[num];
}

Window* Renderer::get_window()
{
	return window;
}

VkSemaphore Renderer::get_image_ava()
{
	return image_ava;
}

VkSemaphore Renderer::get_render_finished()
{
	return render_finished;
}

std::vector<VkCommandBuffer> Renderer::get_command_buffers()
{
	return command_buffers;
}

const	uint32_t Renderer::getVulkanGraphicsQueueFamilyIndex() const
{
	return graphics_queue_family_index;
}

const VkInstance Renderer::getInstance() const
{
	return instance;
}

const VkPhysicalDevice Renderer::getPhysicalDevice() const
{
	return gpu;
}

const VkDevice Renderer::getDevice() const
{
	return device;
}

VkQueue Renderer::get_graphics_queue()  
{
	return graphics_queue;
}

VkQueue Renderer::get_present_queue()
{
	return present_queue;
}

Window *Renderer::makeWindow(uint32_t x, uint32_t y, char * name)
{
	window = new Window(this,x,y,name);
	return window;
} 

bool Renderer::run()
{
	if (window == nullptr) {
		std::cout << "window closed";
	}

	if (window != nullptr) {
		return window->Update();
	}
	return 1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT		flags,
	VkDebugReportObjectTypeEXT	obj_type,
	uint64_t					src_obj,
	size_t						location,
	int32_t						msg_code,
	const char *				layer_prefix,
	const char *				msg,
	void *						user_data
) 
{
	std::ostringstream stream;
	stream << "VKDBG: ";
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		stream << "INFO: ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		stream << "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		stream << "PERFORMANCE: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		stream << "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		stream << "DEBUG: ";
	}
	stream << "@[" << layer_prefix << "]: ";
	stream << msg << std::endl;
	std::cout << stream.str();

#if defined( _WIN32 )
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}

void Renderer::SetUp_Debug() 
{
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		0;
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");

	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//instance_extensions.push_back(VK_EXT_debug_report);
}

PFN_vkCreateDebugReportCallbackEXT		fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT		fvkDestroyDebugReportCallbackEXT = nullptr;

void Renderer::Init_Debug() 
{
	fvkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
										(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	fvkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
										(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT) {
		assert(0 && "Vulkan ERROR: Can't fetch debug function pointers.");
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(instance, &debug_callback_create_info, nullptr, &debug_report);
}


void Renderer::Dest_Debug()
{
	fvkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
	debug_report = VK_NULL_HANDLE;
}

void Renderer::SetUp_layers_extensions()
{
	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void Renderer::Init_Graphics_Pipeline()
{
	auto vert_shader_code = load_shader("vert.spv");
	auto frag_shader_code = load_shader("frag.spv");
	
	VkShaderModuleCreateInfo vert_shader_info{};
	vert_shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vert_shader_info.codeSize = vert_shader_code.size();
	vert_shader_info.pCode = (uint32_t*)vert_shader_code.data();

	
	VkShaderModuleCreateInfo frag_shader_info{};
	frag_shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	frag_shader_info.codeSize = frag_shader_code.size();
	frag_shader_info.pCode = (uint32_t*)frag_shader_code.data();
	

	CheckError(vkCreateShaderModule(device, &vert_shader_info, nullptr, &vert_shader_module), -20);
	CheckError(vkCreateShaderModule(device, &frag_shader_info, nullptr, &frag_shader_module), -20);

	VkPipelineShaderStageCreateInfo pipeline_shader_cInfo_vert{};
	pipeline_shader_cInfo_vert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipeline_shader_cInfo_vert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeline_shader_cInfo_vert.module = vert_shader_module;
	pipeline_shader_cInfo_vert.pName = "main";

	VkPipelineShaderStageCreateInfo pipeline_shader_cInfo_frag{};
	pipeline_shader_cInfo_frag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipeline_shader_cInfo_frag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeline_shader_cInfo_frag.module = frag_shader_module;
	pipeline_shader_cInfo_frag.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { pipeline_shader_cInfo_vert , pipeline_shader_cInfo_frag };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // OptionalW
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo input_create_info{};
	input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_create_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)window->get_swap_chain_c_info().imageExtent.width;
	viewport.height = (float)window->get_swap_chain_c_info().imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors{};
	scissors.extent = window->get_swap_chain_c_info().imageExtent;

	VkPipelineViewportStateCreateInfo viewport_state_cInfo{};
	viewport_state_cInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_cInfo.viewportCount = 1;
	viewport_state_cInfo.pViewports = &viewport;
	viewport_state_cInfo.scissorCount = 1;
	viewport_state_cInfo.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	CheckError(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline_layout) , -22);

	VkAttachmentDescription color_attach{};
	color_attach.format = window->get_swap_chain_c_info().imageFormat;
	color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency depend = {};
	depend.srcSubpass = VK_SUBPASS_EXTERNAL;
	depend.dstSubpass = 0;
	depend.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	depend.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &color_attach;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &depend;

	CheckError(vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass) , -23);

	VkGraphicsPipelineCreateInfo pipeline_create_info{};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.pColorBlendState = &colorBlending;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shaderStages;
	pipeline_create_info.pVertexInputState = &vertexInputInfo;
	pipeline_create_info.pInputAssemblyState = &input_create_info;
	pipeline_create_info.pViewportState = &viewport_state_cInfo;
	pipeline_create_info.pRasterizationState = &rasterizer;
	pipeline_create_info.pMultisampleState = &multisampling;
	pipeline_create_info.pColorBlendState = &colorBlending;
	pipeline_create_info.pDynamicState = nullptr;
	pipeline_create_info.pDepthStencilState = nullptr;

	pipeline_create_info.layout = pipeline_layout;
	pipeline_create_info.renderPass = render_pass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = -1;

	CheckError( vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipe_line) , -24 );

}

void Renderer::Init_Frame_Buffer()
{
	frame_buffers.resize( window->get_swap_chain_images().size() );

	for (size_t i = 0; i < window->get_swap_chain_image_views().size(); i++) {
		VkImageView attachments[] = { (window->get_swap_chain_image_views())[i] };

		VkFramebufferCreateInfo frame_buffer_info = {};
		frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_info.renderPass = render_pass;
		frame_buffer_info.attachmentCount = 1;
		frame_buffer_info.pAttachments = attachments;
		frame_buffer_info.height = window->get_swap_chain_c_info().imageExtent.height;
		frame_buffer_info.width = window->get_swap_chain_c_info().imageExtent.width;
		frame_buffer_info.layers = 1;

		CheckError( vkCreateFramebuffer(device, &frame_buffer_info, nullptr, &frame_buffers[i] ), -25);
	}

}

void Renderer::Dest_Frame_Buffer()
{
	for (int i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(device, frame_buffers[i], nullptr);
	}
}

void Renderer::Init_Command_Buffer()
{
	

	VkCommandPoolCreateInfo command_pool_create_info{};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;

	CheckError( vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool), -30);

	command_buffers.resize(window->get_swap_chain_images().size());

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

	CheckError(vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) , -31);

	for (size_t i = 0; i < command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(command_buffers[i], &begin_info);

		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = render_pass;
		render_pass_begin_info.framebuffer = frame_buffers[i];
		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = window->get_swap_chain_c_info().imageExtent;

		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clearColor;
	
		vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line);

		vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);

		CheckError( vkEndCommandBuffer(command_buffers[i]), -32);
	}
}

void Renderer::Dest_Command_Buffer()
{
	vkDestroyCommandPool(device, command_pool, nullptr);
}

void Renderer::Init_Semaphores()
{
	VkSemaphoreCreateInfo sema_create_info = {};
	sema_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	CheckError( vkCreateSemaphore(device, &sema_create_info, nullptr, &image_ava) , -33);
	CheckError( vkCreateSemaphore(device, &sema_create_info, nullptr, &render_finished), -33);
}

void Renderer::Dest_Semaphores()
{
	vkDestroySemaphore(device, image_ava, nullptr);
	vkDestroySemaphore(device, render_finished, nullptr);
}

std::vector<char> Renderer::load_shader(const std::string & filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		exit(-100);
	}

	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);
	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();

	return buffer;
}

void Renderer::Dest_PipeLine() {
	vkDestroyPipeline(device, pipe_line, nullptr);
	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	vkDestroyShaderModule(device, frag_shader_module, nullptr);
	vkDestroyShaderModule(device, vert_shader_module, nullptr);
	vkDestroyRenderPass(device, render_pass, nullptr);
}
