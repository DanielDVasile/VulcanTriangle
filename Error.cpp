#include "Error.h"

void CheckError(VkResult err, uint32_t exit_code)
{
	if (err != VK_SUCCESS) {
		exit(exit_code);
	}
}
