#pragma once

namespace Zahra
{

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

	private:
		static void InitMonoDomains();
		static void LoadCoreAssembly(std::filesystem::path filepath);
		static void ShutdownMonoDomains();

		static void ReflectAssemblyTypes();
	};

}
