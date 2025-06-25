using Djinn;
using Djinn.CustomAttributes;
using System;
using System.Collections.Generic;
using System.IO;

namespace Bud.Examples
{
	public class MazeGenerator : Entity
	{
		////////////////////////////////////////////////////////////////////////
		// Constructors
		public MazeGenerator() : base() {}
		public MazeGenerator(ulong uuid) : base(uuid) { }

		////////////////////////////////////////////////////////////////////////
		// Type defs
		private enum MazeType
		{
			Hedge, // hedge maze, blue + clouds skybox
			Manor, // corridors of a tudor-style manor house (paintings? windows?)
			Mine, // stone walls/ceiling, with wooden supports
			Dungeon, // stone brick walls/ceiling
			Ice, // icy crevasse (subsurface scattering?)
			Hell, // obsidian walls with bones and lava emerging, red firey skybox
		}

		private enum Direction
		{
			East = 0,
			North = 1,
			West = 2,
			South = 3
		}

		private struct GridSite
		{
			public int X,Y;
			public Dictionary<Direction, bool> NeighbourTo;

			public GridSite(int x, int y)
			{
				X = x;
				Y = y;
				NeighbourTo = new Dictionary<Direction, bool>();
			}
		}

		////////////////////////////////////////////////////////////////////////
		// Data fields
		private Random rng;
		private uint Width = 0, Height = 0;
		// TODO: should really only keep the sites in the chosen cluster, this 
		private Dictionary<(int, int), GridSite> Grid;
		private List<(int, int)> Cluster;
		private (int, int) Entrance, Exit;
		private List<ulong> ManagedEntities;

		//private TransformComponent transform;
		//[ExposedField] private float InputStrength_Linear;

		////////////////////////////////////////////////////////////////////////
		// Private methods
		private void ResetGrid(uint width, uint height)
		{
			Grid.Clear();

			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					Grid[(x, y)] = new GridSite(x, y);
				}
			}
		}

		private uint FindLongestPath()
		{
			uint pathLength = 0;

			// find a pair of points maximally far away (within the same cluster)
			// append each coordinate pair in the cluster to Cluster

			return pathLength;
		}

		private void GenerateMaze(double p, uint width, uint height, uint minMaxPathLength)
		{
			Width = width;
			Height = height;

			Cluster = new List<(int, int)>();

			ResetGrid(width, height);

			uint pathLength = 0;
			uint attempts = 0;
			while (pathLength < minMaxPathLength)
			{
				attempts++;

				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						if (x > 0)
						{
							bool addHorizontalEdge = rng.NextDouble() < p;
							Grid[(x, y)].NeighbourTo[Direction.West] = addHorizontalEdge;
							Grid[(x - 1, y)].NeighbourTo[Direction.East] = addHorizontalEdge;
						}

						if (y > 0)
						{
							bool addVerticalEdge = rng.NextDouble() < p;
							Grid[(x, y)].NeighbourTo[Direction.South] = addVerticalEdge;
							Grid[(x, y - 1)].NeighbourTo[Direction.North] = addVerticalEdge;
						}
					}
				}

				pathLength = FindLongestPath();
			}
			Log.Info($"Maze generation required {attempts} attempts");
		}

		private void PopulateScene(MazeType type)
		{
			ManagedEntities = new List<ulong>();

			// add floor/wall tiles, plus ceiling tiles or a skybox
			// add some kind of representation of a door/gate at the entrance and exit
			// move player to entrance, facing away from door
		}

		////////////////////////////////////////////////////////////////////////
		// Public methods
		public void OnCreate()
		{
			TimeSpan time = DateTime.UtcNow - new DateTime(1970, 1, 1);
			int seed = (int)time.TotalSeconds;
			rng = new Random(seed);

			// TEMP: (should really happen during runtime)
			GenerateMaze(.5, 10, 10, 20);
			PopulateScene(MazeType.Hedge);
		}

		// The OnEarlyUpdate method will be called each frame, before the physics engine is updated
		public void OnEarlyUpdate(float dt)
		{
			
		}

		// The OnLateUpdate method will be called each frame, after the physics engine is updated
		public void OnLateUpdate(float dt)
		{
			
		}

	}
}
