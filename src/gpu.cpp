#include "gpu.hpp"
#include "utility.hpp"
#include "bits.hpp"
#include <bit>

ClassicVCom_Nova64::GPU::GPU() : CurrentRenderer(nullptr), direct_drawing_surface(4096 * 4096)
{
	Registers.graphics_mode_control = { 1280, 720, 7, 0x00 };
	Registers.framebuffer_control = { 0, 0 };
	PaletteTable[0] = GenerateDefaultPaletteTable(7, { GenerateFieldBitmask<uint8_t>(5, 3), 5 }, { GenerateFieldBitmask<uint8_t>(2, 3), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
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
		case GPUChipsetFunction::GenerateDefaultPaletteTable:
		{
			struct alignas(8) OperandControl
			{
				uint8_t palette_table;
			} operand_control = std::bit_cast<OperandControl>(args[0]);
			if (operand_control.palette_table < 8)
			{
				switch (Registers.graphics_mode_control.color_mode)
				{
					case 0:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(0, { 0x01, 0 }, { 0x01, 0 }, { 0x01, 0 });
						break;
					}
					case 1:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(1, { 0x03, 0 }, { 0x03, 0 }, { 0x03, 0 });
						break;
					}
					case 2:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(2, { GenerateFieldBitmask<uint8_t>(2, 1), 2 }, { GenerateFieldBitmask<uint8_t>(1, 1), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 3:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(3, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(1, 1), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 4:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(4, { GenerateFieldBitmask<uint8_t>(3, 2), 3 }, { GenerateFieldBitmask<uint8_t>(1, 2), 1 }, { GenerateFieldBitmask<uint8_t>(0, 1), 0 });
						break;
					}
					case 5:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(5, { GenerateFieldBitmask<uint8_t>(4, 2), 4 }, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
					case 6:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(6, { GenerateFieldBitmask<uint8_t>(4, 3), 4 }, { GenerateFieldBitmask<uint8_t>(2, 2), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
					case 7:
					{
						PaletteTable[operand_control.palette_table] = GenerateDefaultPaletteTable(7, { GenerateFieldBitmask<uint8_t>(5, 3), 5 }, { GenerateFieldBitmask<uint8_t>(2, 3), 2 }, { GenerateFieldBitmask<uint8_t>(0, 2), 0 });
						break;
					}
				}
				CopyPaletteTableToRenderer(operand_control.palette_table);
			}
			break;
		}
		case GPUChipsetFunction::CommitPaletteTableChanges:
		{
			struct alignas(8) OperandControl
			{
				uint8_t palette_table;
			} operand_control = std::bit_cast<OperandControl>(args[0]);
			CopyPaletteTableToRenderer(operand_control.palette_table);
			break;
		}
	}
}

void ClassicVCom_Nova64::GPU::CopyPaletteTableToRenderer(uint8_t index)
{
	if (index < 8)
	{
		PaletteTableBufferData data;
		PaletteTableData &CurrentPaletteTable = PaletteTable[index];
		for (size_t i = 0; i < CurrentPaletteTable.size(); ++i)
		{
			data[i].red = static_cast<float>(CurrentPaletteTable[i].red) / 255.0f;
			data[i].green = static_cast<float>(CurrentPaletteTable[i].green) / 255.0f;
			data[i].blue = static_cast<float>(CurrentPaletteTable[i].blue) / 255.0f;
			data[i].alpha = 1.0f;
		}
		if (CurrentRenderer != nullptr)
		{
			CurrentRenderer->UpdatePaletteTableBuffer(index, data);
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
