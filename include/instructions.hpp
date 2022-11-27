#ifndef _INSTRUCTIONS_HPP_
#define _INSTRUCTIONS_HPP_

#include "types.hpp"
#include "bits.hpp"
#include <concepts>
#include <cstdint>
#include <array>
#include <vector>

namespace ClassicVCom_Nova64
{
	enum class InstructionType : uint16_t
	{
		NoOperation = 0x0000,
		SystemCall = 0x0001,
		InterruptReturn = 0x000E,
		Return = 0x000F,
		Push = 0x0010,
		Pop = 0x0011,
		Move = 0x0020,
		Swap = 0x0021,
		Compare = 0x0030,
		Jump = 0x0031,
		Call = 0x0032,
		Add = 0x0040,
		Subtract = 0x0041,
		Multiply = 0x0042,
		Divide = 0x0043,
		IncrementDecrement = 0x0044,
		ShiftLeftRight = 0x0045,
		RotateLeftRight = 0x0046,
		Or = 0x0047,
		SetClear = 0x00D0,
		ChipCall = 0x00F0,
		ShadowFetchAndExecute = 0x0100
	};

	struct alignas(8) BaseInstructionData
	{
		Word_LE instruction_type;
		std::array<uint8_t, 6> data;
	};

	class CPU;

	struct InstructionCallbackData
	{
		CPU &CurrentCPU;
		std::array<QWord_LE, 4> instruction_data;
		std::vector<QWord_LE> current_data;
		void (*callback)(InstructionCallbackData &data);
	};

	struct PointerControlData
	{
		uint8_t offset_type;
		uint8_t pointer_type;
		uint8_t target;
	};

	template <uint16_t offset_start_bit, uint16_t pointer_type_start_bit, uint16_t target_start_bit>
	inline PointerControlData GetPointerControlData(Word_LE &operand_control)
	{
		return { static_cast<uint8_t>((operand_control & GenerateFieldBitmask<uint16_t>(offset_start_bit, 2)) >> offset_start_bit), static_cast<uint8_t>((operand_control & GenerateFieldBitmask<uint16_t>(pointer_type_start_bit, 2)) >> pointer_type_start_bit), static_cast<uint8_t>((operand_control & GenerateFieldBitmask<uint16_t>(target_start_bit, 2)) >> target_start_bit) };
	}

	template <DWordCompatible T, std::signed_integral T2> requires (sizeof(T) == sizeof(T2))
	inline void SetConditionFlag(T operand_0, T operand_1, QWord_LE &flags, bool signed_mode)
	{
		if (!signed_mode)
		{
			if (operand_0 == operand_1)
			{
				flags |= 0x20;
			}
			else if (operand_0 < operand_1)
			{
				flags |= 0x40;
			}
			else
			{
				flags |= 0x80;
			}
		}
		else
		{
			if (static_cast<T2>(operand_0) == static_cast<T2>(operand_1))
			{
				flags |= 0x20;
			}
			else if (static_cast<T2>(operand_0) < static_cast<T2>(operand_1))
			{
				flags |= 0x40;
			}
			else
			{
				flags |= 0x80;
			}
		}
	}

