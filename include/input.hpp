#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "chipset.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <array>
#include <deque>
#include <variant>
#include <SDL.h>

namespace ClassicVCom_Nova64
{
	enum class InputDeviceType
	{
		Keyboard = 0,
		Mouse = 1,
		Joystick = 2,
		Game_Controller = 3,
		MIDI_Input = 4,
		Microphone = 5
	};

	enum class GameInputDeviceType
	{
		Joystick, GameController
	};

	enum class InputChipsetFunction
	{
		RetrieveKeyboardData = 0,
		RetrieveMouseData = 1
	};

	enum class KeyboardEventType
	{
		None, KeyDown, KeyUp
	};

	enum class MouseEventType
	{
		None, MouseMotion, MouseButtonDown, MouseButtonUp, MouseWheel
	};

	enum class JoystickEventType
	{
		None, JoystickAxisMotion, JoystickBallMotion, JoystickHatMotion, JoystickButtonDown,
		JoystickButtonUp, JoystickAdded, JoystickRemoved
	};

	enum class GameControllerEventType
	{
		None, ControllerAxisMotion, ControllerButtonDown, ControllerButtonUp, ControllerDeviceAdded,
		ControllerDeviceRemoved
	};

	struct KeyboardData
	{
		KeyboardEventType event_type;
		SDL_Scancode scancode;
		Word_LE modifiers;
	};

	struct KeyboardDevice
	{
		std::deque<KeyboardData> data_queue;
		bool data_retrieved;
	};

	struct MouseEmptyData
	{
	};

	struct MouseMotionData
	{
		DWord_LE button_state;
		DWord_LE x; // Signed
		DWord_LE y; // Signed
		DWord_LE x_rel; // Signed
		DWord_LE y_rel; // Signed
	};

	struct MouseButtonData
	{
		DWord_LE x; // Signed
		DWord_LE y; // Signed
		uint8_t button;
		uint8_t clicks;
	};

	struct MouseWheelData
	{
		DWord_LE x; // Signed
		DWord_LE y; // Signed
		DWord_LE direction;
	};

	struct MouseData
	{
		MouseEventType event_type;
		std::variant<MouseEmptyData, MouseMotionData, MouseButtonData, MouseWheelData> data;
	};

	struct MouseDevice
	{
		std::deque<MouseData> data_queue;
		bool data_retrieved;
	};

	struct JoystickEmptyData
	{
	};

	struct JoystickAxisMotionData
	{
		Word_LE value; // Signed
		uint8_t axis;
		uint8_t port; // (0-3)
	};

	struct JoystickBallMotionData
	{
		Word_LE x_rel; // Signed
		Word_LE y_rel; // Signed
		uint8_t ball;
		uint8_t port; // (0-3)
	};

	struct JoystickButtonData
	{
		uint8_t button;
		uint8_t port; // (0-3)
	};

	struct JoystickAddRemoveData
	{
		DWord_LE index; // Signed
	};

	struct JoystickData
	{
		JoystickEventType event_type;
		std::variant<JoystickEmptyData, JoystickAxisMotionData, JoystickBallMotionData, JoystickButtonData, JoystickAddRemoveData> data;
	};

	struct JoystickDevice
	{
		std::deque<JoystickData> data_queue;
		bool data_retrieved;
	};

	struct GameControllerEmptyData
	{
	};

	struct GameControllerAxisMotionData
	{
		Word_LE value; // Signed
		uint8_t axis;
		uint8_t port; // (0-3)
	};

	struct GameControllerButtonData
	{
		uint8_t button;
		uint8_t port; // (0-3)
	};

	struct GameControllerAddRemoveData
	{
		DWord_LE index; // Signed
	};

	struct GameControllerData
	{
		GameControllerEventType event_type;
		std::variant<GameControllerEmptyData, GameControllerAxisMotionData, GameControllerButtonData, GameControllerAddRemoveData> data;
	};

