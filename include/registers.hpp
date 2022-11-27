#ifndef _REGISTERS_HPP_
#define _REGISTERS_HPP_

#include "cpu.hpp"
#include "instructions.hpp"

namespace ClassicVCom_Nova64
{
	inline void GPRegisterToGPRegisterTransfer(QWord_LE &gp_reg0, QWord_LE &gp_reg1, uint8_t data_size, uint8_t gp_reg0_field_index, uint8_t gp_reg1_field_index)
	{
		switch (data_size)
		{
			case 0:
			{
				ByteField &gp_register_0_field = reinterpret_cast<ByteField &>(gp_reg0);
				ByteField &gp_register_1_field = reinterpret_cast<ByteField &>(gp_reg1);
				gp_register_0_field[gp_reg0_field_index] = gp_register_1_field[gp_reg1_field_index];
				break;
			}
			case 1:
			{
				WordField &gp_register_0_field = reinterpret_cast<WordField &>(gp_reg0);
				WordField &gp_register_1_field = reinterpret_cast<WordField &>(gp_reg1);
				gp_register_0_field[gp_reg0_field_index] = gp_register_1_field[gp_reg1_field_index];
				break;
			}
			case 2:
			{
				DWordField &gp_register_0_field = reinterpret_cast<DWordField &>(gp_reg0);
				DWordField &gp_register_1_field = reinterpret_cast<DWordField &>(gp_reg1);
				gp_register_0_field[gp_reg0_field_index] = gp_register_1_field[gp_reg1_field_index];
				break;
			}
			case 3:
			{
				gp_reg0 = gp_reg1;
				break;
			}
		}
	}