	template <DWordCompatible T>
	inline void IncrementDecrementValue(T &operand, QWord_LE &flags, uint8_t inc_dec_control)
	{
		if (inc_dec_control == 0)
		{
			T tmp = operand + 1;
			flags &= ~(0x09);
			if (tmp < operand)
			{
				flags |= 0x08;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			operand = tmp;
		}
		else
		{
			T tmp = operand - 1;
			flags &= ~(0x11);
			if (tmp > operand)
			{
				flags |= 0x10;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			operand = tmp;
		}
	}

	namespace Instruction
	{
		namespace CommonExecuteCycles
		{
			void Complete_ExecuteCycle(InstructionCallbackData &data);
			template <void (*callback)(InstructionCallbackData &)>
			void Dummy_ExecuteCycle(InstructionCallbackData &data);
		}

		// inline void InvalidInstruction(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);
		// inline void NoOperation(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);
		
		namespace Interrupt
		{
			void ExecuteCycle_1(InstructionCallbackData &data);
			void ExecuteCycle_2(InstructionCallbackData &data);
			void ExecuteCycle_3(InstructionCallbackData &data);
			void ExecuteCycle_4(InstructionCallbackData &data);
			void ExecuteCycle_5(InstructionCallbackData &data);
		};

		namespace InvalidInstructionType
		{
			void ExecuteCycle(InstructionCallbackData &data);
		}

		namespace NoOperationInstruction
		{
			void ExecuteCycle(InstructionCallbackData &data);
		}

		// void SystemCall(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace SystemCallInstruction
		{
			void ExecuteCycle(InstructionCallbackData &data);
		}

		// void InterruptReturn(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace InterruptReturnInstruction
		{
			void ExecuteCycle_1(InstructionCallbackData &data);
			void ExecuteCycle_2(InstructionCallbackData &data);
			void ExecuteCycle_3(InstructionCallbackData &data);
			void ExecuteCycle_4(InstructionCallbackData &data);
			void ExecuteCycle_5(InstructionCallbackData &data);
		}
		
		// void Push(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace PushInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Data_ExecuteCycle(InstructionCallbackData &data);
			void QWord_ImmediateValue_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
		}

		// void Pop(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace PopInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T, bool is_byte = std::is_same<T, uint8_t>(), bool is_word = std::is_same<T, Word_LE>(), bool is_dword = std::is_same<T, DWord_LE>(), bool is_qword = std::is_same<T, QWord_LE>()>
			void Register_ExecuteCycle(InstructionCallbackData &data);
			void DataDiscard_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
		}

		// void Move(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace MoveInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T, QWordAlignmentRequired T2>
			void Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T, QWordAlignmentRequired T2>
			void Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Register_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void Register_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Register_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Register_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Byte_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void Byte_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Byte_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Byte_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			void Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void Word_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires DWordMinimumRequired<T> && QWordMaximumRequired<T>
			void Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			void Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			void Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			void Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			void Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <typename T> requires WordMinimumRequired<T> && DWordMaximumRequired<T>
			void Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T>
			void Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Absolute_Pointer_Self_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Absolute_Pointer_Chipset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Base_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Stack_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Absolute_Pointer_Self_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &data), void (*word_callback)(InstructionCallbackData &data), void (*dword_callback)(InstructionCallbackData &data), void (*qword_callback)(InstructionCallbackData &data)>
			void Absolute_Pointer_Chipset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Base_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <void (*byte_callback)(InstructionCallbackData &), void (*word_callback)(InstructionCallbackData &), void (*dword_callback)(InstructionCallbackData &), void (*qword_callback)(InstructionCallbackData &)>
			void Stack_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
		}

		// void Compare(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);
		
		namespace CompareInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <WordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3> 
			void Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data);
			void QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data);
		}

		// inline void Jump(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace JumpInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data);
			void Pointer_NoCrossRegion_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data);
			void Absolute_Pointer_Self_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data);
			void Pointer_Register_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data);
			void Pointer_Register_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data);
			void CrossRegion_ExecuteCycle(InstructionCallbackData &data);
		}

		// void Add(CPU &CurrentCPU, BaseInstructionData &instruction_data);
		// void Subtract(CPU &CurrentCPU, BaseInstructionData &instruction_data);
		/*
		template <void (*ArithmeticFuncByte)(uint8_t &, uint8_t &, QWord_LE &, bool, bool), void (*ArithmeticFuncWord)(Word_LE &, Word_LE &, QWord_LE &, bool, bool), void (*ArithmeticFuncDWord)(DWord_LE &, DWord_LE &, QWord_LE &, bool, bool), void (*ArithmeticFuncQWord)(QWord_LE &, QWord_LE &, QWord_LE &, bool, bool)>
		void AddSubtract(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);
		*/

		namespace AddInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			void Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
		}

		namespace SubtractInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMinimumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			void Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
			template <DWordMaximumRequired T, std::signed_integral T2, QWordAlignmentRequired T3>
			void Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data);
			void Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data);
		}

		// void IncrementDecrement(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace IncrementDecrementInstruction
		{
			void Base_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Self_ExecuteCycle_3(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Absolute_Pointer_Chipset_ExecuteCycle_3(InstructionCallbackData &data);
			template <DWordCompatible T, void (*callback)(InstructionCallbackData &)>
			void Pointer_Register_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data);
			template <DWordCompatible T>
			void Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data);
		}

		// inline void SetClear(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace SetClearInstruction
		{
			void Flags1_ExecuteCycle(InstructionCallbackData &data);
			void Flags2_ExecuteCycle(InstructionCallbackData &data);
		}

		// inline void ChipCall(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace ChipCallInstruction
		{
			void ExecuteCycle(InstructionCallbackData &data);
		}

		// inline void ShadowFetchAndExecute(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed);

		namespace ShadowFetchAndExecuteInstruction
		{
			void ExecuteCycle_1(InstructionCallbackData &data);
			void ExecuteCycle_2(InstructionCallbackData &data);
			void ExecuteCycle_3(InstructionCallbackData &data);
		}
	}
}

#endif
