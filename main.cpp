

#include "HeaderFiles\Renderer.h"
#include <vulkan\vulkan.hpp>
#include "Error.h"

void draw_frame(Renderer rend);

int main() {

	char* wait;

	wait = (char*)malloc(50);

	Renderer rend;
	
	while (rend.run()) {
		draw_frame(rend);
	}

	vkQueueWaitIdle(rend.get_graphics_queue());

	rend.close();

	free(wait);

	return 0;
}

void draw_frame(Renderer rend) {
	uint32_t image_index;
	CheckError(vkAcquireNextImageKHR(rend.getDevice(), rend.get_window()->get_swap_chain(), UINT64_MAX, rend.get_image_ava(), VK_NULL_HANDLE, &image_index) , -39);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { rend.get_image_ava() };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = rend.get_pcommand_buffer_at(image_index);


	VkSemaphore signal_semaphore[] = { rend.get_render_finished() };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphore;

	CheckError( vkQueueSubmit( rend.get_graphics_queue(), 1, &submit_info , VK_NULL_HANDLE) , -36);

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphore;

	VkSwapchainKHR swap_chain_KHR[] = { rend.get_window()->get_swap_chain() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chain_KHR;
	present_info.pImageIndices = &image_index;

	CheckError( vkQueuePresentKHR( rend.get_present_queue() , &present_info) , 404 );
}