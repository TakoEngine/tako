export module Tako.Application;

namespace tako
{
	export class Application
	{
	public:
		static int argc;
		static char** argv;
	};
}

int tako::Application::argc = 0;
char** tako::Application::argv = nullptr;
