#ifndef _GPU_HPP_
#define _GPU_HPP_

#include "renderer.hpp"
#include "chipset.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <array>
#include <vector>

namespace ClassicVCom_Nova64
{
	enum class GPUChipsetFunction
	{
		ClearScreen = 0,
		InitializeFontModules = 18,
		GenerateDefaultColorIndexTable = 32,
		CommitColorIndexTableChanges = 33
	};

	struct alignas(8) GraphicsModeControlData
	{
		Word_LE width;
		Word_LE height;
		uint8_t color_mode;
		uint8_t control_flags;
	};

	struct alignas(8) FramebufferControlRegisterData
	{
		uint8_t clear_color_index;
		uint8_t clear_color_index_table;
	};

	struct ColorIndexTableEntryData
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct ColorMaskControlData
	{
		uint8_t mask;
		uint8_t offset;
	};

	using ColorIndexTableData = std::array<ColorIndexTableEntryData, 256>;

	consteval ColorIndexTableData GenerateDefaultColorIndexTable(uint8_t color_mode, ColorMaskControlData red_mask, ColorMaskControlData green_mask, ColorMaskControlData blue_mask)
	{
		ColorIndexTableData color_index_table;
		uint16_t colors_available = (1 << (color_mode + 1));
		if (color_mode < 7)
		{
			for (size_t i = colors_available; i < color_index_table.size(); ++i)
			{
				color_index_table[i] = { 0, 0, 0 };
			}
		}
		for (uint16_t i = 0; i < colors_available; ++i)
		{
			float red = static_cast<float>((i & red_mask.mask) >> red_mask.offset) / static_cast<float>(red_mask.mask >> red_mask.offset);
			float green = static_cast<float>((i & green_mask.mask) >> green_mask.offset) / static_cast<float>(green_mask.mask >> green_mask.offset);
			float blue = static_cast<float>((i & blue_mask.mask) >> blue_mask.offset) / static_cast<float>(blue_mask.mask >> blue_mask.offset);
			color_index_table[i].red = static_cast<uint8_t>(red * 255.0f);
			color_index_table[i].green = static_cast<uint8_t>(green * 255.0f);
			color_index_table[i].blue = static_cast<uint8_t>(blue * 255.0f);
		}
		return color_index_table;
	}

	class GPU;

	class FontModule
	{
		public:
			FontModule();
			~FontModule();
			void Initialize();
			friend GPU;
		private:
			uint8_t font_color_mode;
			uint8_t font_mask_color;
			uint8_t font_character_width;
			uint8_t font_character_height;
			Word_LE font_x_coordinate_start;
			Word_LE font_y_coordinate_start;
			DWord_LE font_start_codepoint;
			Word_LE font_character_count;
			uint8_t font_texture;
			uint8_t unused;
			std::array<uint8_t, 256> font_color_index_table;
	};

	class GPU
	{
		public:
			GPU();
			~GPU();
			void operator()(Word_LE &function, std::array<QWord_LE, 8> &args);

