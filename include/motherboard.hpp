#ifndef _MOTHERBOARD_HPP_
#define _MOTHERBOARD_HPP_

#include "renderer.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "network.hpp"
#include "floating_point.hpp"
#include "dma.hpp"
#include "os.hpp"
#include <cstdint>
#include <array>
#include <vector>

namespace ClassicVCom_Nova64
{
	class Motherboard
	{
		public:
			Motherboard(uint64_t ram);
			~Motherboard();

			inline void SetupGPU(Renderer *renderer)
			{
				MainGPU.LinkWithRenderer(renderer);
				MainGPU.CopyPaletteTableToRenderer(0);
			}
			
			inline CPU *GetCPU()
			{
				return &MainCPU;
			}

			inline GPU *GetGPU()
			{
				return &MainGPU;
			}
			
			inline Input *GetInput()
			{
				return &MainInput;
			}

			inline Timer *GetTimer()
			{
				return &MainTimer;
			}
			
			inline FloatingPoint *GetFloatingPoint()
			{
				return &MainFloatingPoint;
			}
			
			template <QWordCompatible T>
			inline T ChipsetReadFromMemoryGroup(uint8_t chipset, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T data = 0;
				switch (chipset)
				{
					case 0x00:
					{
						data = MainGPU.ReadFromMemoryGroup<T>(MainCPU, group, address, offset_data, read_return);
						break;
					}
					case 0x03:
					{
						data = MainInput.ReadFromMemoryGroup<T>(MainCPU, group, address, offset_data, read_return);
						break;
					}
					case 0x04:
					{
						data = MainTimer.ReadFromMemoryGroup<T>(MainCPU, group, address, offset_data, read_return);
						break;
					}
					case 0x07:
					{
						data = MainFloatingPoint.ReadFromMemoryGroup<T>(MainCPU, group, address, offset_data, read_return);
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return data;
			}
			
			template <QWordCompatible T>
			inline void ChipsetWriteToMemoryGroup(uint8_t chipset, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
			{
				switch (chipset)
				{
					case 0x00:
					{
						MainGPU.WriteToMemoryGroup<T>(MainCPU, group, address, offset_data, data, write_return);
						break;
					}
					case 0x03:
					{
						MainInput.WriteToMemoryGroup<T>(MainCPU, group, address, offset_data, data, write_return);
						break;
					}
					case 0x04:
					{
						MainTimer.WriteToMemoryGroup<T>(MainCPU, group, address, offset_data, data, write_return);
						break;
					}
					case 0x07:
					{
						MainFloatingPoint.WriteToMemoryGroup<T>(MainCPU, group, address, offset_data, data, write_return);
						break;
					}
					default:
					{
						write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
						break;
					}
				}
			}

			inline void ChipsetEndOfInterrupt(uint8_t chipset, uint8_t interrupt)
			{
				switch (chipset)
				{
					case 0x03:
					{
						MainInput.EndOfInterrupt(interrupt);
						break;
					}
					case 0x04:
					{
						MainTimer.EndOfInterrupt(interrupt);
						break;
					}
				}
			}

			template <QWordCompatible T>
			friend void PushDataToStack(CPU &cpu, T data);
			template <QWordCompatible T>
			friend T PopDataFromStack(CPU &cpu);
			template <QWordCompatible T>
			friend T LoadDataFromSystemMemory(CPU &cpu, Word_LE program_id, uint8_t region_id, DWord_LE address, OffsetData offset_data);
			template <QWordCompatible T>
			friend void StoreDataToSystemMemory(CPU &cpu, Word_LE program_id, uint8_t region_id, DWord_LE address, OffsetData offset_data, T data);
			template <uint8_t region_flag_bitmask>
			friend bool HasRegionFlagSupport(CPU &cpu, Word_LE program_id, uint8_t region_id);
			// friend void RunProgram(CPU &cpu, uint8_t *system_memory);
			// friend void ExitProgram(CPU &cpu, Word_LE program_id);
			friend class CPU;
			friend class Kernel;
		private:
			CPU MainCPU;
			GPU MainGPU;
			Audio MainAudio;
			Input MainInput;
			Timer MainTimer;
			Network MainNetwork;
			FloatingPoint MainFloatingPoint;
			DMA MainDMA;
			Kernel OSKernel;
			uint64_t total_system_ram;
			uint64_t available_system_ram;
			std::vector<ProgramDescriptor> program_descriptor_table;
			std::vector<MemoryRegionTableEntryData> memory_region_table;
			std::vector<uint8_t> memory;
	};
}

#endif
