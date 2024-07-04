#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Zahra
{
	class Meadow : public Application
	{
	public:

		Meadow()
			: Application("Meadow")
		{
			//PushLayer(new ExampleLayer());
			PushLayer(new EditorLayer());
		}

		~Meadow()
		{

		}


	};

	Application* CreateApplication()
	{
		return new Meadow();
	}

}

