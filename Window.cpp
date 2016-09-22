

#include "Window.h"
#include <assert.h>
#include <iostream>
#include "Error.h"





Window::Window(Renderer *rend, int x, int y, char* name)
{
	surface_size_x = x;
	surface_size_y = y;
	window_name = name;
	renderer = rend;
	Init_Window();
	Init_Surface();
	Init_SwapChain();
	Init_SwapChainImages();
}

Window::~Window()
{
	Dest_SwapChainImages();
	Dest_SwapChain();
	Dest_Surface();
	Dest_Window();
}

bool Window::Update() {
	
	Update_Window();
	return !close_flag;
}

VkSurfaceKHR Window::get_surface()
{
	return surface;
}

VkSwapchainKHR Window::get_swap_chain()
{
	return swapChain;
}

VkSwapchainCreateInfoKHR Window::get_swap_chain_c_info()
{
	return swapChain_Cinfo;
}

std::vector<VkImage> Window::get_swap_chain_images()
{
	return swapChain_images;
}

std::vector<VkImageView> Window::get_swap_chain_image_views()
{
	return swapChain_image_views;
}


void Window::Close()
{
	close_flag = true;
}

LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_SIZE:
		// we get here if the window has changed size, we should rebuild most
		// of our window resources before rendering to this window again.
		// ( no need for this because our window sizing by hand is disabled )
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::Init_Window() {
	WNDCLASSEX win_class{};
	assert(surface_size_x > 0);
	assert(surface_size_y > 0);

	_win32_instance = GetModuleHandle(0);
	_win32_class_name = window_name + "_" + std::to_string(_win32_class_id_counter);
	_win32_class_id_counter++;

	win_class.cbSize = sizeof(win_class);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = _win32_instance;
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = _win32_class_name.c_str();
	win_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&win_class)) {
		CheckError(VK_ERROR_INVALID_SHADER_NV, -6);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// Create window with the registered class:
	RECT wr = { 0, 0, LONG(surface_size_x), LONG(surface_size_y) };
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	_win32_window = CreateWindowEx(0,
		_win32_class_name.c_str(),		// class name
		window_name.c_str(),			// app name
		style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		_win32_instance,				// hInstance
		NULL);							// no extra parameters
	if (!_win32_window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(_win32_window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(_win32_window, SW_SHOW);
	SetForegroundWindow(_win32_window);
	SetFocus(_win32_window);

}

void Window::Dest_Window() {
	DestroyWindow(_win32_window);
	UnregisterClass(_win32_class_name.c_str(), _win32_instance);
}

#ifdef _WIN32

void Window::Init_Surface() {

	VkPhysicalDevice gpu = renderer->getPhysicalDevice();
	VkWin32SurfaceCreateInfoKHR surface_info {};

	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = _win32_instance;
	surface_info.hwnd = _win32_window;
	CheckError( vkCreateWin32SurfaceKHR(renderer->getInstance(), &(surface_info), nullptr, &(surface)) , -5 );

	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu, renderer->getVulkanGraphicsQueueFamilyIndex(), surface, &WSI_supported);

	if (!WSI_supported) {
		assert(0 && "WSI Not Supported");
		exit(-9);
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_cap);
	if (surface_cap.currentExtent.width < UINT32_MAX) {
		surface_size_x = surface_cap.currentExtent.width;
		surface_size_y = surface_cap.currentExtent.height;
	}

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
	if (format_count == 0) {
		assert(0 && "No formats were detected "); 
		exit(-8);
	}
	std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surface_formats.data());
	if (surface_formats[0].format == VK_FORMAT_UNDEFINED) {
		surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
		surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		surface_format = surface_formats[0];
	}
}

#endif // if _WIN32

void Window::Update_Window() {
	MSG msg;
	if (PeekMessage(&msg, _win32_window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::Dest_Surface() {
	vkDestroySurfaceKHR(renderer->getInstance(), surface, 0);
}

void Window::Init_SwapChain()
{
	if (swapc_image_count > surface_cap.maxImageCount)
		swapc_image_count = surface_cap.maxImageCount;
	if (swapc_image_count < surface_cap.minImageCount + 1)
		swapc_image_count = surface_cap.minImageCount + 1;

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t present_modec;
	{
		vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->getPhysicalDevice(), surface,&present_modec, nullptr);
		std::vector<VkPresentModeKHR> present_mode_list(present_modec);
		vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->getPhysicalDevice(), surface, &present_modec, present_mode_list.data());
		for (auto i : present_mode_list ) {
			if (i == VK_PRESENT_MODE_MAILBOX_KHR)
				present_mode = i;
		}
	}
	//create swapChain creation info
	{
		swapChain_Cinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChain_Cinfo.surface = surface;
		swapChain_Cinfo.minImageCount = 2;
		swapChain_Cinfo.imageFormat = surface_format.format;
		swapChain_Cinfo.imageColorSpace = surface_format.colorSpace;
		swapChain_Cinfo.imageExtent.width = surface_size_x;
		swapChain_Cinfo.imageExtent.height = surface_size_y;
		swapChain_Cinfo.imageArrayLayers = 1;
		swapChain_Cinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChain_Cinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChain_Cinfo.queueFamilyIndexCount = 0;
		swapChain_Cinfo.pQueueFamilyIndices = nullptr;
		swapChain_Cinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapChain_Cinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChain_Cinfo.presentMode = present_mode;
		swapChain_Cinfo.clipped = VK_TRUE;
		swapChain_Cinfo.oldSwapchain = VK_NULL_HANDLE;
		vkCreateSwapchainKHR(renderer->getDevice(), &swapChain_Cinfo, nullptr, &swapChain);
	}

	vkGetSwapchainImagesKHR(renderer->getDevice(), swapChain, &swapChain_image_count, nullptr);
}

void Window::Dest_SwapChain()
{
	vkDestroySwapchainKHR(renderer->getDevice(), swapChain, nullptr);
}

void Window::Init_SwapChainImages()
{
	swapChain_images.resize(swapChain_image_count);
	swapChain_image_views.resize(swapChain_image_count);

	CheckError(vkGetSwapchainImagesKHR(renderer->getDevice(), swapChain, &swapChain_image_count, swapChain_images.data()) , -2 );

	for (uint32_t i = 0; i < swapChain_image_count; i++) {
		VkImageViewCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_create_info.image = swapChain_images[i];
		image_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_create_info.format = surface_format.format;
		image_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_create_info.subresourceRange.baseMipLevel = 0;
		image_create_info.subresourceRange.levelCount = 1;
		image_create_info.subresourceRange.baseArrayLayer = 0;
		image_create_info.subresourceRange.layerCount = 1;

		

		CheckError(vkCreateImageView(renderer->getDevice(), &image_create_info, nullptr, &swapChain_image_views[i]) , -3);
	}

}

void Window::Dest_SwapChainImages()
{
	for (auto i : swapChain_image_views) {
		vkDestroyImageView(renderer->getDevice(), i, nullptr);
	}
}
