- All shader stages in a given pipeline must share a single push constant block
- In the shader source code we add an additional layout declaration e.g.
```
	layout(push_constant) uniform constants
	{
		vec4 vector;
		mat4 matrix;
	} PushConstants;
```
- We provide `VkPushConstantRange` structs to `VkPipelineCreateInfo`, specifying which stages the push constant block is used in, and the sizes/offsets of each block member (i.e. its complete memory layout).
- To obtain these layouts we add a new loop in `VulkanShader::Reflect`:
	`for (const auto& resource : resources.push_constant_buffers[0]) { ... }`
- When recording command buffers at render time, we can set the values of our push constant uniforms using the command `VkCmdPushConstants`.


```

```