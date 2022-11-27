#ifndef _PROGRAM_HPP_
#define _PROGRAM_HPP_

#include "types.hpp"
#include <cstdint>
#include <array>

namespace ClassicVCom_Nova64
{
	struct alignas(16) ProgramDescriptor
	{
		DWord_LE start_addr;
		DWord_LE program_executable_mem_size;
		DWord_LE program_control;
		DWord_LE program_data;
	};

	struct alignas(16) MemoryRegionTableEntryData
	{
		DWord_LE memory_region_start_addr;
		DWord_LE memory_region_size;
		struct
		{
			uint8_t memory_region_flags;
			std::array<uint8_t, 3> unused;
		} memory_region_control;
		DWord_LE reserved;
	};

	struct OffsetData;

	class CPU;
	class Kernel;

	struct Program
	{
		ProgramDescriptor *descriptor;
		uint8_t *program_executable_memory;
		std::array<uint8_t *, 16> memory_region;
	};

	/*
	void RunProgram(CPU &cpu, uint8_t *system_memory);
	void ExitProgram(CPU &cpu, Word_LE program_id);
	*/
}

#endif
