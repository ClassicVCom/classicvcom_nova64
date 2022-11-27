#include "os.hpp"
#include "motherboard.hpp"
#include "cpu.hpp"
#include "program.hpp"
#include "cvne.hpp"
#include <fmt/core.h>
#include <cstring>

ClassicVCom_Nova64::Kernel::Kernel(Motherboard &motherboard) : CurrentMotherboard(motherboard)
{
}

ClassicVCom_Nova64::Kernel::~Kernel()
{
}

bool ClassicVCom_Nova64::Kernel::CreateProcess()
{
	CPU &CurrentCPU = CurrentMotherboard.MainCPU;
	uint16_t program_id = 0;
	for (auto &i : CurrentCPU.ProgramVectorTable)
	{
		if (i.descriptor->program_control & 0x01)
		{
			++program_id;
			continue;
		}
		else
		{
			i.descriptor->program_control |= 0x01;
			i.program_executable_memory = &CurrentMotherboard.memory[i.descriptor->start_addr];
			MemoryRegionTableEntryData *program_memory_region_table = &CurrentMotherboard.memory_region_table[program_id * 16];
			for (uint8_t i2 = 0; i2 < 16; ++i2)
			{
				i.memory_region[i2] = &i.program_executable_memory[program_memory_region_table[i2].memory_region_start_addr];
			}
			Process new_process;
			new_process.name = "";
			new_process.program = &i;
			new_process.state.IP = { 0x00000000, program_id };
			new_process.state.BP = { 0x00000000, (15 << 12) | program_id };
			new_process.state.SP = { 0x00000000, (15 << 12) | program_id };
			ProcessList.push_back(new_process);
			CurrentMotherboard.available_system_ram -= i.descriptor->program_executable_mem_size;
			if (ProcessList.size() == 1)
			{
				CurrentCPU.Start();
			}
			return true;
		}
	}
	return false;
}

void ClassicVCom_Nova64::Kernel::DestroyProcess(Word_LE program_id, QWord_LE return_code)
{
	CPU &CurrentCPU = CurrentMotherboard.MainCPU;
	Program &CurrentProgram = CurrentCPU.ProgramVectorTable[program_id];
	if (CurrentProgram.descriptor->program_control & 0x01)
	{
		if (CurrentProgram.descriptor->program_control & 0x02)
		{
			struct ChildProgramData
			{
				Word_LE parent_program_id;
				Word_LE reserved;
			} child_program_data = reinterpret_cast<ChildProgramData &>(CurrentProgram.descriptor->program_data);
			size_t parent_process = 0;
			for (auto &i : ProcessList)
			{
				if (i.program == &CurrentCPU.ProgramVectorTable[child_program_data.parent_program_id])
				{
					i.state.GPR_Registers[0] = return_code;
					break;
				}
			}
		}
		CurrentProgram.descriptor->program_control &= ~(0x01);
		CurrentProgram.program_executable_memory = nullptr;
		for (size_t i = 0; i < CurrentProgram.memory_region.size(); ++i)
		{
			CurrentProgram.memory_region[i] = nullptr;
		}
		CurrentMotherboard.available_system_ram += CurrentProgram.descriptor->program_executable_mem_size;
		size_t current_process = 0;
		for (auto &i : ProcessList)
		{
			if (i.program == &CurrentProgram)
			{
				ProcessList.erase(ProcessList.begin() + current_process);
				fmt::print("Process Destroyed.\n");
				if (ProcessList.size() == 0)
				{
					CurrentCPU.Stop();
				}
				break;
			}
			++current_process;
		}
	}
}

void ClassicVCom_Nova64::Kernel::LoadProcessState(Word_LE program_id)
{
	CPU &CurrentCPU = CurrentMotherboard.MainCPU;
	Program &CurrentProgram = CurrentCPU.ProgramVectorTable[program_id];
	if (CurrentProgram.descriptor->program_control & 0x01)
	{
		for (auto &i : ProcessList)
		{
			if (i.program == &CurrentProgram)
			{
				CurrentCPU.GPR_Registers = i.state.GPR_Registers;
				CurrentCPU.SI = i.state.SI;
				CurrentCPU.DI = i.state.DI;
				CurrentCPU.FL = i.state.FL;
				CurrentCPU.IP = i.state.IP;
				CurrentCPU.BP = i.state.BP;
				CurrentCPU.SP = i.state.SP;
				break;
			}
		}
	}
}

void ClassicVCom_Nova64::Kernel::SaveProcessState(Word_LE program_id)
{
	CPU &CurrentCPU = CurrentMotherboard.MainCPU;
	Program &CurrentProgram = CurrentCPU.ProgramVectorTable[program_id];
	if (CurrentProgram.descriptor->program_control & 0x01)
	{
		for (auto &i : ProcessList)
		{
			if (i.program == &CurrentProgram)
			{
				i.state.GPR_Registers = CurrentCPU.GPR_Registers;
				i.state.SI = CurrentCPU.SI;
				i.state.DI = CurrentCPU.DI;
				i.state.FL = CurrentCPU.FL;
				i.state.IP = CurrentCPU.IP;
				i.state.BP = CurrentCPU.BP;
				i.state.SP = CurrentCPU.SP;
				break;
			}
		}
	}
}
