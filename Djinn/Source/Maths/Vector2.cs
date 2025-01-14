
using System;

namespace Djinn
{
	public struct Vector2
	{

		public float X, Y;

		public static Vector2 Zero => new Vector2(0.0f);
		public static Vector2 One => new Vector2(1.0f);

		public Vector2(float scalar)
		{
			X = scalar;
			Y = scalar;
		}

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			return new Vector2(a.X + b.X, a.Y + b.Y);
		}

		public static Vector2 operator -(Vector2 a, Vector2 b)
		{
			return new Vector2(a.X - b.X, a.Y - b.Y);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public float NormSquared()
		{
			return X * X + Y * Y;
		}

		public float Norm()
		{
			return (float)Math.Sqrt(NormSquared());
		}

		public void Normalise()
		{
			float norm = Norm();
			if (norm == 0) return;

			float invNorm = 1 / Norm();
			X *= invNorm;
			Y *= invNorm;
		}

	}

}
