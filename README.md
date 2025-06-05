# Custom Game Engine – In Progress

This project began as a personal exploration of game engine architecture and low-level graphics programming. It has since evolved into a comprehensive showcase of my software engineering skills, covering system design, real-time 3D graphics, UX design, tooling, and performance instrumentation.

## Key Features

- **Custom Vulkan Renderer**  
  Written from scratch, replacing an earlier OpenGL implementation. Provides fine-grained control over GPU resources and rendering pipelines.

- **Modular Engine Architecture**  
  Highly structured design with clear platform abstraction layers and strong modularity for extensibility and maintenance.

- **ECS-Based Scene System**  
  Built on [EnTT](https://github.com/skypjack/entt), supporting a variety of component types with full serialization/deserialization.

- **2D Physics Integration**  
  Basic physics and collision detection using Box2D, with room for future expansion.

- **C# Scripting Integration**  
  Embedded [Mono](https://www.mono-project.com/) runtime ("Djinn") for scripting, fully integrated with the ECS and engine events.

- **Level Editor Tool – Meadow**  
  Built using the engine itself, featuring a GUI powered by ImGui for live editing and engine interaction.

- **Instrumentation and Profiling Tools**  
  Includes memory usage tracking, frame timing, and customizable profiling overlays.

## In Development

- **Asset and Project System**  
  In-memory and on-disk project/asset management integrated into Meadow.

- **Multi-threaded Systems**  
  For tasks like draw call buffering and live asset reloading.

- **3D Rendering Pipeline**  
  Multi-pass, physically-based rendering pipeline currently in early development.

---

Feel free to explore the codebase and reach out with any feedback or questions.
