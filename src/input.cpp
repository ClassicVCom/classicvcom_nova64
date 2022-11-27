#include "input.hpp"
#include <fmt/core.h>

ClassicVCom_Nova64::Input::Input()
{
	Registers.input_device_gate = 0x01;
	for (uint8_t i = 0; i < interrupt_descriptor_table.size(); ++i)
	{
		interrupt_descriptor_table[i] = ChipsetInterruptDescriptorTableEntryData{ { 0, 0, 0 }, 0 };
	}
}

ClassicVCom_Nova64::Input::~Input()
{
}

void ClassicVCom_Nova64::Input::operator()(Word_LE &function, std::array<QWord_LE, 8> &args)
{
	switch (static_cast<InputChipsetFunction>(static_cast<uint16_t>(function)))
	{
		case InputChipsetFunction::RetrieveKeyboardData:
		{
			struct alignas(8)
			{
				Word_LE scancode;
				Word_LE modifiers;
				DWord_LE unused;
			} result = { 0, 0, 0 };
			KeyboardData data = PopKeyboardData(true);
			result.scancode = data.scancode;
			result.modifiers = data.modifiers;
			args[0] = std::bit_cast<QWord_LE>(result);
			break;
		}
		case InputChipsetFunction::RetrieveMouseData:
		{
			MouseData mouse_data = PopMouseData(true);
			switch (mouse_data.event_type)
			{
				case MouseEventType::MouseMotion:
				{
					MouseMotionData &mouse_motion = std::get<MouseMotionData>(mouse_data.data);
					struct alignas(8)
					{
						DWord_LE button_state;
					} button_result = { mouse_motion.button_state };
					struct alignas(8)
					{
						DWord_LE x;
						DWord_LE y;
					} coordinate_result = { mouse_motion.x, mouse_motion.y };
					struct alignas(8)
					{
						DWord_LE x_rel;
						DWord_LE y_rel;
					} relative_velocity_result = { mouse_motion.x_rel, mouse_motion.y_rel };
					args[0] = std::bit_cast<QWord_LE>(button_result);
					args[1] = std::bit_cast<QWord_LE>(coordinate_result);
					args[2] = std::bit_cast<QWord_LE>(relative_velocity_result);
					break;
				}
				case MouseEventType::MouseButtonDown:
				case MouseEventType::MouseButtonUp:
				{
					MouseButtonData &mouse_button = std::get<MouseButtonData>(mouse_data.data);
					struct alignas(8)
					{
						uint8_t button;
						uint8_t clicks;
					} button_result = { mouse_button.button, mouse_button.clicks };
					struct alignas(8)
					{
						DWord_LE x;
						DWord_LE y;
					} coordinate_result = { mouse_button.x , mouse_button.y };
					args[0] = std::bit_cast<QWord_LE>(button_result);
					args[1] = std::bit_cast<QWord_LE>(coordinate_result);
					break;
				}
				case MouseEventType::MouseWheel:
				{
					MouseWheelData &mouse_wheel = std::get<MouseWheelData>(mouse_data.data);
					struct alignas(8)
					{
						DWord_LE x;
						DWord_LE y;
					} wheel_result = { mouse_wheel.x, mouse_wheel.y };
					struct alignas(8)
					{
						DWord_LE direction;
					} wheel_extra_result = { mouse_wheel.direction };
					args[0] = std::bit_cast<QWord_LE>(wheel_result);
					args[1] = std::bit_cast<QWord_LE>(wheel_extra_result);
					break;
				}
				default:
				{
					args[0] = 0;
					break;
				}
			}
			break;
		}
	}
}
