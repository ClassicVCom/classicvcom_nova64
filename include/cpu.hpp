#ifndef _CPU_HPP_
#define _CPU_HPP_

#include "program.hpp"
#include "types.hpp"
#include "instructions.hpp"
#include "math.hpp"
#include "chipset.hpp"
#include <cstring>
#include <chrono>
#include <array>
#include <vector>
#include <deque>
#include <fmt/core.h>

namespace ClassicVCom_Nova64
{
	struct alignas(8) GPR_Data
	{
		QWord_LE r0, r1, r2, r3, r4, r5, r6, r7;

		QWord_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return r0; }
				case 1: { return r1; }
				case 2: { return r2; }
				case 3: { return r3; }
				case 4: { return r4; }
				case 5: { return r5; }
				case 6: { return r6; }
				case 7: { return r7; }
			}
			return r0;
		}
	};

	struct alignas(8) ByteField
	{
		uint8_t f_0, f_1, f_2, f_3, f_4, f_5, f_6, f_7;
		
		uint8_t &operator[](int index)
		{
			switch (index)
			{
				case 0: { return f_0; }
				case 1: { return f_1; }
				case 2: { return f_2; }
				case 3: { return f_3; }
				case 4: { return f_4; }
				case 5: { return f_5; }
				case 6: { return f_6; }
				case 7: { return f_7; }
			}
			return f_0;
		}
	};

	struct alignas(8) WordField
	{
		Word_LE f_0, f_1, f_2, f_3;

		Word_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return f_0; }
				case 1: { return f_1; }
				case 2: { return f_2; }
				case 3: { return f_3; }
			}
			return f_0;
		}
	};

	struct alignas(8) DWordField
	{
		DWord_LE f_0, f_1;

		DWord_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return f_0; }
				case 1: { return f_1; }
			}
			return f_0;
		}
	};

	struct alignas(8) IndexRegisterData
	{
		DWord_LE offset; // Signed
		DWord_LE unused;
	};

	struct alignas(8) MPRegisterData
	{
		DWord_LE address;
		DWord_LE memory_control;
	};

	struct alignas(8) CPUVersionData
	{
		Word_LE major;
		Word_LE minor;
		Word_LE patch;
	};

	enum class IndexRegisterType
	{
		None, Source, Destination
	};

	struct OffsetData
	{
		DWord_LE offset;
		IndexRegisterType index_register_used;
	};

	struct alignas(8) CrossRegionJumpData
	{
		DWord_LE address;
		uint8_t region;
	};

	enum class CycleState
	{
		Fetch, Execute
	};

	class Motherboard;
	class Kernel;

	class GPU;
	class Input;
	class Timer;
	class FloatingPoint;

	template <uint8_t extra_data_count>
	using ExtraQWordData = std::array<uint8_t, sizeof(uint64_t) * extra_data_count>;

	struct alignas(4) ProgramMemoryControlData
	{
		Word_LE program_id;
		uint8_t region_id;
	};

	/*
	struct InstructionFunction
	{
		void (*func)(CPU &, BaseInstructionData &, uint32_t &);
		const char *display_str;
	};

	using InstructionFunctionTableArray = std::array<InstructionFunction, 0x10000>;
	*/

	struct InstructionBaseCallback
	{
		void (*callback)(InstructionCallbackData &data);
		const char *display_str;
	};

	using InstructionBaseCallbackTableArray = std::array<InstructionBaseCallback, 0x10000>;

	inline void SetProgram(MPRegisterData &mp_register, Word_LE program_id, uint8_t region_id)
	{
		mp_register.memory_control &= ~(0xFFFF);
		mp_register.memory_control |= ((region_id << 12) | (program_id & 0xFFF));
	}

	inline void SetProgramID(MPRegisterData &mp_register, Word_LE program_id)
	{
		mp_register.memory_control &= ~(0xFFF);
		mp_register.memory_control |= (program_id & 0xFFF);
	}

	inline void SetProgramRegion(MPRegisterData &mp_register, uint8_t region_id)
	{
		mp_register.memory_control &= ~(0xF000);
		mp_register.memory_control |= (region_id << 12);
	}

	inline ProgramMemoryControlData GetProgramMemoryControlData(MPRegisterData &mp_register)
	{
		return { (mp_register.memory_control & 0xFFF), static_cast<uint8_t>((mp_register.memory_control & 0xF000) >> 12) };
	}

	// consteval InstructionFunctionTableArray CreateInstructionFunctionTable();
	consteval InstructionBaseCallbackTableArray CreateInstructionBaseCallbackTable();

	class CPU
	{
		public:
			CPU(double cycles_per_second = 1000000.0, Motherboard *CurrentMotherboard = nullptr);
			~CPU();
			void SetupProgramVectorTables();

			inline double GetCyclesPerSecond() const
			{
				return cycles_per_second;
			}
			
			inline void Start()
			{
				running = true;
				cycle_tp = std::chrono::high_resolution_clock::now();
				cycle_accumulator = 0.0;
			}
			
			inline void Stop()
			{
				running = false;
			}
			
			void RunCycles();
			
			inline bool IsRunning() const
			{
				return running;
			}

			template <uint8_t chipset, uint8_t interrupt>
			inline bool IssueInterruptRequest()
			{
				if (FL & 0x100)
				{
					irq_queue.push_back({ chipset, interrupt });
					return true;
				}
				return false;
			}
			
			void LoadTestProgram();
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
			
			template <uint8_t extra_data_count>
			inline ExtraQWordData<extra_data_count> FetchExtraData()
			{
				ExtraQWordData<extra_data_count> extra_data;
				Word_LE current_program_id = (IP.memory_control & 0xFFF);
				uint8_t current_region_id = ((IP.memory_control & 0xF000) >> 12);
				Program &CurrentProgram = ProgramVectorTable[current_program_id];
				uint8_t *current_region = CurrentProgram.memory_region[current_region_id];
				memcpy(extra_data.data(), &current_region[IP.address], extra_data.size());
				IP.address += extra_data.size();
				return extra_data;
			}

			template <QWordAlignmentRequired T>
			inline T FastFetchExtraData()
			{
				T extra_data;
				Word_LE current_program_id = (IP.memory_control & 0xFFF);
				uint8_t current_region_id = ((IP.memory_control & 0xF000) >> 12);
				Program &CurrentProgram = ProgramVectorTable[current_program_id];
				uint8_t *current_region = CurrentProgram.memory_region[current_region_id];
				memcpy(&extra_data, &current_region[IP.address], sizeof(T));
				IP.address += sizeof(T);
				return extra_data;
			}

			// friend void RunProgram(CPU &cpu, uint8_t *system_memory);
			// friend void ExitProgram(CPU &cpu, Word_LE program_id);

			friend GPU;
			friend Input;
			friend Timer;
			friend FloatingPoint;

			friend void Instruction::CommonExecuteCycles::Complete_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::Interrupt::ExecuteCycle_1(InstructionCallbackData &data);
			friend void Instruction::Interrupt::ExecuteCycle_2(InstructionCallbackData &data);
			friend void Instruction::Interrupt::ExecuteCycle_3(InstructionCallbackData &data);
			friend void Instruction::Interrupt::ExecuteCycle_4(InstructionCallbackData &data);
			friend void Instruction::Interrupt::ExecuteCycle_5(InstructionCallbackData &data);

			friend void Instruction::InvalidInstructionType::ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::NoOperationInstruction::ExecuteCycle(InstructionCallbackData &data);
			
			friend void Instruction::SystemCallInstruction::ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::InterruptReturnInstruction::ExecuteCycle_1(InstructionCallbackData &data);
			friend void Instruction::InterruptReturnInstruction::ExecuteCycle_2(InstructionCallbackData &data);
			friend void Instruction::InterruptReturnInstruction::ExecuteCycle_3(InstructionCallbackData &data);
			friend void Instruction::InterruptReturnInstruction::ExecuteCycle_4(InstructionCallbackData &data);
			friend void Instruction::InterruptReturnInstruction::ExecuteCycle_5(InstructionCallbackData &data);
			

			friend void Instruction::PushInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PushInstruction::Data_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::PushInstruction::QWord_ImmediateValue_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PushInstruction::Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PushInstruction::Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::PopInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T, bool is_byte, bool is_word, bool is_dword, bool is_qword>
			friend void Instruction::PopInstruction::Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::PopInstruction::DataDiscard_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::PopInstruction::Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::MoveInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T, QWordAlignmentRequired T2, uint8_t operand_0_register, uint8_t operand_0_field_index>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T, QWordAlignmentRequired T2>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Register_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Register_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Register_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Register_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Byte_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Byte_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Byte_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Byte_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::QWord_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Word_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires DWordMinimumRequired<T> && QWordMaximumRequired<T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::QWord_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::QWord_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			friend void Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Self_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::CompareInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::CompareInstruction::Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::CompareInstruction::Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::Base_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::CompareInstruction::Stack_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::JumpInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Pointer_NoCrossRegion_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Absolute_Pointer_Self_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Absolute_Pointer_Self_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Pointer_Register_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::Pointer_Register_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data);
			friend void Instruction::JumpInstruction::CrossRegion_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::AddInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::AddInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::AddInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::SubtractInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			friend void Instruction::SubtractInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SubtractInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::IncrementDecrementInstruction::Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_3(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_3(InstructionCallbackData &data);
			template <DWordCompatible T, void (*callback)(InstructionCallbackData &)>
			friend void Instruction::IncrementDecrementInstruction::Pointer_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			friend void Instruction::IncrementDecrementInstruction::Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			
			friend void Instruction::SetClearInstruction::Flags1_ExecuteCycle(InstructionCallbackData &data);
			friend void Instruction::SetClearInstruction::Flags2_ExecuteCycle(InstructionCallbackData &data);

			friend void Instruction::CPUIDInstruction::ExecuteCycle(InstructionCallbackData &data);			

			friend void Instruction::ChipCallInstruction::ExecuteCycle(InstructionCallbackData &data);

			// friend inline void Instruction::ShadowFetchAndExecute(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

			/*
			friend void Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_1(InstructionCallbackData &data);
			friend void Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_2(InstructionCallbackData &data);
			friend void Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_3(InstructionCallbackData &data);
			*/
			
			friend Kernel;
		private:
			// GPR_Data GPR_Registers;
			std::array<QWord_LE, 8> GPR_Registers;
			IndexRegisterData SI;
			IndexRegisterData DI;
			QWord_LE FL;
			MPRegisterData IP;
			MPRegisterData BP;
			MPRegisterData SP;
			MPRegisterData SOP;
			MPRegisterData DOP;
			QWord_LE SR;
			QWord_LE data_bus;
			double cycles_per_second;
			double cycle_rate;
			CycleState current_cycle_state;
			InstructionCallbackData current_instruction_callback;
			std::chrono::high_resolution_clock::time_point cycle_tp;
			double cycle_accumulator;
			Word_LE current_program_id;
			bool running;
			BaseInstructionData CurrentInstruction;
			Motherboard *CurrentMotherboard;
			std::vector<Program> ProgramVectorTable;
			std::deque<ChipsetInterruptRequestData> irq_queue;
			const CPUVersionData version = { 0, 1, 0 };
			// const InstructionFunctionTableArray instruction_function_table;
			const InstructionBaseCallbackTableArray instruction_base_callback_table;
	};
}

#endif