	inline void IndexRegisterToGPRegisterTransfer(QWord_LE &gp_reg, IndexRegisterData &idx_reg, uint8_t data_size, uint8_t gp_reg_field_index, uint8_t idx_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				DWordField &idx_register_field = reinterpret_cast<DWordField &>(idx_reg);
				gp_register_field[gp_reg_field_index] = idx_register_field[idx_reg_field_index];
				break;
			}
			case 3:
			{
				gp_reg = std::bit_cast<QWord_LE>(idx_reg);
				break;
			}
		}
	}

	inline void GPRegisterToIndexRegisterTransfer(IndexRegisterData &idx_reg, QWord_LE &gp_reg, uint8_t data_size, uint8_t idx_reg_field_index, uint8_t gp_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &idx_register_field = reinterpret_cast<DWordField &>(idx_reg);
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				idx_register_field[idx_reg_field_index] = gp_register_field[gp_reg_field_index];
				break;
			}
			case 3:
			{
				idx_reg = std::bit_cast<IndexRegisterData>(gp_reg);
				break;
			}
		}
	}

	inline void MPRegisterToGPRegisterTransfer(QWord_LE &gp_reg, MPRegisterData &mp_reg, uint8_t data_size, uint8_t gp_reg_field_index, uint8_t mp_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				DWordField &mp_register_field = reinterpret_cast<DWordField &>(mp_reg);
				gp_register_field[gp_reg_field_index] = mp_register_field[mp_reg_field_index];
				break;
			}
			case 3:
			{
				gp_reg = std::bit_cast<QWord_LE>(mp_reg);
				break;
			}
		}
	}

	inline void GPRegisterToMPRegisterTransfer(MPRegisterData &mp_reg, QWord_LE &gp_reg, uint8_t data_size, uint8_t mp_reg_field_index, uint8_t gp_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &mp_register_field = reinterpret_cast<DWordField &>(mp_reg);
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				mp_register_field[mp_reg_field_index] = gp_register_field[gp_reg_field_index];
				break;
			}
			case 3:
			{
				mp_reg = std::bit_cast<MPRegisterData>(gp_reg);
				break;
			}
		}
	}

	inline void IndexRegisterToIndexRegisterTransfer(IndexRegisterData &idx_reg0, IndexRegisterData &idx_reg1, uint8_t data_size, uint8_t idx_reg0_field_index, uint8_t idx_reg1_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &idx_register_0_field = reinterpret_cast<DWordField &>(idx_reg0);
				DWordField &idx_register_1_field = reinterpret_cast<DWordField &>(idx_reg1);
				idx_register_0_field[idx_reg0_field_index] = idx_register_1_field[idx_reg1_field_index];
				break;
			}
			case 3:
			{
				idx_reg0 = idx_reg1;
				break;
			}
		}
	}

	inline void MPRegisterToIndexRegisterTransfer(IndexRegisterData &idx_reg, MPRegisterData &mp_reg, uint8_t data_size, uint8_t idx_reg_field_index, uint8_t mp_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &idx_register_field = reinterpret_cast<DWordField &>(idx_reg);
				DWordField &mp_register_field = reinterpret_cast<DWordField &>(mp_reg);
				idx_register_field[idx_reg_field_index] = mp_register_field[mp_reg_field_index];
				break;
			}
			case 3:
			{
				idx_reg = std::bit_cast<IndexRegisterData>(mp_reg);
				break;
			}
		}
	}

	inline void MPRegisterToMPRegisterTransfer(MPRegisterData &mp_reg0, MPRegisterData &mp_reg1, uint8_t data_size, uint8_t mp_reg0_field_index, uint8_t mp_reg1_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &mp_register_0_field = reinterpret_cast<DWordField &>(mp_reg0);
				DWordField &mp_register_1_field = reinterpret_cast<DWordField &>(mp_reg1);
				mp_register_0_field[mp_reg0_field_index] = mp_register_1_field[mp_reg1_field_index];
				break;
			}
			case 3:
			{
				mp_reg0 = mp_reg1;
				break;
			}
		}
	}

	inline void IndexRegisterToMPRegisterTransfer(MPRegisterData &mp_reg, IndexRegisterData &idx_reg, uint8_t data_size, uint8_t mp_reg_field_index, uint8_t idx_reg_field_index)
	{
		switch (data_size)
		{
			case 2:
			{
				DWordField &mp_register_field = reinterpret_cast<DWordField &>(mp_reg);
				DWordField &idx_register_field = reinterpret_cast<DWordField &>(idx_reg);
				mp_register_field[mp_reg_field_index] = idx_register_field[idx_reg_field_index];
				break;
			}
			case 3:
			{
				mp_reg = std::bit_cast<MPRegisterData>(idx_reg);
				break;
			}
		}
	}

	inline void ImmediateValueToGPRegisterTransfer(CPU &CurrentCPU, QWord_LE &gp_reg, BaseInstructionData &instruction_data, uint8_t data_size, uint8_t gp_reg_field_index)
	{
		switch (data_size)
		{
			case 0:
			{
				ByteField &gp_register_field = reinterpret_cast<ByteField &>(gp_reg);
				gp_register_field[gp_reg_field_index] = instruction_data.data[5];
				break;
			}
			case 1:
			{
				ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
				WordField &gp_register_field = reinterpret_cast<WordField &>(gp_reg);
				Word_LE &immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
				gp_register_field[gp_reg_field_index] = immediate_value;
				break;
			}
			case 2:
			{
				ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
				gp_register_field[gp_reg_field_index] = immediate_value;
				break;
			}
			case 3:
			{
				ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
				gp_reg = std::bit_cast<QWord_LE>(extra_data);
				break;
			}
		}
	}

	inline void SystemMemoryToGPRegisterTransfer(CPU &CurrentCPU, QWord_LE &gp_reg, Word_LE program_id, uint8_t region_id, DWord_LE source_pointer, OffsetData offset_data, uint8_t data_size, uint8_t gp_reg_field_index)
	{
		switch (data_size)
		{
			case 0:
			{
				ByteField &gp_register_field = reinterpret_cast<ByteField &>(gp_reg);
				gp_register_field[gp_reg_field_index] = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, program_id, region_id, source_pointer, offset_data);
				break;
			}
			case 1:
			{
				WordField &gp_register_field = reinterpret_cast<WordField &>(gp_reg);
				gp_register_field[gp_reg_field_index] = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, program_id, region_id, source_pointer, offset_data);
				break;
			}
			case 2:
			{
				DWordField &gp_register_field = reinterpret_cast<DWordField &>(gp_reg);
				gp_register_field[gp_reg_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, program_id, region_id, source_pointer, offset_data);
				break;
			}
			case 3:
			{
				gp_reg = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, program_id, region_id, source_pointer, offset_data);
				break;
			}
		}
	}
}

#endif
