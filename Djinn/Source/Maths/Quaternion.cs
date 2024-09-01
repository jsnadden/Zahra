
using System;

namespace Djinn
{
	public struct Quaternion
	{
		public float W, X, Y, Z;

		public static Quaternion Zero	=> new Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
		public static Quaternion One	=> new Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
		public static Quaternion I		=> new Quaternion(0.0f, 1.0f, 0.0f, 0.0f);
		public static Quaternion J		=> new Quaternion(0.0f, 0.0f, 1.0f, 0.0f);
		public static Quaternion K		=> new Quaternion(0.0f, 0.0f, 0.0f, 1.0f);


		public Quaternion(float w, float x, float y, float z)
		{
			W = w;
			X = x;
			Y = y;
			Z = z;
		}

		public Quaternion(float w, Vector3 xyz)
		{
			X = xyz.X;
			Y = xyz.Y;
			Z = xyz.Z;
			W = w;
		}

		public Quaternion(Vector3 eulers) // I believe this is Z->Y->X order (i.e. Rx*Ry*Rz in SO(3))  
		{
			Vector3 c = new Vector3((float)Math.Cos(eulers.X * 0.5f), (float)Math.Cos(eulers.Y * 0.5f), (float)Math.Cos(eulers.Z * 0.5f));
			Vector3 s = new Vector3((float)Math.Sin(eulers.X * 0.5f), (float)Math.Sin(eulers.Y * 0.5f), (float)Math.Sin(eulers.Z * 0.5f));

			W = c.X * c.Y * c.Z + s.X * s.Y * s.Z;
			X = s.X * c.Y * c.Z - c.X * s.Y * s.Z;
			Y = c.X * s.Y * c.Z + s.X * c.Y * s.Z;
			Z = c.X * c.Y * s.Z - s.X * s.Y * c.Z;
		}

		public float Real
		{
			get { return W; }
			set { W = value; }
		}

		public Vector3 Imaginary
		{
			get => new Vector3(X, Y, Z);
			set
			{
				X = value.X;
				Y = value.Y;
				Z = value.Z;
			}
		}

		public float Norm()
		{
			return (float)Math.Sqrt(NormSquared());
		}

		public float NormSquared()
		{
			return W * W + X * X + Y * Y + Z * Z;
		}

		public void Normalise()
		{
			float norm = Norm();

			if (norm == 0) System.Diagnostics.Debugger.Break();

			float invNorm = 1 / norm;
			W *= invNorm;
			X *= invNorm;
			Y *= invNorm;
			Z *= invNorm;
		}

		public Quaternion Conjugate()
		{
			return new Quaternion(W, -X, -Y, -Z);
		}

		public Quaternion Inverse()
		{
			float ns = NormSquared();

			if (ns == 0) System.Diagnostics.Debugger.Break();

			return Conjugate() * (1 / ns);
		}

		public static Quaternion operator +(Quaternion a, Quaternion b)
		{
			return new Quaternion(a.W + b.W, a.X + b.X, a.Y + b.Y, a.Z + b.Z);
		}

		public static Quaternion operator *(Quaternion a, float b)
		{
			return new Quaternion( b * a.W, b * a.X, b * a.Y, b * a.Z);
		}

		public static Quaternion operator *(Quaternion a, Quaternion b)
		{
			return new Quaternion(
				a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z,
				a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
				a.W * b.Y + a.Y * b.W + a.Z * b.X - a.X * b.Z,
				a.W * b.Z + a.Z * b.W + a.X * b.Y - a.Y * b.X
				);
		}

		public Vector3 Rotate(Vector3 u)
		{
			Quaternion q = new Quaternion(0.0f, u);
			Vector3 v = ((this * q) * this.Conjugate()).Imaginary;
			return v;
		}
	}
}
