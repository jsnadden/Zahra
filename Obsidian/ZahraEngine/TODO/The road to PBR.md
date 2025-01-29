1) Clean up current state of affairs:
		- Move `ShaderResourceManager` into `RenderPass`
		- Add push constant capabilities, and use them for camera data (instead of uniforms)
2) Implement a generic `Material` class (basically just a generic wrapper for a bunch of uniforms, push constants, textures, etc.). This should include (de)serialisation
4) Implement an `AssetManager`!!! Assets include:
		- Textures
		- Meshes
		- Materials
		- Scripts
		- Scenes
		- Fonts
		- Audio
1) Make `SpriteComponent` hold an `AssetID` referring to a `Material`, instead of a `Texture2D` directly. This will, if nothing else, provide a means to test out the next step...
2) Add a new panel to the editor to inspect, edit, create and copy`Material` assets. Drag and drop functionality, view texture thumbnails etc.
3) Begin working on PBR shaders!!