#include "gpu.hpp"
#include "utility.hpp"
#include "bits.hpp"
#include <bit>

ClassicVCom_Nova64::GPU::GPU() : CurrentRenderer(nullptr), direct_drawing_surface(4096 * 4096)
{
	Registers.graphics_mode_control = { 1280, 720, 7, 0x00 };
	Registers.framebuffer_control = { 0, 0 };
	ColorIndexTable[0] = GenerateDefaultColorIndexTable(7, { GenerateFieldBitmask<uint8_t>(5, 3), 5 }, { GenerateFieldBitmask<uint8_t>(2, 3), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
}

ClassicVCom_Nova64::GPU::~GPU()
{
}

void ClassicVCom_Nova64::GPU::operator()(Word_LE &function, std::array<QWord_LE, 8> &args)
{
	switch (static_cast<GPUChipsetFunction>(static_cast<uint16_t>(function)))
	{
		case GPUChipsetFunction::ClearScreen:
		{
			CurrentRenderer->ClearMainFramebuffer();
			break;
		}
		case GPUChipsetFunction::InitializeFontModules:
		{
			for (size_t i = 0; i < FontModules.size(); ++i)
			{
				FontModules[i].Initialize();
			}
			break;
		}
		case GPUChipsetFunction::GenerateDefaultColorIndexTable:
		{
			struct alignas(8) OperandControl
			{
				uint8_t color_index_table;
			} operand_control = std::bit_cast<OperandControl>(args[0]);
			if (operand_control.color_index_table < 8)
			{
				switch (Registers.graphics_mode_control.color_mode)
				{
					case 0:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(0, { 0x01, 0 }, { 0x01, 0 }, { 0x01, 0 });
						break;
					}
					case 1:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(1, { 0x03, 0 }, { 0x03, 0 }, { 0x03, 0 });
						break;
					}
					case 2:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(2, { GenerateFieldBitmask<uint8_t>(2, 1), 2 }, { GenerateFieldBitmask<uint8_t>(1, 1), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 3:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(3, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(1, 1), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 4:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(4, { GenerateFieldBitmask<uint8_t>(3, 2), 3 }, { GenerateFieldBitmask<uint8_t>(1, 2), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 5:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(5, { GenerateFieldBitmask<uint8_t>(4, 2), 4 }, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
					case 6:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(6, { GenerateFieldBitmask<uint8_t>(4, 3), 4 }, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
					case 7:
					{
						ColorIndexTable[operand_control.color_index_table] = GenerateDefaultColorIndexTable(7, { GenerateFieldBitmask<uint8_t>(5, 3), 5 }, { GenerateFieldBitmask<uint8_t>(2, 3), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
				}
				CopyColorIndexTableToRenderer(operand_control.color_index_table);
			}
			break;
		}
		case GPUChipsetFunction::CommitColorIndexTableChanges:
		{
			struct alignas(8) OperandControl
			{
				uint8_t color_index_table;
			} operand_control = std::bit_cast<OperandControl>(args[0]);
			CopyColorIndexTableToRenderer(operand_control.color_index_table);
			break;
		}
	}
}

void ClassicVCom_Nova64::GPU::CopyColorIndexTableToRenderer(uint8_t index)
{
	if (index < 8)
	{
		ColorIndexTableBufferData data;
		ColorIndexTableData &CurrentColorIndexTable = ColorIndexTable[index];
		for (size_t i = 0; i < CurrentColorIndexTable.size(); ++i)
		{
			data[i].red = static_cast<float>(CurrentColorIndexTable[i].red) / 255.0f;
			data[i].green = static_cast<float>(CurrentColorIndexTable[i].green) / 255.0f;
			data[i].blue = static_cast<float>(CurrentColorIndexTable[i].blue) / 255.0f;
			data[i].alpha = 1.0f;
		}
		if (CurrentRenderer != nullptr)
		{
			CurrentRenderer->UpdateColorIndexTableBuffer(index, data);
		}
	}
}

ClassicVCom_Nova64::FontModule::FontModule()
{
	Initialize();
}

ClassicVCom_Nova64::FontModule::~FontModule()
{
}

void ClassicVCom_Nova64::FontModule::Initialize()
{
	font_color_mode = 0;
	font_mask_color = 0;
	font_character_width = 0;
	font_character_height = 0;
	font_x_coordinate_start = 0;
	font_y_coordinate_start = 0;
	font_start_codepoint = 0;
	font_texture = 0;
	font_color_index_table = GenerateIdentityMap<uint8_t, 256>();
}
