#ifndef _INSTRUCTIONS_INLINE_HPP_
#define _INSTRUCTIONS_INLINE_HPP_

#include "instructions.hpp"
#include "cpu.hpp"
#include "types.hpp"
#include "system.hpp"

namespace ClassicVCom_Nova64
{
	namespace Instruction
	{
		inline void InvalidInstruction(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			++cycles_processed;
		}

		inline void NoOperation(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			++cycles_processed;
		}

		inline void Jump(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			Word_LE &operand_control = reinterpret_cast<Word_LE &>(instruction_data.data[0]);
			uint8_t operand_type = (operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
			uint8_t jump_type = ((operand_control & GenerateFieldBitmask<uint16_t>(2, 5)) >> 2);
			bool cross_region_jump = ((operand_control & GenerateFieldBitmask<uint16_t>(7, 1)) >> 7);
			bool perform_jump = true;
			switch (jump_type)
			{
				case 1:
				{
					perform_jump = (CurrentCPU.FL & 0x01);
					break;
				}
				case 2:
				{
					perform_jump = !(CurrentCPU.FL & 0x01);
					break;
				}
				case 3:
				{
					perform_jump = (CurrentCPU.FL & 0x02);
					break;
				}
				case 4:
				{
					perform_jump = !(CurrentCPU.FL & 0x02);
					break;
				}
				case 5:
				{
					perform_jump = (CurrentCPU.FL & 0x04);
					break;
				}
				case 6:
				{
					perform_jump = !(CurrentCPU.FL & 0x04);
					break;
				}
				case 7:
				{
					perform_jump = (CurrentCPU.FL & 0x08);
					break;
				}
				case 8:
				{
					perform_jump = !(CurrentCPU.FL & 0x08);
					break;
				}
				case 9:
				{
					perform_jump = (CurrentCPU.FL & 0x10);
					break;
				}
				case 10:
				{
					perform_jump = !(CurrentCPU.FL & 0x10);
					break;
				}
				case 11:
				{
					perform_jump = (CurrentCPU.FL & 0x20);
					break;
				}
				case 12:
				{
					perform_jump = !(CurrentCPU.FL & 0x20);
					break;
				}
				case 13:
				{
					perform_jump = (CurrentCPU.FL & 0x40);
					break;
				}
				case 14:
				{
					perform_jump = (CurrentCPU.FL & 0x60);
					break;
				}
				case 15:
				{
					perform_jump = (CurrentCPU.FL & 0x80);
					break;
				}
				case 16:
				{
					perform_jump = (CurrentCPU.FL & 0xA0);
					break;
				}
			}
			switch (operand_type)
			{
				case 0:
				{
					if (!perform_jump)
					{
						++cycles_processed;
						break;
					}
					uint8_t source_register = instruction_data.data[2];
					switch (source_register)
					{
						case 0x00:
						case 0x01:
						case 0x02:
						case 0x03:
						case 0x04:
						case 0x05:
						case 0x06:
						case 0x07:
						{
							DWordField &operand_register_dword_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[source_register]);
							if (!cross_region_jump)
							{
								CurrentCPU.IP.address = operand_register_dword_field[0];
								CurrentCPU.SR |= 0x01;
								++cycles_processed;
							}
							else
							{
								Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
								ByteField &operand_register_byte_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[source_register]);
								uint8_t new_region_id = (operand_register_byte_field[4] & 0xF);
								if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
								{
									++cycles_processed;
									break;
								}
								CurrentCPU.IP.address = operand_register_dword_field[0];
								SetProgramRegion(CurrentCPU.IP, new_region_id);
								CurrentCPU.SR |= 0x03;
								cycles_processed += 2;
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					if (!perform_jump)
					{
						++cycles_processed;
						break;
					}
					DWord_LE &operand_immediate_value = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
					if (!cross_region_jump)
					{
						CurrentCPU.IP.address = operand_immediate_value;
						CurrentCPU.SR |= 0x01;
						++cycles_processed;
					}
					else
					{
						Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
						uint8_t region_id = ((operand_control & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
						if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, region_id))
						{
							++cycles_processed;
							break;
						}
						CurrentCPU.IP.address = operand_immediate_value;
						SetProgramRegion(CurrentCPU.IP, region_id);
						CurrentCPU.SR |= 0x03;
						cycles_processed += 2;
					}
					break;
				}
				case 2:
				{
					uint8_t offset_type = ((operand_control & GenerateFieldBitmask<uint16_t>(8, 2)) >> 8);
					uint8_t pointer_type = ((operand_control & GenerateFieldBitmask<uint16_t>(10, 2)) >> 10);
					switch (pointer_type)
					{
						case 0:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							if (!perform_jump)
							{
								++cycles_processed;
								break;
							}
							Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
							uint8_t source_region_id = extra_data[0];
							DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 2:
								{
									offset_data.offset = CurrentCPU.SI.offset;
									break;
								}
							}
							if (!cross_region_jump)
							{
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, source_region_id, source_pointer, offset_data);
								CurrentCPU.SR |= 0x01;
								cycles_processed += 2;
							}
							else
							{
								uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, source_region_id, source_pointer + 4, offset_data) & 0xF);
								if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
								{
									cycles_processed += 2;
									break;
								}
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, source_region_id, source_pointer, offset_data);
								SetProgramRegion(CurrentCPU.IP, new_region_id);
								CurrentCPU.SR |= 0x03;
								cycles_processed += 4;
							}
							break;
						}
						case 1:
						{
							if (!perform_jump)
							{
								++cycles_processed;
								break;
							}
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 1:
								{
									DWord_LE &offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
									offset_data.offset = offset;
									break;
								}
								case 2:
								{
									offset_data.offset = CurrentCPU.SI.offset;
									break;
								}
							}
							if (!cross_region_jump)
							{
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, offset_data);
								CurrentCPU.SR |= 0x01;
								cycles_processed += 2;
							}
							else
							{
								Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
								uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address + 4, offset_data) & 0xF);
								if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
								{
									cycles_processed += 2;
									break;
								}
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, offset_data);
								SetProgramRegion(CurrentCPU.IP, new_region_id);
								CurrentCPU.SR |= 0x03;
								cycles_processed += 4;
							}
							break;
						}
						case 2:
						{
							if (!perform_jump)
							{
								++cycles_processed;
								break;
							}
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 1:
								{
									DWord_LE &offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
									offset_data.offset = offset;
									break;
								}
								case 2:
								{
									offset_data.offset = CurrentCPU.SI.offset;
									break;
								}
							}
							if (!cross_region_jump)
							{
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, offset_data);
								CurrentCPU.SR |= 0x01;
								cycles_processed += 2;
							}
							else
							{
								Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
								uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address + 4, offset_data) & 0xF);
								if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
								{
									cycles_processed += 2;
									break;
								}
								CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, offset_data);
								SetProgramRegion(CurrentCPU.IP, new_region_id);
								CurrentCPU.SR |= 0x03;
								cycles_processed += 4;
							}
							break;
						}
					}
					break;
				}
			}
		}

		inline void SetClear(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			struct alignas(8) SetClearBaseInstruction
			{
				Word_LE instruction_type;
				Word_LE operand_control;
				DWord_LE flags_1;
			} set_clear_base_instruction = reinterpret_cast<SetClearBaseInstruction &>(instruction_data);
			uint8_t toggle_state = (set_clear_base_instruction.operand_control & GenerateFieldBitmask<uint16_t>(0, 1));
			uint8_t flags_size = ((set_clear_base_instruction.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
			if (toggle_state)
			{
				CurrentCPU.FL |= static_cast<QWord_LE>(set_clear_base_instruction.flags_1);
			}
			else
			{
				CurrentCPU.FL &= ~(static_cast<QWord_LE>(set_clear_base_instruction.flags_1));
			}
			if (flags_size >= 1)
			{
				ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
				DWord_LE &flags_2 = reinterpret_cast<DWord_LE &>(extra_data[0]);
				if (toggle_state)
				{
					CurrentCPU.FL |= (static_cast<QWord_LE>(flags_2) << 32);
				}
				else
				{
					CurrentCPU.FL &= ~(static_cast<QWord_LE>(flags_2) << 32);
				}
			}
			++cycles_processed;
		}

		inline void ChipCall(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			struct alignas(8) ChipCallInstruction
			{
				Word_LE instruction_type;
				Word_LE virtual_chipset_function_id;
				uint8_t virtual_chipset_port;
			} chipcall_instruction = reinterpret_cast<ChipCallInstruction &>(instruction_data);
			switch (chipcall_instruction.virtual_chipset_port)
			{
				case 0x00:
				{
					GPU *CurrentGPU = CurrentCPU.CurrentMotherboard->GetGPU();
					(*CurrentGPU)(chipcall_instruction.virtual_chipset_function_id, CurrentCPU.GPR_Registers);
					break;
				}
				case 0x03:
				{
					Input *CurrentInput = CurrentCPU.CurrentMotherboard->GetInput();
					(*CurrentInput)(chipcall_instruction.virtual_chipset_function_id, CurrentCPU.GPR_Registers);
					break;
				}
				case 0x07:
				{
					FloatingPoint *CurrentFloatingPoint = CurrentCPU.CurrentMotherboard->GetFloatingPoint();
					(*CurrentFloatingPoint)(chipcall_instruction.virtual_chipset_function_id, CurrentCPU.GPR_Registers);
					break;
				}
			}
			++cycles_processed;
		}

		inline void ShadowFetchAndExecute(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
		{
			struct alignas(8) ShadowFetchAndExecuteInstruction
			{
				Word_LE instruction_type;
				uint8_t region;
				uint8_t unused;
				DWord_LE pointer;
			} shadow_fetch_and_execute_instruction = reinterpret_cast<ShadowFetchAndExecuteInstruction &>(instruction_data);
			if (shadow_fetch_and_execute_instruction.pointer % sizeof(QWord_LE) == 0 && shadow_fetch_and_execute_instruction.region < 15)
			{
				MPRegisterData tmp = CurrentCPU.IP;
				CurrentCPU.IP.address = shadow_fetch_and_execute_instruction.pointer;
				SetProgramRegion(CurrentCPU.IP, shadow_fetch_and_execute_instruction.region);
				BaseInstructionData TargetInstruction = CurrentCPU.FastFetchExtraData<BaseInstructionData>();
				if (static_cast<InstructionType>(static_cast<uint16_t>(TargetInstruction.instruction_type)) == InstructionType::ShadowFetchAndExecute)
				{
					CurrentCPU.IP = tmp;
				}
				else
				{
					CurrentCPU.instruction_function_table[TargetInstruction.instruction_type].func(CurrentCPU, TargetInstruction, cycles_processed);
					if (!(CurrentCPU.SR & 0x01))
					{
						CurrentCPU.IP = tmp;
					}
					else
					{
						if (!(CurrentCPU.SR & 0x02))
						{
							CurrentCPU.IP.memory_control = tmp.memory_control;
						}
					}
				}
			}
			++cycles_processed;
		}
	}
}

#endif
