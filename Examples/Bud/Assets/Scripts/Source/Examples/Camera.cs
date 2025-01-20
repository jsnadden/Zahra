using Djinn;
using Djinn.CustomAttributes;

namespace Bud.Examples
{
	public class Camera : Entity
	{
		Camera() : base() { }
		Camera(ulong uuid) : base(uuid) { }

		// The [EntityID] attribute signals to Zahra's ScriptEngine that a ulong (64-bit
		// unsigned integer) should be treated as an entity ID
		[ExposedField, EntityID] public ulong targetID = 0;
		[ExposedField] public float followLag;
		[ExposedField] public float zoomSpeed;

		private TransformComponent transform;
		private CameraComponent camera;
		private Player target;


		public void OnCreate()
		{
			transform = GetComponent<TransformComponent>();
			camera = GetComponent<CameraComponent>();
		}

		public void OnEarlyUpdate(float dt)
		{
			if (targetID == 0)
			{
				// TODO: get target from targetID field instead of hardcoding a string
				Entity targetEntity = new Entity("circle");
				targetID = targetEntity.UUID;
				Log.Trace(string.Format("Attempting to target entity 'circle' with ID {0}", targetID));

				if (targetID != 0)
				{
					target = targetEntity.As<Player>();
				}
			}

			if (camera != null && camera.ProjectionType == ProjectionType.Perspective)
			{
				Vector3 zoom = Vector3.One;

				if (Input.IsMouseButtonPressed(MouseCode.Button3))
				{ 
					zoom.Z = 1.0f + zoomSpeed * dt;
				}
				if (Input.IsMouseButtonPressed(MouseCode.Button4))
				{
					zoom.Z = 1.0f - zoomSpeed * dt;
				}

				if (zoom.Z != 1.0f)
				{
					transform.Translation *= zoom ;
				}
					
			}
			
		}

		public void OnLateUpdate(float dt)
		{
			if (targetID != 0)
			{
				Vector3 horizontalDifference = target.Position - transform.Translation;
				horizontalDifference.Z = 0;
				transform.Translation += horizontalDifference * (dt / followLag);
			}
		}
	}
}
