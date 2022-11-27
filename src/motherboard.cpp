#include "motherboard.hpp"
#include <cstring>

ClassicVCom_Nova64::Motherboard::Motherboard(uint64_t ram) : MainCPU(30000000.0, this), OSKernel(*this), program_descriptor_table(4096, { 0, 0, 0, 0 }), memory_region_table(4096 * 16)
{
	if (ram < (1 << 20))
	{
		total_system_ram = (1 << 20);
	}
	else
	{
		total_system_ram = ram;
	}
	available_system_ram = total_system_ram;
	memory.resize(total_system_ram);
	memset(memory.data(), 0, total_system_ram);
	MainCPU.SetupProgramVectorTables();
	for (uint8_t i = 0; i < 8; ++i)
	{
		MainTimer.SetupTimerSync(i, MainCPU.GetCyclesPerSecond());
	}
}

ClassicVCom_Nova64::Motherboard::~Motherboard()
{
}
