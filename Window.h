#pragma once

#include "PlatForm.h"
#include "HeaderFiles\Renderer.h"
#include <string>
#include <vector>

class Renderer;

class Window
{
public:
	Window(Renderer *rend , int x, int y, char* name);
	~Window();

	void Close();
	bool Update();
	
	VkSurfaceKHR get_surface();
	VkSwapchainKHR get_swap_chain();
	VkSwapchainCreateInfoKHR get_swap_chain_c_info();
	std::vector<VkImage> get_swap_chain_images();
	std::vector<VkImageView> get_swap_chain_image_views();

private:

	bool close_flag = 0;

	Renderer* renderer = nullptr;

	void Init_Window();
	
	void Dest_Window();
	void Update_Window();
	
	void Init_Surface();
	void Dest_Surface();

	void Init_SwapChain();
	void Dest_SwapChain();

	void Init_SwapChainImages();
	void Dest_SwapChainImages();

	VkSurfaceCapabilitiesKHR surface_cap = {};
	VkSurfaceFormatKHR surface_format = {};

	HINSTANCE _win32_instance = NULL;
	HWND _win32_window = NULL;
	std::string _win32_class_name;
	std::uint64_t _win32_class_id_counter;
	std::string window_name;

	VkSurfaceKHR surface = VK_NULL_HANDLE;

	uint32_t surface_size_x = 1920;
	uint32_t surface_size_y = 1080;

	VkSwapchainCreateInfoKHR swapChain_Cinfo = {};
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	uint32_t swapc_image_count = 2;

	uint32_t swapChain_image_count = 0;
	std::vector<VkImage> swapChain_images;
	std::vector<VkImageView> swapChain_image_views;

};

