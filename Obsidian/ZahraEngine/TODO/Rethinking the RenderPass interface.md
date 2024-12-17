 - Forget targeting the swap chain with these classes for now. Instead:
	 - `SceneRenderer` gets its own collection of render target images, including one called `m_FrameOutput` (or similar), which must be provided by the app's main layer.
	 - `Renderer` gets a method `DrawToSwapchain`, which takes a texture and renders it as a full-screen quad or triangle.
	 - `ImGuiLayer` gets a method `SetRenderTarget`, taking an `Image`. If never set, it can just use its own default target. Either way, it constructs a `Texture2D`, so that in `ImGuiLayer::End`, we can pass that to `DrawToSwapchain`. 
	 This setup covers the following cases:
	 1) For an app with `ImGuiLayer` disabled, at the end of the main app layer `Update` method we get `m_FrameOutput`, construct a texture from it, and pass that to `Renderer::DrawToSwapchain`.
	 2) For an app using `ImGui` as an overlay, we can pass `m_FrameOutput` to `ImGuiLayer::SetRenderTarget`, and in `ImGuiLayer::End` we first render on top of that, construct a texture from it, and pass that to `Renderer::DrawToSwapchain`. 
	 3) For the editor app, instead at `Layer::Update` we construct a texture from `m_FrameOutput` directly, pass that to `ImGuiLayer::RegisterTexture`, and call `ImGui::Image(...)` in `Layer::OnImGuiRender()` to draw it to the viewport. Then after `ImGuiLayer` renders, it can pass its output to `Renderer::DrawToSwapchain`.
	 The main thing to remember here is that if `ImGuiLayer` is enabled, we MUST NOT call `DrawToSwapchain` in other layers	
 - Rather than a `RenderPass` building its own `Framebuffer`, instead in `Renderer::Init()` we construct these separately:
	 1) First, for any attachment we want external access to, create an `Image`
	 2) Create a `Framebuffer`, passing existing attachments via spec, creating the rest in-place
	 3) Create a `RenderPass`, providing a `Framebuffer` in the spec	 
 - With `Framebuffer` objects targeting externally-defined `Image` objects, we have to be careful to avoid duplicate image resize calls, and to order things correctly:
	 1) Resizing cannot simply be "delete image, make new image", because that would break the inheritance graph. Instead keep the existing `Ref<Image>`, and give `Image` its own `Resize` method
	 2) A framebuffer can resize any image it created itself, but should leave alone any it inherits
	 3) Before resizing a framebuffer, any image it inherits must have already been resized.

**Pseudo code of Hazel's `VulkanFramebuffer` creation logic:**
```
{
	Set m_Width and m_Height (each from spec if that was non-zero, and otherwise 
	from window dimensions)

	for (each attachment specified)
	{
		if (attachment was specified with an existing image)
		{
			Add the existing image as an attachment
		}
		else
		{
			Create a new attachment
		}
	}
	
	for (each attachment specified)
	{
		Get/create the image again (this is silly...)
		
		Create a VkAttachmentDescription for this attachment
		{
			Everything here is set up so that all attachments can be
			used as textures immediately after rendered to:
				- format and loadOp are obtained from the spec
				- samples is set to XXX_1_BIT
				- storeOp is always set to XXX_STORE
				- initialLayout is set to XXX_UNDEFINED if the loadOp was
					XXX_CLEAR, and XXX_READ_ONLY_OPTIMAL otherwise
				- finalLayout is always set to
					..._(SHADER OR DEPTH_STENCIL)_READ_ONLY_OPTIMAL
		}
		
		Create a VkAttachmentReference for this attachment
	}
	
	VkSubpassDescription created
	
	External subpass dependencies are set up to make sure each render pass 
	finishes running its fragment shader stage and texture reads before the next
	pass can begin writing to any of its attachments. Once again, this is just
	there to make sure framebuffer attachments can be treated as textures in other 
	render passes
	
	Finally a VkRenderPass and a VkFramebuffer are created (nothing of note here)
}```
