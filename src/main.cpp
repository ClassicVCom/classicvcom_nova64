#include "main.hpp"
#include "cvne.hpp"
#include "bits.hpp"
#include <fmt/core.h>

ClassicVCom_Nova64::Application::Application() : MainMotherboard(64 << 20), retcode(0), fail(false), MainWindow(nullptr)
{
	uint32_t flags = 0;
	SDL_Init(SDL_INIT_VIDEO);
	switch (renderer_type)
	{
		case RendererType::OpenGL:
		case RendererType::OpenGLES_3:
		{
			flags |= SDL_WINDOW_OPENGL;
			if (renderer_type == RendererType::OpenGL)
			{
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			}
			else
			{
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			}
			break;
		}
		case RendererType::Vulkan:
		{
			flags |= SDL_WINDOW_VULKAN;
			break;
		}
	}
	MainWindow = Window(SDL_CreateWindow("ClassicVCom Nova64", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, flags));
	if(!MainRenderer.Setup(*MainWindow))
	{
		fmt::print("Renderer Setup Error:  {}\n", MainRenderer.GetRendererError());
		fail = true;
		retcode = -1;
		return;
	}
	MainMotherboard.SetupGPU(&MainRenderer);
}
			
ClassicVCom_Nova64::Application::~Application()
{
	SDL_Quit();
}

void ClassicVCom_Nova64::Application::RunMainLoop()
{
	bool exit = false;
	CPU *MainCPU = MainMotherboard.GetCPU();
	Input *MainInput = MainMotherboard.GetInput();
	MainCPU->LoadTestProgram();
	while (!exit)
	{
		SDL_Event event;
		QWord_LE input_device_gate = MainInput->GetInputDeviceGate();
		while (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
				{
					if (input_device_gate & 0x01)
					{
						MainInput->PushKeyboardData(KeyboardEventType::KeyDown, event.key.keysym.scancode, event.key.keysym.mod);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x00);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x00>();
						}
					}
					break;
				}
				case SDL_KEYUP:
				{
					if (input_device_gate & 0x01)
					{
						MainInput->PushKeyboardData(KeyboardEventType::KeyUp, event.key.keysym.scancode, event.key.keysym.mod);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x00);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x00>();
						}
					}
					break;
				}
				case SDL_MOUSEMOTION:
				{
					if (input_device_gate & 0x02)
					{
						MouseData mouse_data;
						mouse_data.event_type = MouseEventType::MouseMotion;
						mouse_data.data = MouseMotionData{ event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel };
						MainInput->PushMouseData(mouse_data);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x01);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x01>();
						}
					}
					break;
				}
				case SDL_MOUSEBUTTONDOWN:
				{
					if (input_device_gate & 0x02)
					{
						MouseData mouse_data;
						mouse_data.event_type = MouseEventType::MouseButtonDown;
						mouse_data.data = MouseButtonData{ event.button.x, event.button.y, event.button.button, event.button.clicks };
						MainInput->PushMouseData(mouse_data);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x01);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x01>();
						}
					}
					break;
				}
				case SDL_MOUSEBUTTONUP:
				{
					if (input_device_gate & 0x02)
					{
						MouseData mouse_data;
						mouse_data.event_type = MouseEventType::MouseButtonUp;
						mouse_data.data = MouseButtonData{ event.button.x, event.button.y, event.button.button, event.button.clicks };
						MainInput->PushMouseData(mouse_data);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x01);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x01>();
						}
					}
					break;
				}
				case SDL_MOUSEWHEEL:
				{
					if (input_device_gate & 0x02)
					{
						MouseData mouse_data;
						mouse_data.event_type = MouseEventType::MouseWheel;
						mouse_data.data = MouseWheelData{ event.wheel.x, event.wheel.y, event.wheel.direction };
						MainInput->PushMouseData(mouse_data);
						ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(*MainInput, 0x01);
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							MainCPU->IssueInterruptRequest<0x03, 0x01>();
						}
					}
					break;
				}
				case SDL_QUIT:
				{
					exit = true;
					break;
				}
			}
		}
		if (MainCPU->IsRunning())
		{
			MainCPU->RunCycles();
		}
		MainRenderer.Render(*MainWindow);
	}
}

/*
int ClassicVCom_Nova64::Application::GetReturnCode() const
{
	return retcode;
}

bool ClassicVCom_Nova64::Application::Fail() const
{
	return fail;
}
*/

int main(int argc, char *argv[])
{
	ClassicVCom_Nova64::Application MainApp;
	if (!MainApp.Fail())
	{
		MainApp.RunMainLoop();
	}
	return MainApp.GetReturnCode();
}
