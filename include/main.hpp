#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include "renderer.hpp"
#include "motherboard.hpp"
#include <memory>
#include <SDL.h>

namespace ClassicVCom_Nova64
{
	struct WindowDeleter
	{
		void operator()(SDL_Window *window)
		{
			SDL_DestroyWindow(window);
		}
	};
	
	using Window = std::unique_ptr<SDL_Window, WindowDeleter>;

	class Application
	{
		public:
			Application();
			~Application();
			void RunMainLoop();
			
			inline int GetReturnCode() const
			{
				return retcode;
			}
			
			inline bool Fail() const
			{
				return fail;
			}
		private:
			Motherboard MainMotherboard;
			int retcode;
			bool fail;
			Window MainWindow;
			Renderer MainRenderer;
	};
}

#endif