	struct GameControllerDevice
	{
		std::deque<GameControllerData> data_queue;
		bool data_retrieved;
	};

	struct EmptyGameInputDevice
	{
	};

	struct GameInputDevice
	{
		SDL_JoystickID id;
		GameInputDeviceType device_type;
		std::variant<EmptyGameInputDevice, JoystickDevice, GameControllerDevice> data;
	};

	class Input
	{
		public:
			Input();
			~Input();
			void operator()(Word_LE &function, std::array<QWord_LE, 8> &args);
			
			template <DWordMaximumRequired T>
			inline T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T result = 0;
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 4:
							{
								size_t data_read = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								const uint8_t *input_device_gate_register_ptr = reinterpret_cast<const uint8_t *>(&Registers.input_device_gate);
								memcpy(&result, &input_device_gate_register_ptr[current_register_offset], data_read);
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<const uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&result, &interrupt_descriptor_table_ptr[current_address], sizeof(T));
						if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
						{
							IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
							target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
						}
						read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return result;
			}

			template <QWordRequired T>
			inline T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T result = 0;
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 0:
							{
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								memcpy(&result, &Registers.keyboard_input_status, sizeof(QWord_LE));
								Registers.keyboard_input_status = 0x00;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 1:
							{
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								memcpy(&result, &Registers.mouse_input_status, sizeof(QWord_LE));
								Registers.mouse_input_status = 0x00;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 4:
							{
								size_t data_read = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								const uint8_t *input_device_gate_register_ptr = reinterpret_cast<const uint8_t *>(&Registers.input_device_gate);
								memcpy(&result, &input_device_gate_register_ptr[current_register_offset], data_read);
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<const uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&result, &interrupt_descriptor_table_ptr[current_address], sizeof(T));
						if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
						{
							IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
							target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
						}
						read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return result;
			}

			template <DWordCompatible T>
			inline void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
			{
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 4:
							{
								size_t data_write = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_write -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								uint8_t *input_device_gate_register_ptr = reinterpret_cast<uint8_t *>(&Registers.input_device_gate);
								memcpy(&input_device_gate_register_ptr[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData  &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							default:
							{
								write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&interrupt_descriptor_table_ptr[current_address], &data, sizeof(T));
						if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
						{
							IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
							target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
						}
						write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
						break;
					}
					default:
					{
						write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
						break;
					}
				}
			}

			inline void EndOfInterrupt(uint8_t interrupt)
			{
				switch (interrupt)
				{
					case 0x00:
					{
						if (keyboard.data_retrieved)
						{
							keyboard.data_retrieved = false;
						}
						else
						{
							PopKeyboardData(false);
						}
						break;
					}
					case 0x01:
					{

						if (mouse.data_retrieved)
						{
							mouse.data_retrieved = false;
						}
						else
						{
							PopMouseData(false);
						}
						break;
					}
				}
			}
			
			inline void PushKeyboardData(KeyboardEventType event_type, SDL_Scancode scancode, Word_LE modifiers)
			{
				if (keyboard.data_queue.size() == 16) // Limit up to 16 keyboard data entries in the queue to prevent running out of memory by discarding old data.
				{
					PopKeyboardData(false);
				}
				keyboard.data_queue.push_back({ event_type, scancode, modifiers });
				if (keyboard.data_queue.size() == 1) // Set the Keyboard Input Status Register upon the first entry in the queue.
				{
					switch (event_type)
					{
						case KeyboardEventType::KeyDown:
						{
							Registers.keyboard_input_status = 0x01;
							break;
						}
						case KeyboardEventType::KeyUp:
						{
							Registers.keyboard_input_status = 0x02;
							break;	
						}
						default:
						{
							Registers.keyboard_input_status = 0x00;
							break;
						}
					}
				}
			}

			inline KeyboardData PopKeyboardData(bool retrieve)
			{
				if (keyboard.data_queue.size() > 0)
				{
					KeyboardData data = keyboard.data_queue.front();
					keyboard.data_queue.pop_front();
					if (retrieve)
					{
						keyboard.data_retrieved = true;
					}
					if (keyboard.data_queue.size() > 0)
					{
						KeyboardData new_data = keyboard.data_queue.front();
						switch (new_data.event_type)
						{
							case KeyboardEventType::KeyDown:
							{
								Registers.keyboard_input_status = 0x01;
								break;
							}
							case KeyboardEventType::KeyUp:
							{
								Registers.keyboard_input_status = 0x02;
								break;
							}
						}
					}
					else
					{
						Registers.keyboard_input_status = 0x00;
					}
					return data;
				}
				else
				{
					return { KeyboardEventType::None, SDL_SCANCODE_UNKNOWN, 0x0000 };
				}
			}

			inline void PushMouseData(MouseData data)
			{
				if (mouse.data_queue.size() == 16) // Limit up to 16 mouse data entries in the queue to prevent running out of memory by discarding old data.
				{
					PopMouseData(false);
				}
				mouse.data_queue.push_back(data);
				if (mouse.data_queue.size() == 1) // Set the Mouse Input Status Register upon the first entry in the queue.
				{
					switch (data.event_type)
					{
						case MouseEventType::MouseMotion:
						{
							Registers.mouse_input_status = 0x01;
							break;
						}
						case MouseEventType::MouseButtonDown:
						{
							Registers.mouse_input_status = 0x02;
							break;
						}
						case MouseEventType::MouseButtonUp:
						{
							Registers.mouse_input_status = 0x04;
							break;
						}
						case MouseEventType::MouseWheel:
						{
							Registers.mouse_input_status = 0x08;
							break;
						}
						default:
						{
							Registers.mouse_input_status = 0x00;
							break;
						}
					}
				}
			}

			inline MouseData PopMouseData(bool retrieve)
			{
				if (mouse.data_queue.size() > 0)
				{
					MouseData data = mouse.data_queue.front();
					mouse.data_queue.pop_front();
					if (retrieve)
					{
						mouse.data_retrieved = true;
					}
					if (mouse.data_queue.size() > 0)
					{
						MouseData new_data = mouse.data_queue.front();
						switch (new_data.event_type)
						{
							case MouseEventType::MouseMotion:
							{
								Registers.mouse_input_status = 0x01;
								break;
							}
							case MouseEventType::MouseButtonDown:
							{
								Registers.mouse_input_status = 0x02;
								break;
							}
							case MouseEventType::MouseButtonUp:
							{
								Registers.mouse_input_status = 0x04;
								break;
							}
							case MouseEventType::MouseWheel:
							{
								Registers.mouse_input_status = 0x08;
								break;
							}
							default:
							{
								Registers.mouse_input_status = 0x00;
								break;
							}
						}
					}
					else
					{
						Registers.mouse_input_status = 0x00;
					}
					return data;
				}
				else
				{
					return { MouseEventType::None, MouseEmptyData {} };
				}
			}

			inline QWord_LE GetInputDeviceGate() const
			{
				return Registers.input_device_gate;
			}
			
			template <HasInterruptDescriptorTable T>
			friend void SetInterrupt(T &chipset, uint8_t interrupt, uint64_t isr_addr);
			template <HasInterruptDescriptorTable T>
			friend ChipsetInterruptDescriptorTableEntryData GetInterrupt(T &chipset, uint8_t interrupt);
			template <HasChipsetId T>
			friend std::string GetId(T &chipset);
		private:
			const std::string id = "ClassicVInput-2";
			struct
			{
				QWord_LE keyboard_input_status;
				QWord_LE mouse_input_status;
				QWord_LE joystick_input_status;
				QWord_LE game_controller_input_status;
				QWord_LE input_device_gate;
			} Registers;
			std::array<ChipsetInterruptDescriptorTableEntryData, 5> interrupt_descriptor_table;
			KeyboardDevice keyboard;
			MouseDevice mouse;
			std::array<GameInputDevice, 4> game_input;
	};
}

#endif
