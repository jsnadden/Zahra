1) Clean up current state of affairs:
		~~- Move `ShaderResourceManager` into `RenderPass`~~
		- Add [[Vulkan push constants|push constant]] capabilities to `ShaderResourceManager`
2) Implement a generic `Material` class (basically just wrapper for a `ShaderResourceManager`), extending this with (de)serialisation, and interfaces for the editor AND asset systems
3) Implement an `AssetManager`!!! Assets include:
		- Textures
		- Meshes
		- Materials
		- Scripts
		- Scenes
		- Fonts
		- Audio
4) Make `SpriteComponent` hold an `AssetID` referring to a `Material`, instead of a `Texture2D` directly. This will, if nothing else, provide a means to test out the next step...
5) Add a new panel to the editor to inspect, edit, create and copy`Material` assets. Drag and drop functionality, view texture thumbnails etc.
6) Begin working on PBR shaders!!