			template <DWordCompatible T>
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
						size_t data_read = sizeof(T);
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
						if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
						{
							data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
						}
						switch (current_register)
						{
							case 0:
							{
								const uint8_t *GraphicsModeControlRegister = reinterpret_cast<const uint8_t *>(&Registers.graphics_mode_control);
								memcpy(&result, &GraphicsModeControlRegister[current_register_offset], data_read);
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
								const uint8_t *FramebufferControlRegister = reinterpret_cast<const uint8_t *>(&Registers.framebuffer_control);
								memcpy(&result, &FramebufferControlRegister[current_register_offset], data_read);
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
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						uint8_t color_index_table_index = group - 1;
						const ColorIndexTableData &current_color_index_table = ColorIndexTable[color_index_table_index];
						if (current_address + sizeof(T) - 1 >= (current_color_index_table.size() * sizeof(ColorIndexTableEntryData)))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *current_color_index_table_data = reinterpret_cast<const uint8_t *>(current_color_index_table.data());
						memcpy(&result, &current_color_index_table_data[current_address], sizeof(T));
						read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
						break;
					}
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						uint8_t font_module_index = group - 9;
						const FontModule &current_font_module = FontModules[font_module_index];
						if (current_address + sizeof(T) - 1 >= sizeof(current_font_module))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *current_font_module_data = reinterpret_cast<const uint8_t *>(&current_font_module);
						memcpy(&result, &current_font_module_data[current_address], sizeof(T));
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
						size_t data_write = sizeof(T);
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
						if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
						{
							data_write -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
						}
						switch (current_register)
						{
							case 0:
							{
								GraphicsModeControlData old_graphics_mode_control = Registers.graphics_mode_control;
								uint8_t *GraphicsModeControlRegister = reinterpret_cast<uint8_t *>(&Registers.graphics_mode_control);
								memcpy(&GraphicsModeControlRegister[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								if (Registers.graphics_mode_control.width < 32)
								{
									Registers.graphics_mode_control.width = 32;
								}
								else if (Registers.graphics_mode_control.width > 1920)
								{
									Registers.graphics_mode_control.width = 1920;
								}
								if (Registers.graphics_mode_control.height < 32)
								{
									Registers.graphics_mode_control.height = 32;
								}
								else if (Registers.graphics_mode_control.height > 1080)
								{
									Registers.graphics_mode_control.height = 1080;
								}
								if (Registers.graphics_mode_control.color_mode > 7)
								{
									Registers.graphics_mode_control.color_mode = 7;
								}
								bool resolution_change = (Registers.graphics_mode_control.width != old_graphics_mode_control.width || Registers.graphics_mode_control.height != old_graphics_mode_control.height);
								bool color_mode_change = (Registers.graphics_mode_control.color_mode != old_graphics_mode_control.color_mode);
								bool control_flag_change = (Registers.graphics_mode_control.control_flags != old_graphics_mode_control.control_flags);
								if (control_flag_change)
								{
									CurrentRenderer->SetVSync(Registers.graphics_mode_control.control_flags & 0x01);
								}
								if (resolution_change || color_mode_change)
								{
									if (resolution_change)
									{
										CurrentRenderer->SetResolution(Registers.graphics_mode_control.width, Registers.graphics_mode_control.height);
									}
									if (color_mode_change)
									{
										if (Registers.framebuffer_control.clear_color_index >= (1 << (Registers.graphics_mode_control.color_mode + 1)))
										{
											Registers.framebuffer_control.clear_color_index = 0;
											CurrentRenderer->UpdateFramebufferControl(Registers.framebuffer_control.clear_color_index, Registers.framebuffer_control.clear_color_index_table);
										}
										for (size_t i = 0; i < FontModules.size(); ++i)
										{
											FontModule &current_font_module = FontModules[i];
											if (current_font_module.font_color_mode > Registers.graphics_mode_control.color_mode)
											{
												current_font_module.font_color_mode = Registers.graphics_mode_control.color_mode;
											}
											uint8_t highest_color_index = (1 << (current_font_module.font_color_mode + 1)) - 1;
											if (current_font_module.font_mask_color > highest_color_index)
											{
												current_font_module.font_mask_color = highest_color_index;
											}	
										}
									}
									CurrentRenderer->ClearMainFramebuffer();
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							case 1:
							{
								uint8_t *FramebufferControlRegister = reinterpret_cast<uint8_t *>(&Registers.framebuffer_control);
								memcpy(&FramebufferControlRegister[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								if (Registers.framebuffer_control.clear_color_index >= (1 << (Registers.graphics_mode_control.color_mode + 1)))
								{
									Registers.framebuffer_control.clear_color_index = 0;
								}
								if (Registers.framebuffer_control.clear_color_index_table > 7)
								{
									Registers.framebuffer_control.clear_color_index_table = 7;
								}
								CurrentRenderer->UpdateFramebufferControl(Registers.framebuffer_control.clear_color_index, Registers.framebuffer_control.clear_color_index_table);
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
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						uint8_t color_index_table_index = group - 1;
						ColorIndexTableData &current_color_index_table = ColorIndexTable[color_index_table_index];
						if (current_address + sizeof(T) - 1 >= (current_color_index_table.size() * sizeof(ColorIndexTableEntryData)))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *current_color_index_table_data = reinterpret_cast<uint8_t *>(current_color_index_table.data());
						memcpy(&current_color_index_table_data[current_address], &data, sizeof(T));
						write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
						break;
					}
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						uint8_t font_module_index = group - 9;
						FontModule &current_font_module = FontModules[font_module_index];
						if (current_address + sizeof(T) - 1 >= sizeof(current_font_module))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *current_font_module_data = reinterpret_cast<uint8_t *>(&current_font_module);
						memcpy(&current_font_module_data[current_address], &data, sizeof(T));
						if (current_font_module.font_color_mode > Registers.graphics_mode_control.color_mode || current_font_module.font_color_mode > 7)
						{
							current_font_module.font_color_mode = Registers.graphics_mode_control.color_mode;
						}
						if (current_font_module.font_texture > 1)
						{
							current_font_module.font_texture = 1;
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

			inline void LinkWithRenderer(Renderer *renderer)
			{
				CurrentRenderer = renderer;
			}

			void CopyColorIndexTableToRenderer(uint8_t index);

			template <HasChipsetId T>
			friend std::string GetId(T &chipset);
		private:
			const std::string id = "NovaGFX-64";
			struct
			{
				GraphicsModeControlData graphics_mode_control;
				FramebufferControlRegisterData framebuffer_control;
			} Registers;
			std::array<ColorIndexTableData, 8> ColorIndexTable;
			std::array<FontModule, 16> FontModules;
			std::vector<uint8_t> direct_drawing_surface;
			Renderer *CurrentRenderer;

	};
}

#endif
