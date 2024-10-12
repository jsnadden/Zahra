Hazel's renderer has a (cyclically indexed) list of command queues to submit render commands to. During each game loop the main (aka app) thread is submitting to the current queue, while the render thread is working on executing commands from the NEXT queue. Since all queues are empty at initialisation, drawing will only begin once we're on the last available queue.

HAZEL APPLICATION RUN LOOP:
1) Wait for render thread to complete command queue execution
2) Poll system/input events
3) Cycle render command queues and kick off the render thread to begin executing the next one
4) SUBMIT to current render command queue:
	- Process resource release queue (frees resources marked as such)
	- Get next image from swap chain
	- Reset Vulkan command pools
	- Reset Vulkan descriptor pools
5) Update all layers (potentially including more render command submissions e.g. draw calls)
6) SUBMIT to current render command queue:
	- Render ImGui content for each layer
	- Vulkan presentation commands (using semaphores/fences)
7) Compute frame time and increment frame count

I want to replicate this stuff eventually, but for now I'm just going to keep to a single CPU thread, without the internal command queues. The loop can simply be
1) Compute frame time
2) Poll events
3) Get next swap chain image
4) Reset Vulkan memory pools
5) Update layers
6) Render ImGui
7) Present rendered image