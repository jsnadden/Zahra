Rendering resources:
1) A `Mesh` object loads models (from e.g. a .obj or .dat file) and generates a `VertexBuffer` and `IndexBuffer`.
2) A `MeshTree` object is, shockingly, a tree data structure defining a hierarchical collection of `Mesh` objects and their relative transform data (see diagram below). The idea here is that a single entity in the game can be constructed from a collection of different meshes, each potentially with a different `Material`: e.g. a game character is comprised different body parts, clothing, held objects etc. with their own individual `(Mesh, Materia)` data.

```tikz
\usepackage{tikz-cd}

\begin{document}
\begin{tikzcd}

ModelSpace_1 \arrow[rd] \arrow[dr] & & \\
\vdots & LocalSpace \arrow[r] & WorldSpace \\
ModelSpace_N \arrow[ur] & &

\end{tikzcd}
\end{document}
```

3) A `DynamicMesh` object should augment a `MeshTree` with skeleton/animation data
4) A `Material` object is basically just a wrapper for a `Shader` and ``ShaderResourceManager``, managing a subset of the `UniformBuffers`, `Textures` etc. needed for model rendering