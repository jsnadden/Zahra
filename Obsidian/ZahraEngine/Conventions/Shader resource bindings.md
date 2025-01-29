The Vulkan Spec [constrains the binding of descriptor sets](https://registry.khronos.org/vulkan/specs/1.3-khr-extensions/html/chap14.html#descriptorsets-compatibility) such that within each frame's command buffer recording, we should only bind resources for a given set $j$ after (or simultaneously with) doing so for every set $0 \leq i < j$ . For that reason, to minimise these calls, it's standard to assign resources to sets in order of the frequency with which they are bound. 

Moreover, from the same section of the spec: *"When binding a pipeline, the pipeline **can** correctly access any previously bound descriptor set N if it was bound with compatible pipeline layout for set N, and it was not disturbed."* Therefore we can use a single descriptor set binding across multiple render passes (again, with a mind towards reducing binding calls), providing we maintain some consistency between the shader uniform layouts! To this end, I [[Shader compilation|plan]] to implement `#include` directives in my shader source code, so that I can reuse separate "header files" with these general layout definitions.

Ultimately I'd like to use the following scheme for 3D scene rendering (subject to change and variation e.g. for instanced rendering):

| Set | Update Frequency  |                          Usage                           |  Managed by  |           Bound in            |
| :-: | :---------------: | :------------------------------------------------------: | :----------: | :---------------------------: |
|  0  |     Per frame     |   Scene environment (lights, skybox etc.) & camera(s)    | `RenderPass` | First call to BeginRenderPass |
|  1  |  Per render pass  | Pre/post-processing intermediate data (shadow maps etc.) | `RenderPass` |        BeginRenderPass        |
|  2  |   Per material    |                   Material data... duh                   |  `Material`  |          Draw calls           |
|  3  | Per mesh/instance |                   Skeletal positioning                   |     ???      |              ???              |
