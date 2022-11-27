#ifndef _SYSTEM_HPP_
#define _SYSTEM_HPP_

#include "types.hpp"
#include "motherboard.hpp"
#include "cpu.hpp"
#include <cstring>

namespace ClassicVCom_Nova64
{
	template <QWordCompatible T>
	inline void PushDataToStack(CPU &cpu, T data)
	{
		ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(cpu.SP);
		Program &CurrentProgram = cpu.ProgramVectorTable[current_program_memory_control.program_id];
		if (!(CurrentProgram.descriptor->program_control & 0x01))
		{
			return;
		}
		uint8_t *current_region = CurrentProgram.memory_region[current_program_memory_control.region_id];
		size_t system_region_id = (current_program_memory_control.program_id * 16) + current_program_memory_control.region_id;
		if (!(cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x02))
		{
			return;
		}
		DWord_LE tmp = cpu.SP.address - sizeof(T);
		if (tmp > cpu.SP.address)
		{
			cpu.SP.address = tmp % cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size;
			size_t align_check = cpu.SP.address % sizeof(T);
			if (align_check != 0)
			{
				memcpy(&current_region[cpu.SP.address], &data, sizeof(T) - align_check);
				data >>= ((sizeof(T) - align_check) * 8);
				memcpy(&current_region[0], &data, align_check);
			}
			else
			{
				memcpy(&current_region[cpu.SP.address], &data, sizeof(T));
			}
		}
		else
		{
			cpu.SP.address = tmp;
			memcpy(&current_region[cpu.SP.address], &data, sizeof(T));
		}
	}

	template <QWordCompatible T>
	inline T PopDataFromStack(CPU &cpu)
	{
		T data = 0;
		ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(cpu.SP);
		Program &CurrentProgram = cpu.ProgramVectorTable[current_program_memory_control.program_id];
		if (!(CurrentProgram.descriptor->program_control & 0x01))
		{
			return 0;
		}
		uint8_t *current_region = CurrentProgram.memory_region[current_program_memory_control.region_id];
		size_t system_region_id = (current_program_memory_control.program_id * 16) + current_program_memory_control.region_id;
		if (!(cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x01))
		{
			return 0;
		}
		DWord_LE tmp = (cpu.SP.address + sizeof(T)) % cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size;
		if (tmp < cpu.SP.address)
		{
			size_t align_check = cpu.SP.address % sizeof(T);
			if (align_check != 0)
			{
				T data_2 = 0;
				memcpy(&data, &current_region[cpu.SP.address], sizeof(T) - align_check);
				memcpy(&data_2, &current_region[0], align_check);
				data |= (data_2 << ((sizeof(T) - align_check) * 8));
			}
			else
			{
				memcpy(&data, &current_region[cpu.SP.address], sizeof(T));
			}
		}
		else
		{
			memcpy(&data, &current_region[cpu.SP.address], sizeof(T));
		}
		cpu.SP.address = tmp;
		return data;
	}

	template <QWordCompatible T>
	inline T LoadDataFromSystemMemory(CPU &cpu, Word_LE program_id, uint8_t region_id, DWord_LE address, OffsetData offset_data)
	{
		T data = 0;
		Program &CurrentProgram = cpu.ProgramVectorTable[program_id];
		if (!(CurrentProgram.descriptor->program_control & 0x01))
		{
			return 0;
		}
		uint8_t *current_region = CurrentProgram.memory_region[region_id];
		size_t system_region_id = (program_id * 16) + region_id;
		if (!(cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x01))
		{
			return 0;
		}
		DWord_LE current_address = address + static_cast<int>(offset_data.offset);
		if (cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x20)
		{
			current_address %= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size;
			if ((current_address + (sizeof(T) - 1)) >= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size)
			{
				size_t align_check = current_address % sizeof(T);
				if (align_check != 0)
				{
					T data_2 = 0;
					memcpy(&data, &current_region[current_address], sizeof(T) - align_check);
					memcpy(&data_2, &current_region[0], align_check);
					data |= (data_2 << ((sizeof(T) - align_check) * 8));
				}
				else
				{
					memcpy(&data, &current_region[current_address], sizeof(T));
				}
			}
		}
		else
		{
			if ((current_address + (sizeof(T) - 1)) >= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size)
			{
				return 0;
			}
			memcpy(&data, &current_region[current_address], sizeof(T));
		}
		if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
		{
			IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
			target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
		}
		return data;
	}

	template <QWordCompatible T>
	inline void StoreDataToSystemMemory(CPU &cpu, Word_LE program_id, uint8_t region_id, DWord_LE address, OffsetData offset_data, T data)
	{
		Program &CurrentProgram = cpu.ProgramVectorTable[program_id];
		if (!(CurrentProgram.descriptor->program_control & 0x01))
		{
			return;
		}
		uint8_t *current_region = CurrentProgram.memory_region[region_id];
		size_t system_region_id = (program_id * 16) + region_id;
		if (!(cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x02))
		{
			return;
		}
		DWord_LE current_address = address + static_cast<int>(offset_data.offset);
		if (cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x20)
		{
			current_address %= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size;
			if ((current_address + (sizeof(T) - 1)) >= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size)
			{
				size_t align_check = current_address % sizeof(T);
				if (align_check != 0)
				{
					memcpy(&current_region[current_address], &data, sizeof(T) - align_check);
					data >>= ((sizeof(T) - align_check) * 8);
					memcpy(&current_region[0], &data, align_check);
				}
				else
				{
					memcpy(&current_region[current_address], &data, sizeof(T));
				}
			}
		}
		else
		{
			if ((current_address + (sizeof(T) - 1)) >= cpu.CurrentMotherboard->memory_region_table[system_region_id].memory_region_size)
			{
				return;
			}
			memcpy(&current_region[current_address], &data, sizeof(T));
		}
		if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
		{
			IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
			target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
		}
	}

	template <uint8_t region_flag_bitmask>
	inline bool HasRegionFlagSupport(CPU &cpu, Word_LE program_id, uint8_t region_id)
	{
		Program &CurrentProgram = cpu.ProgramVectorTable[program_id];
		if (!(CurrentProgram.descriptor->program_control & 0x01))
		{
			return false;
		}
		return ((cpu.CurrentMotherboard->memory_region_table[(program_id * 16) + region_id].memory_region_control.memory_region_flags & region_flag_bitmask) == region_flag_bitmask);
	}
}

#endif
