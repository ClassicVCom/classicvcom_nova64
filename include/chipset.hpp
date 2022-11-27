#ifndef _CHIPSET_HPP_
#define _CHIPSET_HPP_

#include "types.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <concepts>

namespace ClassicVCom_Nova64
{
	template <typename T>
	concept HasChipsetId = requires(T obj)
	{
		obj.id;
	};

	enum class ChipsetReturnCode
	{
		Ok,
		MemoryGroupReadSuccessful,
		MemoryGroupReadFailed,
		MemoryGroupWriteSuccessful,
		MemoryGroupWriteFailed
	};

	struct ChipsetInterruptDescriptorTableEntryData
	{
		struct alignas(4)
		{
			Word_LE interrupt_flags;
			uint8_t region;
			uint8_t reserved;
		} ISR_control; // Interrupt Subroutine Control
		DWord_LE address; // Memory Region Local Memory Address
	};

	struct ChipsetInterruptRequestData
	{
		uint8_t chipset;
		uint8_t interrupt;
	};

	template <typename T>
	concept HasInterruptDescriptorTable = requires(T obj)
	{
		{ obj.interrupt_descriptor_table[0] } -> std::convertible_to<ChipsetInterruptDescriptorTableEntryData>;
	};

	/*
	template <typename T, unsigned char idt_size>
	class Chipset
	{
		public:
			Chipset() : id("")
			{
			}
			~Chipset()
			{
			}
			void SetInterrupt(unsigned char interrupt, unsigned long long isr_addr)
			{
				if (interrupt < idt_size)
				{
					interrupt_descriptor_table[interrupt] = isr_addr;
				}
			}
			unsigned long long GetInterrupt(unsigned char interrupt) const
			{
				return (interrupt < idt_size) ? interrupt_descriptor_table[interrupt] : 0;
			}
			std::string GetId()
			{
				return id;
			}
			template <unsigned short method, typename ret_type, typename ...Args>
			ret_type CallMethod(Args...)
			{
				return ret_type();
			}
			void operator()(unsigned short function, unsigned long long args[8])
			{
			}
			template <HasInterruptDescriptorTable T2>
			friend void SetInterrupt(T2 &chipset, unsigned char interrupt, unsigned long long isr_addr);
			template <HasInterruptDescriptorTable T2>
			friend constexpr unsigned long long GetInterrupt(T2 &chipset, unsigned char interrupt);
		private:
			std::array<unsigned long long, idt_size> interrupt_descriptor_table;
			std::string id;
			T ChipsetData;
	};
	*/


	template <HasInterruptDescriptorTable T>
	void SetInterrupt(T &chipset, uint8_t interrupt, uint64_t isr_addr)
	{
		if (interrupt < chipset.interrupt_descriptor_table.size())
		{
			chipset.interrupt_descriptor_table[interrupt] = isr_addr;
		}
	}

	template <HasInterruptDescriptorTable T>
	inline ChipsetInterruptDescriptorTableEntryData GetInterrupt(T &chipset, uint8_t interrupt)
	{
		return (interrupt < chipset.interrupt_descriptor_table.size()) ? chipset.interrupt_descriptor_table[interrupt] : ChipsetInterruptDescriptorTableEntryData{ { 0, 0, 0 }, 0 };
	}

	template <HasChipsetId T>
	std::string GetId(T &chipset)
	{
		return chipset.id;
	}
}
#endif
