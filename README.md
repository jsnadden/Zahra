This project was spawned from my desire to learn general game engine design, and especially low-level graphics programming. My aim now is for this to demonstrate a knowledge of, and proficiency with, a wide range of software engineering concepts. Current features include:
 - A from-scratch renderer using the Vulkan API (replacing and improving on my previous OpenGL implementation).
 - A highly structured engine architecture, with robust platform abstractions, and a great deal of modularity.
 - An ECS-based scene system (with ENTT under the hood), with a growing number of component types, and full (de)serialisation capabilities.
 - 2D physics and collision detection, using Box2D. Fairly simple for now, though I intend to extend this at some stage.
 - An embedded Mono C# scripting engine (Djinn), fully integrated with the scene system.
 - A level editor app (Meadow) written using the engine's API, especially the built-in ImGui overlay.
 - Instrumentation tools for e.g. tracking engine memory usage and performance profiling.

Additionally, I've begun work on the following:
- A project system in Meadow, including both on-disk and in-memory asset management.
- Dedicated threads for e.g. buffering draw calls, or automatic updating asset file changes. 
- Modular, multi-pass, physically-based, 3D scene rendering (very early stages)
