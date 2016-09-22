#pragma once

#include "PlatForm.h"
#include <iostream>
#include "Window.h"

class Window;

class Renderer
{
public:
	Renderer();

	const	VkInstance getInstance() const;
	const   VkPhysicalDevice getPhysicalDevice() const;
	const	VkDevice getDevice() const;
	VkQueue get_graphics_queue();
	VkQueue get_present_queue();
	const	uint32_t getVulkanGraphicsQueueFamilyIndex() const;
	const	VkPhysicalDeviceProperties& GetVulkanPhysicalDeviceProperties() const;
	void make_present_family();
	VkCommandBuffer* get_pcommand_buffer_at(uint32_t num);
	void close();

	Window* get_window();
	VkSemaphore get_image_ava();
	VkSemaphore get_render_finished();
	std::vector<VkCommandBuffer> get_command_buffers();

	Window *makeWindow(uint32_t x, uint32_t y, char* name);

	bool run();

	
private:

	void Init_Renderer();
	void Dest_Renderer();

	void Init_Device();
	void Dest_Device();

	void SetUp_Debug();
	void Init_Debug();
	void Dest_Debug();
	void SetUp_layers_extensions();

	void Init_Graphics_Pipeline();
	void Dest_PipeLine();

	void Init_Frame_Buffer();
	void Dest_Frame_Buffer();

	void Init_Command_Buffer();
	void Dest_Command_Buffer();

	void Init_Semaphores();
	void Dest_Semaphores();

	std::vector<char> load_shader(const std::string& filename);
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;

	VkPipeline pipe_line;

	std::vector<VkFramebuffer> frame_buffers{};
	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers{};

	VkSemaphore image_ava = {};
	VkSemaphore render_finished = {};


	VkInstance instance = 0;
	VkDevice device = 0;
	VkDeviceQueueCreateInfo DeviceInfo = {};
	VkPhysicalDevice gpu = 0;
	VkPhysicalDeviceProperties gpu_properties = {};
	VkQueue graphics_queue = nullptr;
	VkQueue present_queue = nullptr;
	uint32_t graphics_queue_family_index;

	Window* window = nullptr;


	std::vector<const char*>				instance_layers;
	std::vector<const char*>				instance_extensions;
	std::vector<const char*>				device_extensions;

	VkDebugReportCallbackEXT				debug_report = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT		debug_callback_create_info = {};
		
};

