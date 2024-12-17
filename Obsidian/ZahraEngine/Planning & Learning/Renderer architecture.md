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

Rendering resource hierarchy:
1) A Mesh object should be able to load in triangular mesh data (stored in e.g. a .obj or .dat) and inputs this data into a a VertexBuffer and IndexBuffer that it can provide to the renderer.
2) A DynamicMesh object should augment a Mesh with all skeleton/animation data, and must manage their dynamic state
3) A Material object should contain a reference to an appropriate Shader, and a ShaderResourceSet, which builds layouts from the Shader reflection and manages the corresponding UniformBuffers, Textures etc.
4) A Scene should have an additional ShaderResourceSet that specifies the environmental data: lights, skybox, camera etc. Basically all the stuff that a Shader may need, independent of the particular mesh being drawn
5) Scene rendering should begin with submitting the environmental data, followed by a sequence of submissions of (Mesh, Material) pairs (with additional passes between/after for various effects and interactions e.g. shadow mapping and GUI overlays).

**Pseudo code of Hazelnut editor scene rendering:**
```
{
	editorScene->OnRenderEditor(sceneRenderer, dt, editorCamera)
	{
		Setup environmental rendering data (lights, skybox, cameras, etc.)

		if (viewport resize has happened)
			Update every framebuffer, image, etc. with the new size
	
		for (each entity in the scene with a (static or dynamic) mesh component)
		{
			for (each submesh)
			{
				add details to _____MeshDrawList
				add details to _____MeshShadowPassDrawList
			}
		}
	
		sceneRenderer->FlushDrawLists()
		{
			Begin recording command buffer
			
			ShadowMapPass()
			{
				Two passes per cascade (static, animated)
				All target the same depth buffer, but each cascade
				only targets a single layer
				First clears, second loads
			}
			
			SpotShadowMapPass()
			{
				Single render pass, with two pipelines (static, animated)
				Both targetting the same depth buffer
				Both clear 
			}
			
			PreDepthPass()
			{
				Three passes (static, animated, translucent)
				All three share a target depth buffer
				First clears, second and third load
			}
			
			HZBCompute()
			{
				COMPUTE
			}
			
			PreIntegration()
			{
				COMPUTE
			}
			
			LightCullingPass
			{
				COMPUTE
			}
			
			SkyboxPass()
			{
				
			}
			
			
			GeometryPass()		
			{
				
			}
			
			if (Ground Truth Ambient Occlusion is enabled)
			{
				GTAOCompute()
				{
					
				}
				GTAODenoiseCompute()
				{
					
				}
				AOComposite()
				{
					
				}
			}
			
			PreConvolutionCompute()
			{
				
			}
			
			
			if (Jump Flood is enabled)
				JumpFloodPass()
				{
					
				}
	
			if (Screen Space Reflection is enabled)
			{
				SSRCompute()
				{
					
				}
				SSRCompositePass()
				{
					
				}
			}
			
			if (Edge Outlining is enabled)
				EdgeDetectionPass()
				{
					
				}
			
			BloomCompute()
			{
				
			}
			
			
			CompositePass()
			{
				
			}
			

			End recording and submit command buffer

			Throw away the used environmental data and draw lists
		}
		
		Update 3d audio data

		renderer2D->BeginScene(camera view/proj matrices, bool depth test?)
		{
			Set camera uniform buffer data for current frame index

			Set index counts to 0 and batch pointers to their base value
			
			Reset texture slots/indices
		}

		Provide renderer2D with the compositing framebuffer from sceneRenderer as
		its own render target

		for (each entity with a 2d sprite component)
		{
			renderer2D->DrawQuad(sprite deets)
		}

		for (each entity with a text component)
		{
			renderer2D->DrawString(text deets)
		}

		Render 2d debug info
		
		renderer2D->EndScene()
		{
			Start recording command buffer

			for (each primitive type X)
				for (each X batch submitted this scene)
				{
					provide X batch vertex data to vertex buffer
					provide other rendering resources

					Renderer::BeginRenderPass(...)
					{
						Begin X render pass
						(using VkFramebuffer from swapchain or target framebuffer)
						
						Bind pipeline
						Bind descriptor sets
					}
					
					Renderer::RenderGeometry(...)
					{
						Bind vertex buffer
						Bind index buffer
						Bind material descriptor sets
						Set push constants
						Draw indexed
					}
					
					EndRenderPass()
					{
						End X render pass
					}
				}
			}
			
			End/submit command buffer
		}

		Then there's a second renderer2D that does a screenspace
		(i.e. UI, HUD etc.) pass to the same compositing framebuffer,
		with pretty much the same flow as above
	}

	After this, Editorlayer does an additional renderer2D pass, using the
	compositing framebuffer retrieved from sceneRenderer, this time drawing
	screenspace bounding boxes, outlines for selected entities, and icons for
	light/camera/audio components
	
	
}
```
