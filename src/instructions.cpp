#include "instructions.hpp"
#include "registers.hpp"
#include "math.hpp"
#include "bits.hpp"
#include "system.hpp"
#include "cpu.hpp"
#include <fmt/core.h>

void ClassicVCom_Nova64::Instruction::CommonExecuteCycles::Complete_ExecuteCycle(InstructionCallbackData &data)
{
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <void (*callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::CommonExecuteCycles::Dummy_ExecuteCycle(InstructionCallbackData &data)
{
	data.callback = callback;
}

void ClassicVCom_Nova64::Instruction::Interrupt::ExecuteCycle_1(InstructionCallbackData &data)
{
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.IP);
	PushDataToStack<uint8_t>(data.CurrentCPU, current_program_memory_control.region_id);
	data.callback = ExecuteCycle_2;
}

void ClassicVCom_Nova64::Instruction::Interrupt::ExecuteCycle_2(InstructionCallbackData &data)
{
	PushDataToStack<DWord_LE>(data.CurrentCPU, data.CurrentCPU.IP.address);
	data.callback = ExecuteCycle_3;
}

void ClassicVCom_Nova64::Instruction::Interrupt::ExecuteCycle_3(InstructionCallbackData &data)
{
	PushDataToStack<QWord_LE>(data.CurrentCPU, data.CurrentCPU.FL);
	data.callback = ExecuteCycle_4;
}

void ClassicVCom_Nova64::Instruction::Interrupt::ExecuteCycle_4(InstructionCallbackData &data)
{
	CrossRegionJumpData &NewIP = reinterpret_cast<CrossRegionJumpData &>(data.CurrentCPU.data_bus);
	data.CurrentCPU.IP.address = NewIP.address;
	data.callback = ExecuteCycle_5;
}

void ClassicVCom_Nova64::Instruction::Interrupt::ExecuteCycle_5(InstructionCallbackData &data)
{
	CrossRegionJumpData &NewIP = reinterpret_cast<CrossRegionJumpData &>(data.CurrentCPU.data_bus);
	SetProgramRegion(data.CurrentCPU.IP, NewIP.region);
	data.CurrentCPU.data_bus = 0;
	data.CurrentCPU.FL &= ~(0x100);
	data.CurrentCPU.SR |= 0x04;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::InvalidInstructionType::ExecuteCycle(InstructionCallbackData &data)
{
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::NoOperationInstruction::ExecuteCycle(InstructionCallbackData &data)
{
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::SystemCall(ClassicVCom_Nova64::CPU &CurrentCPU, ClassicVCom_Nova64::BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	Word_LE &system_call_id = reinterpret_cast<Word_LE &>(instruction_data.data[0]);
	++cycles_processed;
}
*/

void ClassicVCom_Nova64::Instruction::SystemCallInstruction::ExecuteCycle(InstructionCallbackData &data)
{
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::InterruptReturn(ClassicVCom_Nova64::CPU &CurrentCPU, ClassicVCom_Nova64::BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	if (CurrentCPU.SR & 0x04)
	{
		ChipsetInterruptRequestData irq = CurrentCPU.irq_queue.front();
		CurrentCPU.FL = PopDataFromStack<QWord_LE>(CurrentCPU);
		CurrentCPU.IP.address = PopDataFromStack<DWord_LE>(CurrentCPU);
		SetProgramRegion(CurrentCPU.IP, PopDataFromStack<uint8_t>(CurrentCPU));
		CurrentCPU.CurrentMotherboard->ChipsetEndOfInterrupt(irq.chipset, irq.interrupt);
		CurrentCPU.irq_queue.pop_front();
		CurrentCPU.SR &= ~(0x04);
		cycles_processed += 4;
	}
	else
	{
		++cycles_processed;
	}
}
*/

void ClassicVCom_Nova64::Instruction::InterruptReturnInstruction::ExecuteCycle_1(InstructionCallbackData &data)
{
	if (data.CurrentCPU.SR & 0x04)
	{
		data.CurrentCPU.data_bus = PopDataFromStack<QWord_LE>(data.CurrentCPU);
		data.callback = ExecuteCycle_2;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::InterruptReturnInstruction::ExecuteCycle_2(InstructionCallbackData &data)
{
	data.CurrentCPU.FL = data.CurrentCPU.data_bus;
	data.CurrentCPU.SR |= 0x01;
	data.CurrentCPU.data_bus = PopDataFromStack<DWord_LE>(data.CurrentCPU);
	data.callback = ExecuteCycle_3;
}

void ClassicVCom_Nova64::Instruction::InterruptReturnInstruction::ExecuteCycle_3(InstructionCallbackData &data)
{
	data.CurrentCPU.IP.address = data.CurrentCPU.data_bus;
	data.CurrentCPU.SR |= 0x02;
	data.CurrentCPU.data_bus = PopDataFromStack<uint8_t>(data.CurrentCPU);
	data.callback = ExecuteCycle_4;
}

void ClassicVCom_Nova64::Instruction::InterruptReturnInstruction::ExecuteCycle_4(InstructionCallbackData &data)
{
	SetProgramRegion(data.CurrentCPU.IP, data.CurrentCPU.data_bus);
	data.CurrentCPU.data_bus = 0;
	data.callback = ExecuteCycle_5;
}

void ClassicVCom_Nova64::Instruction::InterruptReturnInstruction::ExecuteCycle_5(InstructionCallbackData &data)
{
	ChipsetInterruptRequestData irq = data.CurrentCPU.irq_queue.front();
	data.CurrentCPU.CurrentMotherboard->ChipsetEndOfInterrupt(irq.chipset, irq.interrupt);
	data.CurrentCPU.irq_queue.pop_front();
	data.CurrentCPU.SR &= ~(0x04);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::Push(ClassicVCom_Nova64::CPU &CurrentCPU, ClassicVCom_Nova64::BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	Word_LE &operand_control = reinterpret_cast<Word_LE &>(instruction_data.data[0]);
	uint8_t operand_type = (operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			uint8_t field_index = ((operand_control & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
			if (field_index > (0x7 >> data_size))
			{
				++cycles_processed;
				return;
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<uint8_t>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 1:
						{
							WordField &register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<Word_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<DWord_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(CurrentCPU, CurrentCPU.GPR_Registers[source_register]);
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
							PushDataToStack<DWord_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(CurrentCPU, std::bit_cast<QWord_LE>(CurrentCPU.SI));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
							PushDataToStack<DWord_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(CurrentCPU, std::bit_cast<QWord_LE>(CurrentCPU.DI));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
							PushDataToStack<DWord_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(CurrentCPU, std::bit_cast<QWord_LE>(CurrentCPU.BP));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
							PushDataToStack<DWord_LE>(CurrentCPU, register_field[field_index]);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(CurrentCPU, std::bit_cast<QWord_LE>(CurrentCPU.SP));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			switch (data_size)
			{
				case 0:
				{
					PushDataToStack<uint8_t>(CurrentCPU, instruction_data.data[2]);
					cycles_processed += 2;
					break;
				}
				case 1:
				{
					Word_LE &immediate_value = reinterpret_cast<Word_LE &>(instruction_data.data[2]);
					PushDataToStack<Word_LE>(CurrentCPU, immediate_value);
					cycles_processed += 2;
					break;
				}
				case 2:
				{
					DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
					PushDataToStack<DWord_LE>(CurrentCPU, immediate_value);
					cycles_processed += 2;
					break;
				}
				case 3:
				{
					ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
					PushDataToStack<QWord_LE>(CurrentCPU, std::bit_cast<QWord_LE>(extra_data));
					cycles_processed += 2;
					break;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t offset_type = ((operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
			uint8_t pointer_type = ((operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_source = ((operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_source)
					{
						case 0:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
							uint8_t region_id = instruction_data.data[2];
							DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 2:
								{
									offset_data.offset = CurrentCPU.SI.offset;
									offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, region_id, source_pointer, offset_data);
									PushDataToStack<uint8_t>(CurrentCPU, data);
									cycles_processed += 3;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, source_pointer, offset_data);
									PushDataToStack<Word_LE>(CurrentCPU, data);
									cycles_processed += 3;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, offset_data);
									PushDataToStack<DWord_LE>(CurrentCPU, data);
									cycles_processed += 3;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, offset_data);
									PushDataToStack<QWord_LE>(CurrentCPU, data);
									cycles_processed += 3;
									break;
								}
							}
						}
						case 1:
						{
							++cycles_processed;
							break;
						}
						case 2:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE &current_memory_group = reinterpret_cast<Word_LE &>(instruction_data.data[2]);
							uint8_t chipset = instruction_data.data[4];
							DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							ChipsetReturnCode read_return = ChipsetReturnCode::Ok;
							switch (offset_type)
							{
								case 2:
								{
									offset_data.offset = CurrentCPU.SI.offset;
									offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(chipset, current_memory_group, source_pointer, offset_data, read_return);
									if (read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PushDataToStack<uint8_t>(CurrentCPU, data);
										cycles_processed += 3;
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 1:
								{
									Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(chipset, current_memory_group, source_pointer, offset_data, read_return);
									if (read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PushDataToStack<Word_LE>(CurrentCPU, data);
										cycles_processed += 3;
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 2:
								{
									DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(chipset, current_memory_group, source_pointer, offset_data, read_return);
									if (read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PushDataToStack<DWord_LE>(CurrentCPU, data);
										cycles_processed += 3;
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 3:
								{
									QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(chipset, current_memory_group, source_pointer, offset_data, read_return);
									if (read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PushDataToStack<QWord_LE>(CurrentCPU, data);
										cycles_processed += 3;
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
					Word_LE bp_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
					uint8_t current_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
					if (bp_program_id != current_program_id)
					{
						if (!HasRegionFlagSupport<0x08>(CurrentCPU, bp_program_id, current_region_id))
						{
							++cycles_processed;
							break;
						}
					}
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = CurrentCPU.SI.offset;
							offset_data.index_register_used = IndexRegisterType::Source;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, bp_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
							PushDataToStack<uint8_t>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 1:
						{
							Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, bp_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
							PushDataToStack<Word_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 2:
						{
							DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, bp_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
							PushDataToStack<DWord_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 3:
						{
							QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, bp_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
							PushDataToStack<QWord_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
					}
					break;
				}
				case 2:
				{
					Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
					Word_LE sp_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
					uint8_t current_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
					if (sp_program_id != current_program_id)
					{
						if (!HasRegionFlagSupport<0x08>(CurrentCPU, sp_program_id, current_region_id))
						{
							++cycles_processed;
							break;
						}
					}
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = CurrentCPU.SI.offset;
							offset_data.index_register_used = IndexRegisterType::Source;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, sp_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
							PushDataToStack<uint8_t>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 1:
						{
							Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, sp_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
							PushDataToStack<Word_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 2:
						{
							DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, sp_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
							PushDataToStack<DWord_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
						case 3:
						{
							QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, sp_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
							PushDataToStack<QWord_LE>(CurrentCPU, data);
							cycles_processed += 3;
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::PushInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PushInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		std::array<uint8_t, 4> data;
	} &push_instruction_base = reinterpret_cast<PushInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			uint8_t field_index = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
			if (field_index > (0x7 >> data_size))
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
				break;
			}
			uint8_t &source_register = push_instruction_base.data[0];
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<uint8_t>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 1:
						{
							WordField &register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<Word_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[source_register]);
							PushDataToStack<DWord_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(data.CurrentCPU, data.CurrentCPU.GPR_Registers[source_register]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
							PushDataToStack<DWord_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(data.CurrentCPU, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
							PushDataToStack<DWord_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(data.CurrentCPU, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
							PushDataToStack<DWord_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(data.CurrentCPU, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
							PushDataToStack<DWord_LE>(data.CurrentCPU, register_field[field_index]);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
						case 3:
						{
							PushDataToStack<QWord_LE>(data.CurrentCPU, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			switch (data_size)
			{
				case 0:
				{
					PushDataToStack<uint8_t>(data.CurrentCPU, push_instruction_base.data[0]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
					break;
				}
				case 1:
				{
					Word_LE &immediate_value = reinterpret_cast<Word_LE &>(push_instruction_base.data[0]);
					PushDataToStack<Word_LE>(data.CurrentCPU, immediate_value);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
					break;
				}
				case 2:
				{
					DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(push_instruction_base.data[0]);
					PushDataToStack<DWord_LE>(data.CurrentCPU, immediate_value);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;;
					break;
				}
				case 3:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.callback = QWord_ImmediateValue_ExecuteCycle;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t pointer_type = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_source = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_source)
					{
						case 0:
						{
							data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					uint8_t offset_type = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
					Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
					Word_LE bp_program_id = (data.CurrentCPU.BP.memory_control & 0xFFF);
					uint8_t current_region_id = ((data.CurrentCPU.BP.memory_control & 0xF000) >> 12);
					if (bp_program_id != current_program_id)
					{
						if (!HasRegionFlagSupport<0x08>(data.CurrentCPU, bp_program_id, current_region_id))
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(push_instruction_base.data[0]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = data.CurrentCPU.SI.offset;
							offset_data.index_register_used = IndexRegisterType::Source;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, bp_program_id, current_region_id, data.CurrentCPU.BP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<uint8_t>>;
							break;
						}
						case 1:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, bp_program_id, current_region_id, data.CurrentCPU.BP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<Word_LE>>;
							break;
						}
						case 2:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, bp_program_id, current_region_id, data.CurrentCPU.BP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<DWord_LE>>;
							break;
						}
						case 3:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, bp_program_id, current_region_id, data.CurrentCPU.BP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<QWord_LE>>;
							break;
						}
					}
					break;
				}
				case 2:
				{
					uint8_t offset_type = ((push_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
					Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
					Word_LE sp_program_id = (data.CurrentCPU.SP.memory_control & 0xFFF);
					uint8_t current_region_id = ((data.CurrentCPU.SP.memory_control & 0xF000) >> 12);
					if (sp_program_id != current_program_id)
					{
						if (!HasRegionFlagSupport<0x08>(data.CurrentCPU, sp_program_id, current_region_id))
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(push_instruction_base.data[0]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = data.CurrentCPU.SI.offset;
							offset_data.index_register_used = IndexRegisterType::Source;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, sp_program_id, current_region_id, data.CurrentCPU.SP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<uint8_t>>;
							break;
						}
						case 1:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, sp_program_id, current_region_id, data.CurrentCPU.SP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<Word_LE>>;
							break;
						}
						case 2:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, sp_program_id, current_region_id, data.CurrentCPU.SP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<DWord_LE>>;
							break;
						}
						case 3:
						{
							data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, sp_program_id, current_region_id, data.CurrentCPU.SP.address, offset_data);
							data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<QWord_LE>>;
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PushInstruction::Data_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	PushDataToStack<T>(data.CurrentCPU, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
}

void ClassicVCom_Nova64::Instruction::PushInstruction::QWord_ImmediateValue_ExecuteCycle(InstructionCallbackData &data)
{
	PushDataToStack<QWord_LE>(data.CurrentCPU, data.instruction_data[1]);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PushInstruction::Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PushAbsolutePointerSelfInstructionData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		uint8_t region_id;
		std::array<uint8_t, 3> unused;
		DWord_LE source_pointer;
	} &push_absolute_pointer_self_instruction = reinterpret_cast<PushAbsolutePointerSelfInstructionData &>(data.instruction_data[0]);
	uint8_t offset_type = ((push_absolute_pointer_self_instruction.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			offset_data.offset = data.CurrentCPU.SI.offset;
			offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_id, push_absolute_pointer_self_instruction.region_id, push_absolute_pointer_self_instruction.source_pointer, offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PushInstruction::Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PushAbsolutePointerChipsetInstructionData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		Word_LE current_memory_group;
		uint8_t chipset;
		uint8_t unused;
		DWord_LE source_pointer;
	} &push_absolute_pointer_chipset_instruction = reinterpret_cast<PushAbsolutePointerChipsetInstructionData &>(data.instruction_data[0]);
	uint8_t offset_type = ((push_absolute_pointer_chipset_instruction.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			offset_data.offset = data.CurrentCPU.SI.offset;
			offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	T value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<T>(push_absolute_pointer_chipset_instruction.chipset, push_absolute_pointer_chipset_instruction.current_memory_group, push_absolute_pointer_chipset_instruction.source_pointer, offset_data, read_return);
	if (read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Data_ExecuteCycle<T>>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

/*
void ClassicVCom_Nova64::Instruction::Pop(ClassicVCom_Nova64::CPU &CurrentCPU, ClassicVCom_Nova64::BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	Word_LE &operand_control = reinterpret_cast<Word_LE &>(instruction_data.data[0]);
	uint8_t operand_type = (operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			uint8_t field_index = ((operand_control & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
			if (field_index > (0x7 >> data_size))
			{
				++cycles_processed;
				return;
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[source_register]);
							register_field[field_index] = PopDataFromStack<uint8_t>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 1:
						{
							WordField &register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[source_register]);
							register_field[field_index] = PopDataFromStack<Word_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[source_register]);
							register_field[field_index] = PopDataFromStack<DWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							CurrentCPU.GPR_Registers[source_register] = PopDataFromStack<QWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
							register_field[field_index] = PopDataFromStack<DWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							CurrentCPU.SI = std::bit_cast<IndexRegisterData>(PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
							register_field[field_index] = PopDataFromStack<DWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							CurrentCPU.DI = std::bit_cast<IndexRegisterData>(PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
							register_field[field_index] = PopDataFromStack<DWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							CurrentCPU.BP = std::bit_cast<MPRegisterData>(PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							DWordField &register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
							register_field[field_index] = PopDataFromStack<DWord_LE>(CurrentCPU);
							cycles_processed += 2;
							break;
						}
						case 3:
						{
							CurrentCPU.SP = std::bit_cast<MPRegisterData>(PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 2;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			switch (data_size)
			{
				case 0:
				{
					PopDataFromStack<uint8_t>(CurrentCPU);
					cycles_processed += 2;
					break;
				}
				case 1:
				{
					PopDataFromStack<Word_LE>(CurrentCPU);
					cycles_processed += 2;
					break;
				}
				case 2:
				{
					PopDataFromStack<DWord_LE>(CurrentCPU);
					cycles_processed += 2;
					break;
				}
				case 3:
				{
					PopDataFromStack<QWord_LE>(CurrentCPU);
					cycles_processed += 2;
					break;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t offset_type = ((operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
			uint8_t pointer_type = ((operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_destination = ((operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_destination)
					{
						case 0:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
							uint8_t region_id = instruction_data.data[2];
							DWord_LE &destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 2:
								{
									offset_data.offset = CurrentCPU.DI.offset;
									offset_data.index_register_used = IndexRegisterType::Destination;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, region_id, destination_pointer, offset_data, PopDataFromStack<uint8_t>(CurrentCPU));
									cycles_processed += 3;
									break;
								}
								case 1:
								{
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, offset_data, PopDataFromStack<Word_LE>(CurrentCPU));
									cycles_processed += 3;
									break;
								}
								case 2:
								{
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, offset_data, PopDataFromStack<DWord_LE>(CurrentCPU));
									cycles_processed += 3;
									break;
								}
								case 3:
								{
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, offset_data, PopDataFromStack<QWord_LE>(CurrentCPU));
									cycles_processed += 3;
									break;
								}
							}
							break;
						}
						case 1:
						{
							++cycles_processed;
							break;
						}
						case 2:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE &current_memory_group = reinterpret_cast<Word_LE &>(instruction_data.data[2]);
							uint8_t chipset = instruction_data.data[4];
							DWord_LE &destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData offset_data = { 0, IndexRegisterType::None };
							ChipsetReturnCode write_return = ChipsetReturnCode::Ok;
							switch (offset_type)
							{
								case 2:
								{
									offset_data.offset = CurrentCPU.DI.offset;
									offset_data.index_register_used = IndexRegisterType::Destination;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = PopDataFromStack<uint8_t>(CurrentCPU);
									CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(chipset, current_memory_group, destination_pointer, offset_data, data, write_return);
									cycles_processed += (write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
									break;
								}
								case 1:
								{
									Word_LE data = PopDataFromStack<Word_LE>(CurrentCPU);
									CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(chipset, current_memory_group, destination_pointer, offset_data, data, write_return);
									cycles_processed += (write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
									break;
								}
								case 2:
								{
									DWord_LE data = PopDataFromStack<DWord_LE>(CurrentCPU);
									CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(chipset, current_memory_group, destination_pointer, offset_data, data, write_return);
									cycles_processed += (write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
									break;
								}
								case 3:
								{
									QWord_LE data = PopDataFromStack<QWord_LE>(CurrentCPU);
									CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(chipset, current_memory_group, destination_pointer, offset_data, data, write_return);
									cycles_processed += (write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					Word_LE current_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
					uint8_t current_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = CurrentCPU.DI.offset;
							offset_data.index_register_used = IndexRegisterType::Destination;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data, PopDataFromStack<uint8_t>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 1:
						{
							StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data, PopDataFromStack<Word_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 2:
						{
							StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data, PopDataFromStack<DWord_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 3:
						{
							StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data, PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
					}
					break;
				}
				case 2:
				{
					Word_LE current_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
					uint8_t current_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							offset_data.offset = relative_offset;
							break;
						}
						case 2:
						{
							offset_data.offset = CurrentCPU.DI.offset;
							offset_data.index_register_used = IndexRegisterType::Destination;
							break;
						}
					}
					switch (data_size)
					{
						case 0:
						{
							StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data, PopDataFromStack<uint8_t>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 1:
						{
							StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data, PopDataFromStack<Word_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 2:
						{
							StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data, PopDataFromStack<DWord_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
						case 3:
						{
							StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data, PopDataFromStack<QWord_LE>(CurrentCPU));
							cycles_processed += 3;
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::PopInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		std::array<uint8_t, 4> data;
	} &pop_instruction_base = reinterpret_cast<PopInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (pop_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((pop_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (data_size)
	{
		case 0:
		{
			data.CurrentCPU.data_bus = PopDataFromStack<uint8_t>(data.CurrentCPU);
			break;
		}
		case 1:
		{
			data.CurrentCPU.data_bus = PopDataFromStack<Word_LE>(data.CurrentCPU);
			break;
		}
		case 2:
		{
			data.CurrentCPU.data_bus = PopDataFromStack<DWord_LE>(data.CurrentCPU);
			break;
		}
		case 3:
		{
			data.CurrentCPU.data_bus = PopDataFromStack<QWord_LE>(data.CurrentCPU);
			break;
		}
	}
	switch (operand_type)
	{
		case 0:
		{
			uint8_t field_index = ((pop_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
			if (field_index > (0x7 >> data_size))
			{
				data.CurrentCPU.data_bus = 0;
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
				break;
			}
			switch (data_size)
			{
				case 0:
				{
					data.callback = Register_ExecuteCycle<uint8_t>;
					break;
				}
				case 1:
				{
					data.callback = Register_ExecuteCycle<Word_LE>;
					break;
				}
				case 2:
				{
					data.callback = Register_ExecuteCycle<DWord_LE>;
					break;
				}
				case 3:
				{
					data.callback = Register_ExecuteCycle<QWord_LE>;
					break;
				}
			}
			break;
		}
		case 1:
		{
			data.callback = DataDiscard_ExecuteCycle;
			break;
		}
		case 2:
		{
			uint8_t pointer_type = ((pop_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_destination = ((pop_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_destination)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<QWord_LE>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<QWord_LE>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (data_size)
					{
						case 0:
						{
							data.callback = Base_Pointer_ExecuteCycle<uint8_t>;
							break;
						}
						case 1:
						{
							data.callback = Base_Pointer_ExecuteCycle<Word_LE>;
							break;
						}
						case 2:
						{
							data.callback = Base_Pointer_ExecuteCycle<DWord_LE>;
							break;
						}
						case 3:
						{
							data.callback = Base_Pointer_ExecuteCycle<QWord_LE>;
							break;
						}
					}
					break;
				}
				case 2:
				{
					switch (data_size)
					{
						case 0:
						{
							data.callback = Stack_Pointer_ExecuteCycle<uint8_t>;
							break;
						}
						case 1:
						{
							data.callback = Stack_Pointer_ExecuteCycle<Word_LE>;
							break;
						}
						case 2:
						{
							data.callback = Stack_Pointer_ExecuteCycle<DWord_LE>;
							break;
						}
						case 3:
						{
							data.callback = Stack_Pointer_ExecuteCycle<QWord_LE>;
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T, bool is_byte, bool is_word, bool is_dword, bool is_qword>
void ClassicVCom_Nova64::Instruction::PopInstruction::Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		uint8_t destination_register;
	} &pop_instruction_register = reinterpret_cast<PopInstructionRegisterData &>(data.instruction_data[0]);
	uint8_t field_index = ((pop_instruction_register.operand_control & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
	switch (pop_instruction_register.destination_register)
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
			if (is_byte)
			{
				ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[pop_instruction_register.destination_register]);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_word)
			{
				WordField &register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[pop_instruction_register.destination_register]);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_dword)
			{
				DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[pop_instruction_register.destination_register]);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_qword)
			{
				data.CurrentCPU.GPR_Registers[pop_instruction_register.destination_register] = data.CurrentCPU.data_bus;
			}
			break;
		}
		case 0x10:
		{
			if (is_dword)
			{
				DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_qword)
			{
				data.CurrentCPU.SI = std::bit_cast<IndexRegisterData>(data.CurrentCPU.data_bus);
			}
			break;
		}
		case 0x11:
		{
			if (is_dword)
			{
				DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_qword)
			{
				data.CurrentCPU.DI = std::bit_cast<IndexRegisterData>(data.CurrentCPU.data_bus);
			}
			break;
		}
		case 0x20:
		{
			if (is_dword)
			{
				DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_qword)
			{
				data.CurrentCPU.BP = std::bit_cast<MPRegisterData>(data.CurrentCPU.data_bus);
				break;
			}
			break;
		}
		case 0x21:
		{
			if (is_dword)
			{
				DWordField &register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
				register_field[field_index] = data.CurrentCPU.data_bus;
			}
			else if (is_qword)
			{
				data.CurrentCPU.SP = std::bit_cast<MPRegisterData>(data.CurrentCPU.data_bus);
			}
			break;
		}
	}
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::PopInstruction::DataDiscard_ExecuteCycle(InstructionCallbackData &data)
{
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data)
{
	data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
	data.callback = Absolute_Pointer_Self_ExecuteCycle_2<T>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		uint8_t region_id;
		std::array<uint8_t, 3> unused;
		DWord_LE destination_pointer;
	} &pop_instruction_absolute_pointer_self = reinterpret_cast<PopInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t offset_type = ((pop_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			offset_data.offset = data.CurrentCPU.DI.offset;
			offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, pop_instruction_absolute_pointer_self.region_id, pop_instruction_absolute_pointer_self.destination_pointer, offset_data, data.CurrentCPU.data_bus);
	data.CurrentCPU.data_bus = 0;
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data)
{
	data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
	data.callback = Absolute_Pointer_Chipset_ExecuteCycle_2<T>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		Word_LE current_memory_group;
		uint8_t chipset;
		uint8_t unused;
		DWord_LE destination_pointer;
	} &pop_instruction_absolute_pointer_chipset = reinterpret_cast<PopInstructionAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t offset_type = ((pop_instruction_absolute_pointer_chipset.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode write_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			offset_data.offset = data.CurrentCPU.DI.offset;
			offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<T>(pop_instruction_absolute_pointer_chipset.chipset, pop_instruction_absolute_pointer_chipset.current_memory_group, pop_instruction_absolute_pointer_chipset.destination_pointer, offset_data, data.CurrentCPU.data_bus, write_return);
	data.CurrentCPU.data_bus = 0;
	if (write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Base_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionBasePointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE relative_offset;
	} &pop_instruction_base_pointer = reinterpret_cast<PopInstructionBasePointerData &>(data.instruction_data[0]);
	uint8_t offset_type = ((pop_instruction_base_pointer.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.BP.memory_control & 0xFFF);
	uint8_t current_region_id = ((data.CurrentCPU.BP.memory_control & 0xF000) >> 12);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 1:
		{
			offset_data.offset = pop_instruction_base_pointer.relative_offset;
			break;
		}
		case 2:
		{
			offset_data.offset = data.CurrentCPU.DI.offset;
			offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, current_region_id, data.CurrentCPU.BP.address, offset_data, data.CurrentCPU.data_bus);
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? CommonExecuteCycles::Dummy_ExecuteCycle<nullptr> : CommonExecuteCycles::Dummy_ExecuteCycle<ShadowFetchAndExecuteInstruction::ExecuteCycle_3>;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::PopInstruction::Stack_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) PopInstructionStackPointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE relative_offset;
	} &pop_instruction_stack_pointer = reinterpret_cast<PopInstructionStackPointerData &>(data.instruction_data[0]);
	uint8_t offset_type = ((pop_instruction_stack_pointer.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.SP.memory_control & 0xFFF);
	uint8_t current_region_id = ((data.CurrentCPU.SP.memory_control & 0xF000) >> 12);
	OffsetData offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 1:
		{
			offset_data.offset = pop_instruction_stack_pointer.relative_offset;
			break;
		}
		case 2:
		{
			offset_data.offset = data.CurrentCPU.DI.offset;
			offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, current_region_id, data.CurrentCPU.SP.address, offset_data, data.CurrentCPU.data_bus);
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? CommonExecuteCycles::Dummy_ExecuteCycle<nullptr> : CommonExecuteCycles::Dummy_ExecuteCycle<ShadowFetchAndExecuteInstruction::ExecuteCycle_3>;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::Move(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	struct OperandControlData
	{
		Word_LE o_0, o_1;

		Word_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return o_0; }
				case 1: { return o_1; }
			}
			return o_0;
		}
	};
	struct OperandTypeData
	{
		uint8_t o_0, o_1;

		uint8_t &operator[](int index)
		{
			switch (index)
			{
				case 0: { return o_0; }
				case 1: { return o_1; }
			}
			return o_0;
		}
	};
	OperandControlData &operand_control = reinterpret_cast<OperandControlData &>(instruction_data.data[0]);
	OperandTypeData operand_type = { static_cast<uint8_t>(operand_control[0] & GenerateFieldBitmask<uint16_t>(0, 1)), static_cast<uint8_t>(operand_control[1] & GenerateFieldBitmask<uint16_t>(0, 2)) };
	uint8_t data_size = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(4, 1));
	switch (operand_type[0])
	{
		case 0:
		{
			uint8_t operand_0_register = instruction_data.data[4];
			uint8_t operand_0_field_index = 0;
			if (register_field_mode)
			{
				operand_0_field_index = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5);
			}
			switch (operand_type[1])
			{
				case 0:
				{
					uint8_t operand_1_register = instruction_data.data[5];
					uint8_t operand_1_field_index = 0;
					if (register_field_mode)
					{
						operand_1_field_index = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2);
					}
					switch (operand_0_register)
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
							switch (operand_1_register)
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
									GPRegisterToGPRegisterTransfer(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x10:
								{
									IndexRegisterToGPRegisterTransfer(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x11:
								{
									IndexRegisterToGPRegisterTransfer(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x20:
								{
									MPRegisterToGPRegisterTransfer(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x21:
								{
									MPRegisterToGPRegisterTransfer(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (operand_1_register)
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
									GPRegisterToIndexRegisterTransfer(CurrentCPU.SI, CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x10:
								{
									++cycles_processed;
									break;
								}
								case 0x11:
								{
									IndexRegisterToIndexRegisterTransfer(CurrentCPU.SI, CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x20:
								{
									MPRegisterToIndexRegisterTransfer(CurrentCPU.SI, CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x21:
								{
									MPRegisterToIndexRegisterTransfer(CurrentCPU.SI, CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (operand_1_register)
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
									GPRegisterToIndexRegisterTransfer(CurrentCPU.DI, CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x10:
								{
									IndexRegisterToIndexRegisterTransfer(CurrentCPU.DI, CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x11:
								{
									++cycles_processed;
									break;
								}
								case 0x20:
								{
									MPRegisterToIndexRegisterTransfer(CurrentCPU.DI, CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x21:
								{
									MPRegisterToIndexRegisterTransfer(CurrentCPU.DI, CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (operand_1_register)
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
									GPRegisterToMPRegisterTransfer(CurrentCPU.BP, CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x10:
								{
									IndexRegisterToMPRegisterTransfer(CurrentCPU.BP, CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x11:
								{
									IndexRegisterToMPRegisterTransfer(CurrentCPU.BP, CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x20:
								{
									++cycles_processed;
									break;
								}
								case 0x21:
								{
									MPRegisterToMPRegisterTransfer(CurrentCPU.BP, CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (operand_1_register)
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
									GPRegisterToMPRegisterTransfer(CurrentCPU.SP, CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x10:
								{
									IndexRegisterToMPRegisterTransfer(CurrentCPU.SP, CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x11:
								{
									IndexRegisterToMPRegisterTransfer(CurrentCPU.SP, CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x20:
								{
									MPRegisterToMPRegisterTransfer(CurrentCPU.SP, CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									++cycles_processed;
									break;
								}
								case 0x21:
								{
									++cycles_processed;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (operand_0_register)
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
							ImmediateValueToGPRegisterTransfer(CurrentCPU, CurrentCPU.GPR_Registers[operand_0_register], instruction_data, data_size, operand_0_field_index);
							++cycles_processed;
							break;
						}
						case 0x10:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_register_field[operand_0_field_index] = immediate_value;
									++cycles_processed;
									break;
								}
								case 3:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									CurrentCPU.SI = std::bit_cast<IndexRegisterData>(extra_data);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_register_field[operand_0_field_index] = immediate_value;
									++cycles_processed;
									break;
								}
								case 3:
								{
									ExtraQWordData<1>  extra_data = CurrentCPU.FetchExtraData<1>();
									CurrentCPU.DI = std::bit_cast<IndexRegisterData>(extra_data);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_register_field[operand_0_field_index] = immediate_value;
									++cycles_processed;
									break;
								}
								case 3:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									CurrentCPU.BP = std::bit_cast<MPRegisterData>(extra_data);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_register_field[operand_0_field_index] = immediate_value;
									++cycles_processed;
									break;
								}
								case 3:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									CurrentCPU.SP = std::bit_cast<MPRegisterData>(extra_data);
									++cycles_processed;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					PointerControlData operand_1_source_pointer_control = GetPointerControlData<2, 4, 6>(operand_control[1]);
					switch (operand_1_source_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_1_source_pointer_control.target)
							{
								case 0:
								{
									Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
									uint8_t operand_1_region_id = instruction_data.data[5];
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
									OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									switch (operand_1_source_pointer_control.offset_type)
									{
										case 2:
										{
											operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
											operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
											break;
										}
									}
									switch (operand_0_register)
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
											// SystemMemoryToGPRegisterTransfer(CurrentCPU, CurrentCPU.GPR_Registers[operand_0_register], current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data, data_size, operand_0_field_index);
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													CurrentCPU.GPR_Registers[operand_0_register] = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													CurrentCPU.SI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													CurrentCPU.DI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													CurrentCPU.BP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													CurrentCPU.SP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									++cycles_processed;
									break;
								}
								case 2:
								{
									struct alignas(8) ChipsetSourceOperandData
									{
										DWord_LE source_pointer;
										Word_LE memory_group;
									};
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									ChipsetSourceOperandData &operand_1_chipset_source_operand_data = reinterpret_cast<ChipsetSourceOperandData &>(extra_data);
									uint8_t operand_1_chipset = instruction_data.data[5];
									OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
									switch (operand_1_source_pointer_control.offset_type)
									{
										case 2:
										{
											operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
											operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
											break;
										}
									}
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														operand_0_register_field[operand_0_field_index] = data;
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														operand_0_register_field[operand_0_field_index] = data;
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														operand_0_register_field[operand_0_field_index] = data;
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 3:
												{
													QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														CurrentCPU.GPR_Registers[operand_0_register] = data;
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
							OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_1_source_offset_data.offset = relative_offset;
									break;
								}
								case 2:
								{
									operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
									operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (operand_0_register)
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
									// SystemMemoryToGPRegisterTransfer(CurrentCPU, CurrentCPU.GPR_Registers[operand_0_register], operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data, data_size, operand_0_field_index);
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.GPR_Registers[operand_0_register] = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.SI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.DI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.BP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.SP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.BP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
							OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_1_source_offset_data.offset = relative_offset;
									break;
								}
								case 2:
								{
									operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
									operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (operand_0_register)
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
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.GPR_Registers[operand_0_register] = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.SI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.DI = std::bit_cast<IndexRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.BP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											operand_0_register_field[operand_0_field_index] = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											CurrentCPU.SP = std::bit_cast<MPRegisterData>(LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, CurrentCPU.SP.address, operand_1_source_offset_data));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			PointerControlData operand_0_destination_pointer_control = GetPointerControlData<5, 7, 9>(operand_control[0]);
			switch (operand_type[1])
			{
				case 0:
				{
					uint8_t operand_1_register = instruction_data.data[5];
					uint8_t operand_1_field_index = 0;
					if (register_field_mode)
					{
						operand_1_field_index = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2);
					}
					switch (operand_0_destination_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.target)
							{
								case 0:
								{
									Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = instruction_data.data[4];
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, CurrentCPU.GPR_Registers[operand_1_register]);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SI));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.DI));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.BP));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SP));
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									++cycles_processed;
									break;
								}
								case 2:
								{
									struct alignas(8) ChipsetDestinationOperandData
									{
										DWord_LE destination_pointer;
										Word_LE memory_group;
									};
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									ChipsetDestinationOperandData &operand_0_chipset_destination_operand_data = reinterpret_cast<ChipsetDestinationOperandData &>(extra_data);
									uint8_t operand_0_chipset = instruction_data.data[4];
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_chipset_destination_operand_data.memory_group, operand_0_chipset_destination_operand_data.destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
													cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_chipset_destination_operand_data.memory_group, operand_0_chipset_destination_operand_data.destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
													cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
													CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_chipset_destination_operand_data.memory_group, operand_0_chipset_destination_operand_data.destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
													cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
													break;
												}
												case 3:
												{
													CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_chipset_destination_operand_data.memory_group, operand_0_chipset_destination_operand_data.destination_pointer, operand_0_destination_offset_data, CurrentCPU.GPR_Registers[operand_1_register], operand_0_chipset_write_return);
													cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
							OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
							switch (operand_0_destination_pointer_control.offset_type)
							{
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_destination_offset_data.offset = relative_offset;
									break;
								}
								case 2:
								{
									operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
									operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
									break;
								}
							}
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &operand_1_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, CurrentCPU.GPR_Registers[operand_1_register]);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SI));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.DI));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.BP));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SP));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
							OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
							switch (operand_0_destination_pointer_control.offset_type)
							{
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									operand_0_destination_offset_data.offset = relative_offset;
									break;
								}
								case 2:
								{
									operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
									operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
									break;
								}
							}
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &operand_1_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, CurrentCPU.GPR_Registers[operand_1_register]);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SI));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.DI));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.BP));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(CurrentCPU.SP));
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (data_size)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_0_region_id = instruction_data.data[4];
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, instruction_data.data[5]);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											struct alignas(8) ChipsetDestinationOperandData
											{
												DWord_LE destination_pointer;
												Word_LE memory_group;
											};
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											ChipsetDestinationOperandData &operand_0_chipset_destination_operand_data = reinterpret_cast<ChipsetDestinationOperandData &>(extra_data);
											uint8_t operand_0_chipset = instruction_data.data[4];
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_chipset_destination_operand_data.memory_group, operand_0_chipset_destination_operand_data.destination_pointer, operand_0_destination_offset_data, instruction_data.data[5], operand_0_chipset_write_return);
											cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
											break;
										}
									}
									break;
								}
								case 1:
								{
									Word_LE operand_0_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											operand_0_destination_offset_data.offset = relative_offset;
											break;
										}
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, operand_0_destination_offset_data, instruction_data.data[5]);
									cycles_processed += 2;
									break;
								}
								case 2:
								{
									Word_LE operand_0_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											operand_0_destination_offset_data.offset = relative_offset;
											break;
										}
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, operand_0_destination_offset_data, instruction_data.data[5]);
									cycles_processed += 2;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_0_region_id = instruction_data.data[4];
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[4]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											struct alignas(8) ImmediateValueToChipsetData
											{
												DWord_LE operand_0_destination_pointer;
												Word_LE operand_0_memory_group;
												Word_LE operand_1_immediate_value;
											};
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											ImmediateValueToChipsetData &immediate_value_to_chipset_data = reinterpret_cast<ImmediateValueToChipsetData &>(extra_data);
											uint8_t operand_0_chipset = instruction_data.data[4];
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											// Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[6]);
											CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, immediate_value_to_chipset_data.operand_0_memory_group, immediate_value_to_chipset_data.operand_0_destination_pointer, operand_0_destination_offset_data, immediate_value_to_chipset_data.operand_1_immediate_value, operand_0_chipset_write_return);
											cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
											break;
										}
									}
									break;
								}
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									Word_LE operand_0_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[4]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									Word_LE operand_0_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[4]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											Word_LE &operand_1_immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
											StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_0_region_id = instruction_data.data[4];
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[4]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
											Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											uint8_t operand_0_chipset = instruction_data.data[4];
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[8]);
											CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_immediate_value, operand_0_chipset_write_return);
											cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
											break;
										}
									}
									break;
								}
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									Word_LE operand_0_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[4]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									Word_LE operand_0_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[4]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWord_LE &operand_1_immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 3:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
											Word_LE operand_0_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_0_region_id = instruction_data.data[4];
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[8]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											struct ImmediateValueToChipsetMemoryGroupData
											{
												DWord_LE operand_0_destination_pointer;
												Word_LE operand_0_memory_group;
												Word_LE unused;
												QWord_LE operand_1_immediate_value;
											} immediate_value_to_chipset_memory_group_data = CurrentCPU.FastFetchExtraData<ImmediateValueToChipsetMemoryGroupData>();
											// ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
											uint8_t operand_0_chipset = instruction_data.data[4];
											OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 2:
												{
													operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
													operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
													break;
												}
											}
											// QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[8]);
											CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, immediate_value_to_chipset_memory_group_data.operand_0_memory_group, immediate_value_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, immediate_value_to_chipset_memory_group_data.operand_1_immediate_value, operand_0_chipset_write_return);
											cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 2 : 1;
											break;
										}
									}
									break;
								}
								case 1:
								{
									Word_LE operand_0_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[8]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
								case 2:
								{
									Word_LE operand_0_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[8]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											QWord_LE &operand_1_immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
											StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, operand_1_immediate_value);
											cycles_processed += 2;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					uint8_t operand_1_source_offset_type = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
					uint8_t operand_1_pointer_type = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(4, 2)) >> 4);
					uint8_t operand_1_pointer_source = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(6, 2)) >> 6);
					switch (operand_0_destination_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.target)
							{
								case 0:
								{
									Word_LE operand_0_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
									uint8_t operand_0_region_id = instruction_data.data[4];
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									switch (operand_1_pointer_type)
									{
										case 0:
										{
											switch (operand_1_pointer_source)
											{
												case 0:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													uint8_t operand_1_region_id = instruction_data.data[5];
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[4]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													break;
												}
												case 2:
												{
													ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													Word_LE &operand_1_memory_group = reinterpret_cast<Word_LE &>(extra_data[12]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[8]);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											Word_LE operand_1_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = relative_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 2:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
											Word_LE operand_1_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &relative_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = relative_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, operand_0_destination_pointer, operand_0_destination_offset_data, data);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									++cycles_processed;
									break;
								}
								case 2:
								{
									OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 2:
										{
											operand_0_destination_offset_data.offset = CurrentCPU.DI.offset;
											operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
											break;
										}
									}
									switch (operand_1_pointer_type)
									{
										case 0:
										{
											uint8_t operand_0_chipset = instruction_data.data[4];
											switch (operand_1_pointer_source)
											{
												case 0:
												{
													struct alignas(8) MemoryToChipsetMemoryGroupData
													{
														DWord_LE operand_0_destination_pointer;
														Word_LE operand_0_memory_group;
														Word_LE unused;
														DWord_LE operand_1_source_pointer;
													} memory_to_chipset_memory_group_data = CurrentCPU.FastFetchExtraData<MemoryToChipsetMemoryGroupData>();
													Word_LE operand_1_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
													uint8_t operand_1_region_id = instruction_data.data[5];
													// DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[8]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, memory_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data);
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, memory_to_chipset_memory_group_data.operand_0_memory_group, memory_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, memory_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data);
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, memory_to_chipset_memory_group_data.operand_0_memory_group, memory_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, memory_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data);
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, memory_to_chipset_memory_group_data.operand_0_memory_group, memory_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, memory_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data);
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, memory_to_chipset_memory_group_data.operand_0_memory_group, memory_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
												case 1:
												{
													++cycles_processed;
													break;
												}
												case 2:
												{
													struct alignas(8) ChipsetMemoryGroupToChipsetMemoryGroupData
													{
														DWord_LE operand_0_destination_pointer;
														Word_LE operand_0_memory_group;
														Word_LE unused;
														DWord_LE operand_1_source_pointer;
														Word_LE operand_1_memory_group;
													} chipset_memory_group_to_chipset_memory_group_data = CurrentCPU.FastFetchExtraData<ChipsetMemoryGroupToChipsetMemoryGroupData>();
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_1_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_0_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
																cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_1_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_0_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
																cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_1_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_0_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
																cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_1_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, chipset_memory_group_to_chipset_memory_group_data.operand_0_memory_group, chipset_memory_group_to_chipset_memory_group_data.operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
																cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											uint8_t operand_0_chipset = instruction_data.data[4];
											Word_LE operand_1_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[8]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
												case 2:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 2:
										{
											uint8_t operand_0_chipset = instruction_data.data[4];
											Word_LE operand_1_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[8]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
												case 2:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_0_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_0_destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(operand_0_chipset, operand_0_memory_group, operand_0_destination_pointer, operand_0_destination_offset_data, data, operand_0_chipset_write_return);
															cycles_processed += (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful) ? 3 : 2;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							Word_LE operand_0_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
							uint8_t operand_0_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
							switch (operand_1_pointer_type)
							{
								case 0:
								{
									switch (operand_1_pointer_source)
									{
										case 0:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											Word_LE operand_1_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = instruction_data.data[5];
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[4]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);

															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_1_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
													DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													Word_LE &operand_1_memory_group = reinterpret_cast<Word_LE &>(extra_data[8]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[4]);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
												case 2:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_1_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													++cycles_processed;
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = operand_1_source_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									Word_LE operand_1_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
									uint8_t operand_1_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = operand_1_source_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.BP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							Word_LE operand_0_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
							uint8_t operand_0_region_id = ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
							switch (operand_1_pointer_type)
							{
								case 0:
								{
									switch (operand_1_pointer_source)
									{
										case 0:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											Word_LE operand_1_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
											uint8_t operand_1_region_id = instruction_data.data[5];
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[4]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data);
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											++cycles_processed;
											break;
										}
										case 2:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													struct alignas(8) ChipsetMemoryGroupToMemoryData
													{
														DWord_LE operand_1_source_pointer;
														Word_LE operand_1_memory_group;
													};
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													ChipsetMemoryGroupToMemoryData &chipset_memory_group_to_memory_data = reinterpret_cast<ChipsetMemoryGroupToMemoryData &>(extra_data);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
												case 1:
												{
													struct alignas(8) ChipsetMemoryGroupToMemoryData
													{
														DWord_LE operand_0_destination_offset;
														DWord_LE operand_1_source_pointer;
														Word_LE operand_1_memory_group;
													};
													ExtraQWordData<2> extra_data = CurrentCPU.FetchExtraData<2>();
													ChipsetMemoryGroupToMemoryData chipset_memory_group_to_memory_data = reinterpret_cast<ChipsetMemoryGroupToMemoryData &>(extra_data);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { chipset_memory_group_to_memory_data.operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { chipset_memory_group_to_memory_data.operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { chipset_memory_group_to_memory_data.operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, chipset_memory_group_to_memory_data.operand_1_memory_group, chipset_memory_group_to_memory_data.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { chipset_memory_group_to_memory_data.operand_0_destination_offset, IndexRegisterType::None }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
												case 2:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													Word_LE &operand_1_memory_group = reinterpret_cast<Word_LE &>(extra_data[4]);
													DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
													uint8_t operand_1_chipset = instruction_data.data[5];
													OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
													switch (operand_1_source_offset_type)
													{
														case 2:
														{
															operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
															operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
															break;
														}
													}
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 1:
														{
															Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 2:
														{
															DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
														case 3:
														{
															QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_memory_group, operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
															if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
															{
																StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
																cycles_processed += 3;
															}
															else
															{
																cycles_processed += 2;
															}
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									Word_LE operand_1_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
									uint8_t operand_1_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = operand_1_source_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_1_program_id, operand_1_region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													++cycles_processed;
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
											DWord_LE &operand_0_destination_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
											OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
											switch (operand_1_source_offset_type)
											{
												case 1:
												{
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[4]);
													operand_1_source_offset_data.offset = operand_1_source_offset;
													break;
												}
												case 2:
												{
													operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
													operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
													break;
												}
											}
											switch (data_size)
											{
												case 0:
												{
													uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 1:
												{
													Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 2:
												{
													DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
												case 3:
												{
													QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, operand_1_source_offset_data);
													StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_0_destination_offset, IndexRegisterType::None }, data);
													cycles_processed += 3;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_offset_type)
											{
												case 0:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 1:
												{
													ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
													DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
												case 2:
												{
													switch (data_size)
													{
														case 0:
														{
															uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<uint8_t>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 1:
														{
															Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<Word_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 2:
														{
															DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<DWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
														case 3:
														{
															QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
															StoreDataToSystemMemory<QWord_LE>(CurrentCPU, operand_0_program_id, operand_0_region_id, CurrentCPU.SP.address, { CurrentCPU.DI.offset, IndexRegisterType::Destination }, data);
															cycles_processed += 3;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		std::array<uint8_t, 2> data;
	} &move_instruction_base = reinterpret_cast<MoveInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type_0 = (move_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(0, 1));
	uint8_t operand_type_1 = (move_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((move_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	switch (operand_type_0)
	{
		case 0:
		{
			bool register_field_mode = (move_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
			uint8_t &operand_0_register = move_instruction_base.data[0];
			uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
			switch (operand_type_1)
			{
				case 0:
				{
					uint8_t &operand_1_register = move_instruction_base.data[1];
					uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
					switch (operand_0_register)
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
							switch (operand_1_register)
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
									GPRegisterToGPRegisterTransfer(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x10:
								{
									IndexRegisterToGPRegisterTransfer(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x11:
								{
									IndexRegisterToGPRegisterTransfer(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x20:
								{
									MPRegisterToGPRegisterTransfer(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x21:
								{
									MPRegisterToGPRegisterTransfer(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (operand_1_register)
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
									GPRegisterToIndexRegisterTransfer(data.CurrentCPU.SI, data.CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x10:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x11:
								{
									IndexRegisterToIndexRegisterTransfer(data.CurrentCPU.SI, data.CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x20:
								{
									MPRegisterToIndexRegisterTransfer(data.CurrentCPU.SI, data.CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x21:
								{
									MPRegisterToIndexRegisterTransfer(data.CurrentCPU.SI, data.CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (operand_1_register)
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
									GPRegisterToIndexRegisterTransfer(data.CurrentCPU.DI, data.CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x10:
								{
									IndexRegisterToIndexRegisterTransfer(data.CurrentCPU.DI, data.CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x11:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x20:
								{
									MPRegisterToIndexRegisterTransfer(data.CurrentCPU.DI, data.CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x21:
								{
									MPRegisterToIndexRegisterTransfer(data.CurrentCPU.DI, data.CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (operand_1_register)
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
									GPRegisterToMPRegisterTransfer(data.CurrentCPU.BP, data.CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x10:
								{
									IndexRegisterToMPRegisterTransfer(data.CurrentCPU.BP, data.CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x11:
								{
									IndexRegisterToMPRegisterTransfer(data.CurrentCPU.BP, data.CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									data.callback = nullptr;
									break;
								}
								case 0x20:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x21:
								{
									MPRegisterToMPRegisterTransfer(data.CurrentCPU.BP, data.CurrentCPU.SP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (operand_1_register)
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
									GPRegisterToMPRegisterTransfer(data.CurrentCPU.SP, data.CurrentCPU.GPR_Registers[operand_1_register], data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x10:
								{
									IndexRegisterToMPRegisterTransfer(data.CurrentCPU.SP, data.CurrentCPU.SI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x11:
								{
									IndexRegisterToMPRegisterTransfer(data.CurrentCPU.SP, data.CurrentCPU.DI, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x20:
								{
									MPRegisterToMPRegisterTransfer(data.CurrentCPU.SP, data.CurrentCPU.BP, data_size, operand_0_field_index, operand_1_field_index);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 0x21:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (operand_0_register)
					{
						case 0x00:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x00]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x00>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x00>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x01:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x01]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x01>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x01>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x02:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x02]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x02>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x02>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x03:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x03]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x03>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x03>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x04:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x04]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x04>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x04>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x05:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x05]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x05>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x05>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x06:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x06]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x06>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x06>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x07:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0x07]);
									register_field[operand_0_field_index] = move_instruction_base.data[1];
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, WordField, 0x07>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x07>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (data_size)
							{
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x10>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (data_size)
							{
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x11>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (data_size)
							{
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x20>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (data_size)
							{
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, DWordField, 0x21>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					PointerControlData operand_1_source_pointer_control = GetPointerControlData<2, 4, 6>(move_instruction_base.operand_control_1);
					switch (operand_1_source_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_1_source_pointer_control.target)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Register_ExecuteCycle;
									break;
								}
								case 1:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
									constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
													break;
												}
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										case 0x11:
										case 0x20:
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_To_Register_Relative_Offset_ExecuteCycle;
									break;
								}
								case 2:
								{
									ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
									OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
													break;
												}
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										case 0x11:
										case 0x20:
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
									constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
													break;
												}
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										case 0x11:
										case 0x20:
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_To_Register_Relative_Offset_ExecuteCycle;
									break;
								}
								case 2:
								{
									ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
									OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
													break;
												}
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										case 0x11:
										case 0x20:
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
													break;
												}
												case 3:
												{
													data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			PointerControlData operand_0_destination_pointer_control = GetPointerControlData<5, 7, 9>(move_instruction_base.operand_control_0);
			switch (operand_type_1)
			{
				case 0:
				{
					bool register_field_mode = (move_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
					uint8_t &operand_1_register = move_instruction_base.data[0];
					uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
					switch (operand_0_destination_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.target)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Register_To_Absolute_Pointer_Self_ExecuteCycle;
									break;
								}
								case 1:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Register_To_Absolute_Pointer_Chipset_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (operand_0_destination_pointer_control.offset_type)
							{
								case 0:
								{
									ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
									constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[operand_1_register]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Register_To_Base_Pointer_Relative_Offset_ExecuteCycle;
									break;
								}
								case 2:
								{
									ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
									OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[operand_1_register]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (operand_0_destination_pointer_control.offset_type)
							{
								case 0:
								{
									ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
									constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[operand_1_register]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Register_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
									break;
								}
								case 2:
								{
									ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
									OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
									switch (operand_1_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 1:
												{
													WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[operand_1_register]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
													StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
												case 3:
												{
													StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (data_size)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Byte_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle;
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Byte_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle;
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
											constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_base.data[1]);
											data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Byte_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle;
											break;
										}
										case 2:
										{
											ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
											OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
											StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_base.data[1]);
											data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
											constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
											StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_base.data[1]);
											data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Byte_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
											break;
										}
										case 2:
										{
											ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
											OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
											StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_base.data[1]);
											data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Word_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle;
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>;
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											struct ExtraInstructionData
											{
												QWord_LE data_1;
												QWord_LE data_2;
											} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
											extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
											data.callback = Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>;
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
										case 1:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 3:
						{
							switch (operand_0_destination_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_0_destination_pointer_control.target)
									{
										case 0:
										{
											struct ExtraInstructionData
											{
												QWord_LE data_1;
												QWord_LE data_2;
											} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
											extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
											data.callback = QWord_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle;
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											struct ExtraInstructionData
											{
												QWord_LE data_1;
												QWord_LE data_2;
											} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
											extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
											data.callback = Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>;
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>;
											break;
										}
										case 1:
										{
											struct ExtraInstructionData
											{
												QWord_LE data_1;
												QWord_LE data_2;
											} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
											extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
											data.callback = QWord_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>;
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>;
											break;
										}
										case 1:
										{
											struct ExtraInstructionData
											{
												QWord_LE data_1;
												QWord_LE data_2;
											} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
											extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
											data.callback = QWord_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
											break;
										}
										case 2:
										{
											data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
											data.callback = Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					PointerControlData operand_1_source_pointer_control = GetPointerControlData<2, 4, 6>(move_instruction_base.operand_control_1);
					switch (operand_0_destination_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_0_destination_pointer_control.target)
							{
								case 0:
								{
									switch (operand_1_source_pointer_control.pointer_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.target)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
													data.callback = nullptr;
													break;
												}
												case 2:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													switch (data_size)
													{
														case 0:
														{
															data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<uint8_t>;
															break;
														}
														case 1:
														{
															data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<Word_LE>;
															break;
														}
														case 2:
														{
															data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<DWord_LE>;
															break;
														}
														case 3:
														{
															data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle<QWord_LE>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 2:
								{
									switch (operand_1_source_pointer_control.pointer_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.target)
											{
												case 0:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
												case 1:
												{
													// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
													data.callback = nullptr;
													break;
												}
												case 2:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
												case 1:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
												case 1:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_0_destination_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (operand_1_source_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_1_source_pointer_control.target)
									{
										case 0:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
											}
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 1:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
													data.callback = nullptr;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (operand_1_source_pointer_control.pointer_type)
							{
								case 0:
								{
									switch (operand_1_source_pointer_control.target)
									{
										case 0:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Self_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
											}
											break;
										}
										case 1:
										{
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											switch (operand_0_destination_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 1:
												{
													struct ExtraInstructionData
													{
														QWord_LE data_1;
														QWord_LE data_2;
													} &extra_instruction_data = reinterpret_cast<ExtraInstructionData &>(data.instruction_data[1]);
													extra_instruction_data = data.CurrentCPU.FastFetchExtraData<ExtraInstructionData>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(operand_1_source_pointer_control.offset_type);
													data.current_data.push_back(data_size);
													data.callback = Absolute_Pointer_Chipset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Base_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_destination_pointer_control.offset_type)
									{
										case 0:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
													data.callback = nullptr;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
										case 1:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
												case 2:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle;
													break;
												}
											}
											break;
										}
										case 2:
										{
											switch (operand_1_source_pointer_control.offset_type)
											{
												case 0:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
												case 1:
												{
													data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
													data.current_data.push_back(data_size);
													data.callback = Stack_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>, Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
													break;
												}
												case 2:
												{
													ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
													OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
													switch (data_size)
													{
														case 0:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>;
															break;
														}
														case 1:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>;
															break;
														}
														case 2:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>;
															break;
														}
														case 3:
														{
															data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
															data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>;
															break;
														}
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::WordMinimumRequired T, ClassicVCom_Nova64::QWordAlignmentRequired T2, uint8_t operand_0_register>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToRegisterFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register_data;
		uint8_t unused;
		T immediate_value;
	} &move_instruction_immediate_value_to_register_field = reinterpret_cast<MoveInstructionImmediateValueToRegisterFieldData &>(data.instruction_data[0]);
	bool register_field_mode = (move_instruction_immediate_value_to_register_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_0_field_index = register_field_mode ? operand_0_field_index = ((move_instruction_immediate_value_to_register_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
	if (operand_0_register >= 0x00 && operand_0_register <= 0x07)
	{
		T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
		register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
	}
	else if (operand_0_register == 0x10)
	{
		T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SI);
		register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
	}
	else if (operand_0_register == 0x11)
	{
		T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.DI);
		register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
	}
	else if (operand_0_register == 0x20)
	{
		T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.BP);
		register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
	}
	else if (operand_0_register == 0x21)
	{
		T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SP);
		register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
	}
	/*
	switch (move_instruction_immediate_value_to_register_field.operand_0_register)
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
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.GPR_Registers[move_instruction_immediate_value_to_register_field.operand_0_register]);
			register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
			break;
		}
		case 0x10:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SI);
			register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
			break;
		}
		case 0x11:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.DI);
			register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
			break;
		}
		case 0x20:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.BP);
			register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
			break;
		}
		case 0x21:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SP);
			register_field[operand_0_field_index] = move_instruction_immediate_value_to_register_field.immediate_value;
			break;
		}
	}
	*/
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionQWordImmediateValueToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
		uint8_t unused;
		QWord_LE immediate_value;
	} &move_instruction_qword_immediate_value_to_register = reinterpret_cast<MoveInstructionQWordImmediateValueToRegisterData &>(data.instruction_data[0]);
	switch (move_instruction_qword_immediate_value_to_register.operand_0_register)
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
			data.CurrentCPU.GPR_Registers[move_instruction_qword_immediate_value_to_register.operand_0_register] = move_instruction_qword_immediate_value_to_register.immediate_value;
			break;
		}
		case 0x10:
		{
			data.CurrentCPU.SI = std::bit_cast<IndexRegisterData>(move_instruction_qword_immediate_value_to_register.immediate_value);
			break;
		}
		case 0x11:
		{
			data.CurrentCPU.DI = std::bit_cast<IndexRegisterData>(move_instruction_qword_immediate_value_to_register.immediate_value);
			break;
		}
		case 0x20:
		{
			data.CurrentCPU.BP = std::bit_cast<MPRegisterData>(move_instruction_qword_immediate_value_to_register.immediate_value);
			break;
		}
		case 0x21:
		{
			data.CurrentCPU.SP = std::bit_cast<MPRegisterData>(move_instruction_qword_immediate_value_to_register.immediate_value);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
		uint8_t operand_1_region_id;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_register = reinterpret_cast<MoveInstructionAbsolutePointerSelfToRegisterData &>(data.instruction_data[0]);
	bool register_field_mode = (move_instruction_absolute_pointer_self_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t data_size = ((move_instruction_absolute_pointer_self_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_absolute_pointer_self_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
	uint8_t operand_1_offset_type = ((move_instruction_absolute_pointer_self_to_register.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (move_instruction_absolute_pointer_self_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
					break;
				}
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		case 0x11:
		case 0x20:
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_register.operand_1_region_id, move_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T, ClassicVCom_Nova64::QWordAlignmentRequired T2>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionPointerDataToRegisterFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
	} &move_instruction_pointer_data_to_register_field = reinterpret_cast<MoveInstructionPointerDataToRegisterFieldData &>(data.instruction_data[0]);
	bool register_field_mode = (move_instruction_pointer_data_to_register_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_pointer_data_to_register_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
	switch (move_instruction_pointer_data_to_register_field.operand_0_register)
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
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.GPR_Registers[move_instruction_pointer_data_to_register_field.operand_0_register]);
			register_field[operand_0_field_index] = data.CurrentCPU.data_bus;
			break;
		}
		case 0x10:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SI);
			register_field[operand_0_field_index] = data.CurrentCPU.data_bus;
			break;
		}
		case 0x11:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.DI);
			register_field[operand_0_field_index] = data.CurrentCPU.data_bus;
			break;
		}
		case 0x20:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.BP);
			register_field[operand_0_field_index] = data.CurrentCPU.data_bus;
			break;
		}
		case 0x21:
		{
			T2 &register_field = reinterpret_cast<T2 &>(data.CurrentCPU.SP);
			register_field[operand_0_field_index] = data.CurrentCPU.data_bus;
			break;
		}
	}
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionQWordPointerDataToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
	} &move_instruction_qword_pointer_data_to_register = reinterpret_cast<MoveInstructionQWordPointerDataToRegisterData &>(data.instruction_data[0]);
	switch (move_instruction_qword_pointer_data_to_register.operand_0_register)
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
			data.CurrentCPU.GPR_Registers[move_instruction_qword_pointer_data_to_register.operand_0_register] = data.CurrentCPU.data_bus;
			break;
		}
		case 0x10:
		{
			data.CurrentCPU.SI = std::bit_cast<IndexRegisterData>(data.CurrentCPU.data_bus);
			break;
		}
		case 0x11:
		{
			data.CurrentCPU.DI = std::bit_cast<IndexRegisterData>(data.CurrentCPU.data_bus);
			break;
		}
		case 0x20:
		{
			data.CurrentCPU.BP = std::bit_cast<MPRegisterData>(data.CurrentCPU.data_bus);
			break;
		}
		case 0x21:
		{
			data.CurrentCPU.SP = std::bit_cast<MPRegisterData>(data.CurrentCPU.data_bus);
			break;
		}
	}
	data.CurrentCPU.data_bus = 0;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
		uint8_t operand_1_chipset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_register = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToRegisterData &>(data.instruction_data[0]);
	bool register_field_mode = (move_instruction_absolute_pointer_chipset_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t data_size = ((move_instruction_absolute_pointer_chipset_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_absolute_pointer_chipset_to_register.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
	uint8_t operand_1_offset_type = ((move_instruction_absolute_pointer_chipset_to_register.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (move_instruction_absolute_pointer_chipset_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.CurrentCPU.data_bus = value;
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 1:
				{
					Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.CurrentCPU.data_bus = value;
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 2:
				{
					DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.CurrentCPU.data_bus = value;
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 3:
				{
					QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.CurrentCPU.data_bus = value;
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerToRegisterRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
		uint8_t unused;
		DWord_LE relative_offset;
	} &move_instruction_base_pointer_to_register_relative_offset = reinterpret_cast<MoveInstructionBasePointerToRegisterRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_base_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_base_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_base_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) >> 5) : 0;
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_to_register_relative_offset.relative_offset, IndexRegisterType::None };
	switch (move_instruction_base_pointer_to_register_relative_offset.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
					break;
				}
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		case 0x11:
		case 0x20:
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_To_Register_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerToRegisterRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_register;
		uint8_t unused;
		DWord_LE relative_offset;
	} &move_instruction_stack_pointer_to_register_relative_offset = reinterpret_cast<MoveInstructionStackPointerToRegisterRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_stack_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_stack_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_0_field_index = register_field_mode ? ((move_instruction_stack_pointer_to_register_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 4)) > 5) : 0;
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_to_register_relative_offset.relative_offset, IndexRegisterType::None };
	switch (move_instruction_stack_pointer_to_register_relative_offset.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, WordField>>;
					break;
				}
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		case 0x11:
		case 0x20:
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, DWordField>>;
					break;
				}
				case 3:
				{
					data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Register_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionRegisterToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t operand_1_register;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_register_to_absolute_pointer_self = reinterpret_cast<MoveInstructionRegisterToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_register_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_register_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_register_to_absolute_pointer_self.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
	uint8_t operand_0_offset_type = ((move_instruction_register_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	switch (move_instruction_register_to_absolute_pointer_self.operand_1_register)
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
			switch (data_size)
			{
				case 0:
				{
					ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_self.operand_1_register]);
					StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 1:
				{
					WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_self.operand_1_register]);
					StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_self.operand_1_register]);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_self.operand_1_register]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_register_to_absolute_pointer_self.operand_0_region_id, move_instruction_register_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Register_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionRegisterToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t operand_1_register;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_register_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionRegisterToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_register_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_register_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_register_to_absolute_pointer_chipset.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
	uint8_t operand_0_offset_type = ((move_instruction_register_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	switch (move_instruction_register_to_absolute_pointer_chipset.operand_1_register)
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
			switch (data_size)
			{
				case 0:
				{
					ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_chipset.operand_1_register]);
					data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(move_instruction_register_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_register_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_register_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
					if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
					{
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 1:
				{
					WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_chipset.operand_1_register]);
					data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(move_instruction_register_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_register_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_register_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
					if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
					{
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_chipset.operand_1_register]);
					data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(move_instruction_register_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_register_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_register_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index], operand_0_chipset_write_return);
					if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
					{
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 3:
				{
					data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(move_instruction_register_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_register_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_register_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[move_instruction_register_to_absolute_pointer_chipset.operand_1_register], operand_0_chipset_write_return);
					if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
					{
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Register_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionRegisterToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_register;
		DWord_LE relative_offset;
	} &move_instruction_register_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionRegisterToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_register_to_base_pointer_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_register_to_base_pointer_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_register_to_base_pointer_relative_offset.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { move_instruction_register_to_base_pointer_relative_offset.relative_offset, IndexRegisterType::None };
	switch (move_instruction_register_to_base_pointer_relative_offset.operand_1_register)
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
			switch (data_size)
			{
				case 0:
				{
					ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_base_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 1:
				{
					WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_base_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_base_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[move_instruction_register_to_base_pointer_relative_offset.operand_1_register]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Register_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionRegisterToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_register;
		DWord_LE relative_offset;
	} &move_instruction_register_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionRegisterToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_register_to_stack_pointer_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (move_instruction_register_to_stack_pointer_relative_offset.operand_control_0 & GenerateFieldBitmask<uint16_t>(4, 1));
	uint8_t operand_1_field_index = register_field_mode ? ((move_instruction_register_to_stack_pointer_relative_offset.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2) : 0;
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { move_instruction_register_to_stack_pointer_relative_offset.relative_offset, IndexRegisterType::None };
	switch (move_instruction_register_to_stack_pointer_relative_offset.operand_1_register)
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
			switch (data_size)
			{
				case 0:
				{
					ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_stack_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 1:
				{
					WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_stack_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<Word_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[move_instruction_register_to_stack_pointer_relative_offset.operand_1_register]);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, data.CurrentCPU.GPR_Registers[move_instruction_register_to_stack_pointer_relative_offset.operand_1_register]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.DI));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.BP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
					StoreDataToSystemMemory<DWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, operand_1_register_field[operand_1_field_index]);
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
				case 3:
				{
					StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, std::bit_cast<QWord_LE>(data.CurrentCPU.SP));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Byte_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionByteImmediateValueToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t immediate_value;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_byte_immediate_value_to_absolute_pointer_self = reinterpret_cast<MoveInstructionByteImmediateValueToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t operand_0_offset_type = ((move_instruction_byte_immediate_value_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_byte_immediate_value_to_absolute_pointer_self.operand_0_region_id, move_instruction_byte_immediate_value_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_byte_immediate_value_to_absolute_pointer_self.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Byte_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionByteImmediateValueToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t immediate_value;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_byte_immediate_value_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionByteImmediateValueToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t operand_0_offset_type = ((move_instruction_byte_immediate_value_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(move_instruction_byte_immediate_value_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_byte_immediate_value_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_byte_immediate_value_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_byte_immediate_value_to_absolute_pointer_chipset.immediate_value, operand_0_chipset_write_return);
	if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Byte_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionByteImmediateValueToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t immediate_value;
		DWord_LE operand_0_relative_offset;
	} &move_instruction_byte_immediate_value_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionByteImmediateValueToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { move_instruction_byte_immediate_value_to_base_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_byte_immediate_value_to_base_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Byte_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionByteImmediateValueToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t immediate_value;
		DWord_LE operand_0_relative_offset;
	} &move_instruction_byte_immediate_value_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionByteImmediateValueToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { move_instruction_byte_immediate_value_to_stack_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<uint8_t>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_byte_immediate_value_to_stack_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <typename T> requires ClassicVCom_Nova64::WordMinimumRequired<T> && ClassicVCom_Nova64::DWordMaximumRequired<T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		T immediate_value;
	} &move_instruction_immediate_value_to_absolute_pointer_self = reinterpret_cast<MoveInstructionImmediateValueToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t operand_0_offset_type = ((move_instruction_immediate_value_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 2);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, move_instruction_immediate_value_to_absolute_pointer_self.operand_0_region_id, move_instruction_immediate_value_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_immediate_value_to_absolute_pointer_self.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::QWord_Immediate_Value_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionQWordImmediateValueToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused_1;
		DWord_LE operand_0_destination_pointer;
		DWord_LE unused_2;
		QWord_LE immediate_value;
	} &move_instruction_qword_immediate_value_to_absolute_pointer_self = reinterpret_cast<MoveInstructionQWordImmediateValueToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t operand_0_offset_type = ((move_instruction_qword_immediate_value_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_qword_immediate_value_to_absolute_pointer_self.operand_0_region_id, move_instruction_qword_immediate_value_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_qword_immediate_value_to_absolute_pointer_self.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Word_Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionWordImmediateValueToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE immediate_value;
	} &move_instruction_word_immediate_value_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionWordImmediateValueToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t offset_type = ((move_instruction_word_immediate_value_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(move_instruction_word_immediate_value_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_word_immediate_value_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_word_immediate_value_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_word_immediate_value_to_absolute_pointer_chipset.immediate_value, operand_0_chipset_write_return);
	if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <typename T> requires ClassicVCom_Nova64::DWordMinimumRequired<T> && ClassicVCom_Nova64::QWordMaximumRequired<T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused_1;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE unused_2;
		T immediate_value;
	} &move_instruction_immediate_value_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionImmediateValueToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t offset_type = ((move_instruction_immediate_value_to_absolute_pointer_chipset.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 2);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<T>(move_instruction_immediate_value_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_immediate_value_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_immediate_value_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, move_instruction_immediate_value_to_absolute_pointer_chipset.immediate_value, operand_0_chipset_write_return);
	if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::WordMinimumRequired T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToBasePointerNoOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &move_instruction_immediate_value_to_base_pointer_no_offset = reinterpret_cast<MoveInstructionImmediateValueToBasePointerNoOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_base_pointer_no_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <typename T> requires ClassicVCom_Nova64::WordMinimumRequired<T> && ClassicVCom_Nova64::DWordMaximumRequired<T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		T immediate_value;
	} &move_instruction_immediate_value_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionImmediateValueToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { move_instruction_immediate_value_to_base_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_base_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::QWord_Immediate_Value_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionQWordImmediateValueToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused_1;
		DWord_LE operand_0_relative_offset;
		DWord_LE unused_2;
		QWord_LE immediate_value;
	} &move_instruction_qword_immediate_value_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionQWordImmediateValueToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { move_instruction_qword_immediate_value_to_base_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_qword_immediate_value_to_base_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::WordMinimumRequired T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToBasePointerDestinationIndexRegisterOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &move_instruction_immediate_value_to_base_pointer_destination_index_register_offset = reinterpret_cast<MoveInstructionImmediateValueToBasePointerDestinationIndexRegisterOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_base_pointer_destination_index_register_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::WordMinimumRequired T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToStackPointerNoOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &move_instruction_immediate_value_to_stack_pointer_no_offset = reinterpret_cast<MoveInstructionImmediateValueToStackPointerNoOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_stack_pointer_no_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <typename T> requires ClassicVCom_Nova64::WordMinimumRequired<T> && ClassicVCom_Nova64::DWordMaximumRequired<T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		T immediate_value;
	} &move_instruction_immediate_value_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionImmediateValueToStackPointerRelativeOffsetData &>(data.instruction_data);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { move_instruction_immediate_value_to_stack_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_stack_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::QWord_Immediate_Value_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionQWordImmediateValueToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused_1;
		DWord_LE operand_0_relative_offset;
		DWord_LE unused_2;
		QWord_LE immediate_value;
	} &move_instruction_qword_immediate_value_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionQWordImmediateValueToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { move_instruction_qword_immediate_value_to_stack_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<QWord_LE>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_qword_immediate_value_to_stack_pointer_relative_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::WordMinimumRequired T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Immediate_Value_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionImmediateValueToStackPointerDestinationIndexRegisterOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &move_instruction_immediate_value_to_stack_pointer_destination_index_register_offset = reinterpret_cast<MoveInstructionImmediateValueToStackPointerDestinationIndexRegisterOffsetData &>(data.instruction_data[0]);
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, move_instruction_immediate_value_to_stack_pointer_destination_index_register_offset.immediate_value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t operand_1_region_id;
		DWord_LE operand_0_destination_pointer;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_absolute_pointer_self = reinterpret_cast<MoveInstructionAbsolutePointerSelfToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t data_size = ((move_instruction_absolute_pointer_self_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	uint8_t operand_1_offset_type = ((move_instruction_absolute_pointer_self_to_absolute_pointer_self.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_absolute_pointer_self.operand_1_region_id, move_instruction_absolute_pointer_self_to_absolute_pointer_self.operand_1_source_pointer, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionPointerDataToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_pointer_data_to_absolute_pointer_self = reinterpret_cast<MoveInstructionPointerDataToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	uint8_t operand_0_offset_type = ((move_instruction_pointer_data_to_absolute_pointer_self.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, move_instruction_pointer_data_to_absolute_pointer_self.operand_0_region_id, move_instruction_pointer_data_to_absolute_pointer_self.operand_0_destination_pointer, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t operand_1_chipset;
		DWord_LE operand_0_destination_pointer;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t operand_1_offset_type = ((move_instruction_absolute_pointer_chipset_to_absolute_pointer_self.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	T value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<T>(move_instruction_absolute_pointer_chipset_to_absolute_pointer_self.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_absolute_pointer_self.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_absolute_pointer_self.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
	if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerNoOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_base_pointer_no_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionBasePointerNoOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_absolute_pointer_self.operand_1_relative_offset, IndexRegisterType::None };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerSourceIndexRegisterOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_base_pointer_source_index_register_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionBasePointerSourceIndexRegisterOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerNoOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_stack_pointer_no_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionStackPointerNoOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_absolute_pointer_self.operand_1_relative_offset, IndexRegisterType::None };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerSourceIndexRegisterOffsetToAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_region_id;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
	} &move_instruction_stack_pointer_source_index_register_offset_to_absolute_pointer_self = reinterpret_cast<MoveInstructionStackPointerSourceIndexRegisterOffsetToAbsolutePointerSelfData &>(data.instruction_data[0]);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Self_ExecuteCycle<T>>;
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t operand_1_region_id;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE unused;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionAbsolutePointerSelfToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_region_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_region_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_region_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_region_id, move_instruction_absolute_pointer_self_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionPointerDataToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_pointer_data_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionPointerDataToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	T value = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_0_chipset_write_return = ChipsetReturnCode::Ok;
	switch (operand_0_offset_type)
	{
		case 2:
		{
			operand_0_destination_offset_data.offset = data.CurrentCPU.DI.offset;
			operand_0_destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<T>(move_instruction_pointer_data_to_absolute_pointer_chipset.operand_0_chipset, move_instruction_pointer_data_to_absolute_pointer_chipset.operand_0_memory_group, move_instruction_pointer_data_to_absolute_pointer_chipset.operand_0_destination_pointer, operand_0_destination_offset_data, value, operand_0_chipset_write_return);
	if (operand_0_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t operand_1_chipset;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE unused;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			}
			else
			{
				data.current_data.pop_back();
				data.current_data.pop_back();
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 1:
		{
			Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			}
			else
			{
				data.current_data.pop_back();
				data.current_data.pop_back();
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 2:
		{
			DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			}
			else
			{
				data.current_data.pop_back();
				data.current_data.pop_back();
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 3:
		{
			QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_absolute_pointer_chipset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			}
			else
			{
				data.current_data.pop_back();
				data.current_data.pop_back();
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerNoOffsetToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_base_pointer_no_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionBasePointerNoOffsetToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused_1;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE unused_2;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_absolute_pointer_chipset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerSourceIndexRegisterToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_base_pointer_source_index_register_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionBasePointerSourceIndexRegisterToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerNoOffsetToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_stack_pointer_no_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionStackPointerNoOffsetToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused_1;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
		Word_LE unused_2;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_absolute_pointer_chipset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Absolute_Pointer_Chipset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerSourceIndexRegisterToAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t operand_0_chipset;
		uint8_t unused;
		DWord_LE operand_0_destination_pointer;
		Word_LE operand_0_memory_group;
	} &move_instruction_stack_pointer_source_index_register_offset_to_absolute_pointer_chipset = reinterpret_cast<MoveInstructionStackPointerSourceIndexRegisterToAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Absolute_Pointer_Chipset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToBasePointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_region_id;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_base_pointer = reinterpret_cast<MoveInstructionAbsolutePointerSelfToBasePointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_region_id;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionAbsolutePointerSelfToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionPointerDataToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
	} &move_instruction_pointer_data_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionPointerDataToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { move_instruction_pointer_data_to_base_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;

}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &data), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &data), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &data), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &data)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToBasePointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_chipset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_base_pointer = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToBasePointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 1:
		{
			Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 2:
		{
			DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 3:
		{
			QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_chipset;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 1:
		{
			Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 2:
		{
			DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 3:
		{
			QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_base_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToBasePointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_base_pointer = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToBasePointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_base_pointer.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_base_pointer_relative_offset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Base_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToBasePointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_base_pointer = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToBasePointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_base_pointer.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_base_pointer_relative_offset = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_base_pointer_relative_offset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToStackPointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_region_id;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_stack_pointer = reinterpret_cast<MoveInstructionAbsolutePointerSelfToStackPointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Self_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerSelfToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_region_id;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_source_pointer;
	} &move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionAbsolutePointerSelfToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_region_id, move_instruction_absolute_pointer_self_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_0_destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionPointerDataToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
	} &move_instruction_pointer_data_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionPointerDataToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { move_instruction_pointer_data_to_stack_pointer_relative_offset.operand_0_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Pointer_Data_To_Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_0_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_0_destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, operand_0_program_memory_control.program_id, operand_0_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_0_destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToStackPointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_chipset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_stack_pointer = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToStackPointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 1:
		{
			Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 2:
		{
			DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 3:
		{
			QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Absolute_Pointer_Chipset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionAbsolutePointerChipsetToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t unused;
		uint8_t operand_1_chipset;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionAbsolutePointerChipsetToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (data_size)
	{
		case 0:
		{
			uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 1:
		{
			Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 2:
		{
			DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
		case 3:
		{
			QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_chipset, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_memory_group, move_instruction_absolute_pointer_chipset_to_stack_pointer_relative_offset.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
			if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
			{
				data.current_data.push_back(value);
				data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			}
			else
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToStackPointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_stack_pointer = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToStackPointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_stack_pointer.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionBasePointerRelativeOffsetToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_base_pointer_relative_offset_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionBasePointerRelativeOffsetToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { move_instruction_base_pointer_relative_offset_to_stack_pointer_relative_offset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Base_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData operand_1_source_offset_data = { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

template <void (*byte_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*word_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*dword_callback)(ClassicVCom_Nova64::InstructionCallbackData &), void (*qword_callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Stack_Pointer_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToStackPointerData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_stack_pointer = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToStackPointerData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_stack_pointer.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<byte_callback>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<word_callback>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<dword_callback>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<qword_callback>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_No_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Relative_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) MoveInstructionStackPointerRelativeOffsetToStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE operand_0_relative_offset;
		DWord_LE operand_1_relative_offset;
	} &move_instruction_stack_pointer_relative_offset_to_stack_pointer_relative_offset = reinterpret_cast<MoveInstructionStackPointerRelativeOffsetToStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data = { move_instruction_stack_pointer_relative_offset_to_stack_pointer_relative_offset.operand_1_relative_offset, IndexRegisterType::None };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::MoveInstruction::Stack_Pointer_Source_Index_Register_Offset_To_Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	ProgramMemoryControlData operand_1_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData operand_1_source_offset_data { data.CurrentCPU.SI.offset, IndexRegisterType::Source };
	switch (data_size)
	{
		case 0:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>;
			break;
		}
		case 1:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>;
			break;
		}
		case 2:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>;
			break;
		}
		case 3:
		{
			data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, operand_1_program_memory_control.program_id, operand_1_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>;
			break;
		}
	}
}

/*
void ClassicVCom_Nova64::Instruction::Compare(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	struct OperandControlData
	{
		Word_LE o_0, o_1;
		
		Word_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return o_0; }
				case 1: { return o_1; }
			}
			return o_0;
		}
	};
	struct OperandTypeData
	{
		uint8_t o_0, o_1;

		uint8_t &operator[](int index)
		{
			switch (index)
			{
				case 0: { return o_0; }
				case 1: { return o_1; }
			}
			return o_0;
		}
	};
	OperandControlData &operand_control = reinterpret_cast<OperandControlData &>(instruction_data.data[0]);
	OperandTypeData operand_type = { static_cast<uint8_t>(operand_control[0] & GenerateFieldBitmask<uint16_t>(0, 1)), static_cast<uint8_t>(operand_control[1] & GenerateFieldBitmask<uint16_t>(0, 2)) };
	uint8_t data_size = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(4, 1));
	bool signed_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(5, 1));
	CurrentCPU.FL &= ~(0xE0);
	switch (operand_type[0])
	{
		case 0:
		{
			uint8_t operand_0_register = instruction_data.data[4];
			uint8_t operand_0_field_index = 0;
			if (register_field_mode)
			{
				operand_0_field_index = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(6, 4)) >> 6);
			}
			switch (operand_type[1])
			{
				case 0:
				{
					uint8_t operand_1_register = instruction_data.data[5];
					uint8_t operand_1_field_index = 0;
					if (register_field_mode)
					{
						operand_1_field_index = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2);
					}
					switch (operand_0_register)
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
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
										case 1:
										{
											WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											WordField &operand_1_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
										case 3:
										{
											SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], CurrentCPU.GPR_Registers[operand_1_register], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												CurrentCPU.FL |= 0x20;
											}
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												CurrentCPU.FL |= 0x20;
											}
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												CurrentCPU.FL |= 0x20;
											}
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
										}
										break;
									}
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												++cycles_processed;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], CurrentCPU.FL, signed_mode);
											++cycles_processed;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												CurrentCPU.FL |= 0x20;
											}
											++cycles_processed;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (operand_0_register)
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
							switch (data_size)
							{
								case 0:
								{
									ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
									SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], instruction_data.data[5], CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
									Word_LE &immediate_value = reinterpret_cast<Word_LE &>(extra_data[0]);
									SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
								case 3:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], std::bit_cast<QWord_LE>(extra_data), CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									if (operand_0_field_index)
									{
										++cycles_processed;
										break;
									}
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									if (operand_0_field_index)
									{
										++cycles_processed;
										break;
									}
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									if (operand_0_field_index)
									{
										++cycles_processed;
										break;
									}
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (data_size)
							{
								case 2:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									if (operand_0_field_index)
									{
										++cycles_processed;
										break;
									}
									DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
									DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
									SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], immediate_value, CurrentCPU.FL, signed_mode);
									++cycles_processed;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					PointerControlData operand_1_source_pointer_control = GetPointerControlData<2, 4, 6>(operand_control[1]);
					switch (operand_1_source_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_1_source_pointer_control.target)
							{
								case 0:
								{
									Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
									uint8_t operand_1_region_id = instruction_data.data[5];
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &operand_1_source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
									OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									switch (operand_1_source_pointer_control.offset_type)
									{
										case 2:
										{
											operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
											operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
											break;
										}
									}
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_register], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_register], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_register], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_register], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, operand_1_region_id, operand_1_source_pointer, operand_1_source_offset_data), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									++cycles_processed;
									break;
								}
								case 2:
								{
									struct alignas(8) ChipsetSourceOperandData
									{
										DWord_LE source_pointer;
										Word_LE memory_group;
									};
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									ChipsetSourceOperandData &operand_1_chipset_source_operand_data = reinterpret_cast<ChipsetSourceOperandData &>(extra_data);
									uint8_t operand_1_chipset = instruction_data.data[5];
									OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
									ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
									switch (operand_1_source_pointer_control.offset_type)
									{
										case 2:
										{
											operand_1_source_offset_data.offset = CurrentCPU.SI.offset;
											operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
											break;
										}
									}
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], data, CurrentCPU.FL, signed_mode);
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], data, CurrentCPU.FL, signed_mode);
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], data, CurrentCPU.FL, signed_mode);
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
												case 3:
												{
													QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(operand_1_chipset, operand_1_chipset_source_operand_data.memory_group, operand_1_chipset_source_operand_data.source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
													if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
													{
														SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], data, CurrentCPU.FL, signed_mode);
														cycles_processed += 2;
													}
													else
													{
														++cycles_processed;
													}
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 1:
						{
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;

													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
									DWord_LE &operand_1_source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}

											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { operand_1_source_offset, IndexRegisterType::None }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 2:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 1:
												{
													WordField &operand_0_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 2:
												{
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_0_register]);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
												case 3:
												{
													SetConditionFlag<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[operand_0_register], LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														++cycles_processed;
														break;
													}
													DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
													SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source }), CurrentCPU.FL, signed_mode);
													cycles_processed += 2;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::CompareInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		std::array<uint8_t, 2> data;
	} &compare_instruction_base = reinterpret_cast<CompareInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_0_type = (compare_instruction_base.operand_0_control & GenerateFieldBitmask<uint16_t>(0, 1));
	uint8_t operand_1_type = (compare_instruction_base.operand_1_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((compare_instruction_base.operand_0_control & GenerateFieldBitmask<uint16_t>(1, 3)) >> 1);
	bool register_field_mode = (compare_instruction_base.operand_0_control & GenerateFieldBitmask<uint16_t>(4, 1));
	bool signed_mode = (compare_instruction_base.operand_0_control & GenerateFieldBitmask<uint16_t>(5, 1));
	data.CurrentCPU.FL &= ~(0xE0);
	switch (operand_0_type)
	{
		case 0:
		{
			uint8_t &operand_0_register = compare_instruction_base.data[0];
			uint8_t operand_0_field_index = 0;
			if (register_field_mode)
			{
				operand_0_field_index = ((compare_instruction_base.operand_0_control & GenerateFieldBitmask<uint16_t>(6, 4)) >> 6);
			}
			switch (operand_1_type)
			{
				case 0:
				{
					uint8_t &operand_1_register = compare_instruction_base.data[1];
					uint8_t operand_1_field_index = 0;
					if (register_field_mode)
					{
						operand_1_field_index = ((compare_instruction_base.operand_1_control & GenerateFieldBitmask<uint16_t>(2, 4)) >> 2);
					}
					switch (operand_0_register)
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
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 0:
										{
											ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											ByteField &operand_1_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 1:
										{
											WordField &operand_0_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											WordField &operand_1_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<Word_LE, int16_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 2:
										{
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
										case 3:
										{
											SetConditionFlag<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[operand_0_register], data.CurrentCPU.GPR_Registers[operand_1_register], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												data.CurrentCPU.FL |= 0x20;
											}
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												data.CurrentCPU.FL |= 0x20;
											}
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												data.CurrentCPU.FL |= 0x20;
											}
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (operand_1_register)
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
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_1_register]);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x10:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x11:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x20:
								{
									switch (data_size)
									{
										case 2:
										{
											if (operand_0_field_index || operand_1_field_index)
											{
												// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
												data.callback = nullptr;
												break;
											}
											DWordField &operand_0_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
											DWordField &operand_1_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
											SetConditionFlag<DWord_LE, int32_t>(operand_0_register_field[operand_0_field_index], operand_1_register_field[operand_1_field_index], data.CurrentCPU.FL, signed_mode);
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
								case 0x21:
								{
									switch (data_size)
									{
										case 2:
										{
											if (!operand_0_field_index && !operand_1_field_index)
											{
												data.CurrentCPU.FL |= 0x20;
											}
											// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
											data.callback = nullptr;
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (operand_0_register)
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
							switch (data_size)
							{
								case 0:
								{
									ByteField &operand_0_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_0_register]);
									SetConditionFlag<uint8_t, int8_t>(operand_0_register_field[operand_0_field_index], compare_instruction_base.data[1], data.CurrentCPU.FL, signed_mode);
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(signed_mode);
									data.callback = QWord_Immediate_Value_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 0x10:
						{
							switch (data_size)
							{
								case 2:
								{
									if (operand_0_field_index)
									{
										// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
										data.callback = nullptr;
										break;
									}
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
							}
							break;
						}
						case 0x11:
						{
							switch (data_size)
							{
								case 2:
								{
									if (operand_0_field_index)
									{
										// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
										data.callback = nullptr;
										break;
									}
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
							}
							break;
						}
						case 0x20:
						{
							switch (data_size)
							{
								case 2:
								{
									if (operand_0_field_index)
									{
										// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
										data.callback = nullptr;
										break;
									}
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
							}
							break;
						}
						case 0x21:
						{
							switch (data_size)
							{
								case 2:
								{
									if (operand_0_field_index)
									{
										// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
										data.callback = nullptr;
										break;
									}
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.callback = Immediate_Value_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					PointerControlData operand_1_source_pointer_control = GetPointerControlData<2, 4, 6>(compare_instruction_base.operand_1_control);
					switch (operand_1_source_pointer_control.pointer_type)
					{
						case 0:
						{
							switch (operand_1_source_pointer_control.target)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_1_source_pointer_control.offset_type);
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.current_data.push_back(data_size);
									data.callback = Absolute_Pointer_Self_To_Register_ExecuteCycle;
									break;
								}
								case 1:
								{
									// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
									data.callback = nullptr;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_1_source_pointer_control.offset_type);
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.current_data.push_back(data_size);
									data.callback = Absolute_Pointer_Chipset_To_Register_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 1:
						{
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
													break;
												}
												case 2:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
												case 3:
												{
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.current_data.push_back(data_size);
									data.callback = Base_Pointer_Relative_Offset_To_Register_ExecuteCycle;
									break;
								}
								case 2:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
													break;
												}
												case 2:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
												case 3:
												{
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
						case 2:
						{
							ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
							switch (operand_1_source_pointer_control.offset_type)
							{
								case 0:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
													break;
												}
												case 2:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
												case 3:
												{
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.current_data.push_back(operand_0_field_index);
									data.current_data.push_back(signed_mode);
									data.current_data.push_back(data_size);
									data.callback = Stack_Pointer_Relative_Offset_To_Register_ExecuteCycle;
									break;
								}
								case 2:
								{
									switch (operand_0_register)
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
											switch (data_size)
											{
												case 0:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
													break;
												}
												case 1:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
													break;
												}
												case 2:
												{
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
												case 3:
												{
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
													break;
												}
											}
											break;
										}
										case 0x10:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x11:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x20:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
										case 0x21:
										{
											switch (data_size)
											{
												case 2:
												{
													if (operand_0_field_index)
													{
														// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
														data.callback = nullptr;
														break;
													}
													data.current_data.push_back(operand_0_field_index);
													data.current_data.push_back(signed_mode);
													data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source }));
													data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
													break;
												}
											}
											break;
										}
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			break;
		}
	}
}

template <ClassicVCom_Nova64::WordMinimumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::CompareInstruction::Immediate_Value_To_Register_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionImmediateValueToRegisterFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t unused;
		T immediate_value;
	} &compare_instruction_immediate_value_to_register_field = reinterpret_cast<CompareInstructionImmediateValueToRegisterFieldData &>(data.instruction_data[0]);
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	switch (compare_instruction_immediate_value_to_register_field.operand_0_register)
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
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[compare_instruction_immediate_value_to_register_field.operand_0_register]);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], compare_instruction_immediate_value_to_register_field.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x10:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.SI);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], compare_instruction_immediate_value_to_register_field.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x11:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.DI);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], compare_instruction_immediate_value_to_register_field.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x20:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.BP);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], compare_instruction_immediate_value_to_register_field.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x21:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.SP);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], compare_instruction_immediate_value_to_register_field.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::QWord_Immediate_Value_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionQWordImmediateValueToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t unused;
		QWord_LE immediate_value;
	} &compare_instruction_qword_immediate_value_to_register = reinterpret_cast<CompareInstructionQWordImmediateValueToRegisterData &>(data.instruction_data[0]);
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	switch (compare_instruction_qword_immediate_value_to_register.operand_0_register)
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
			SetConditionFlag<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[compare_instruction_qword_immediate_value_to_register.operand_0_register], compare_instruction_qword_immediate_value_to_register.immediate_value, data.CurrentCPU.FL, signed_mode);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::Absolute_Pointer_Self_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionAbsolutePointerSelfToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t operand_1_region_id;
		DWord_LE operand_1_source_pointer;
	} &compare_instruction_absolute_pointer_self_to_register = reinterpret_cast<CompareInstructionAbsolutePointerSelfToRegisterData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (compare_instruction_absolute_pointer_self_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
					break;
				}
				case 2:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
				case 3:
				{
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, compare_instruction_absolute_pointer_self_to_register.operand_1_region_id, compare_instruction_absolute_pointer_self_to_register.operand_1_source_pointer, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::CompareInstruction::Pointer_Data_To_Register_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionPointerDataToRegisterFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_0_type;
		Word_LE operand_1_type;
		uint8_t operand_0_register;
	} &compare_instruction_pointer_data_to_register_field = reinterpret_cast<CompareInstructionPointerDataToRegisterFieldData &>(data.instruction_data[0]);
	T value = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	switch (compare_instruction_pointer_data_to_register_field.operand_0_register)
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
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[compare_instruction_pointer_data_to_register_field.operand_0_register]);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x10:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.SI);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x11:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.DI);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x20:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.BP);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
		case 0x21:
		{
			T3 &operand_0_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.SP);
			SetConditionFlag<T, T2>(operand_0_register_field[operand_0_field_index], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::QWord_Pointer_Data_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionQWordPointerDataToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
	} &compare_instruction_qword_pointer_data_to_register = reinterpret_cast<CompareInstructionQWordPointerDataToRegisterData &>(data.instruction_data[0]);
	QWord_LE value = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	switch (compare_instruction_qword_pointer_data_to_register.operand_0_register)
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
			SetConditionFlag<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[compare_instruction_qword_pointer_data_to_register.operand_0_register], value, data.CurrentCPU.FL, signed_mode);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::Absolute_Pointer_Chipset_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionAbsolutePointerChipsetToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t operand_1_chipset;
		DWord_LE operand_1_source_pointer;
		Word_LE operand_1_memory_group;
	} &compare_instruction_absolute_pointer_chipset_to_register = reinterpret_cast<CompareInstructionAbsolutePointerChipsetToRegisterData &>(data.instruction_data[0]);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_1_offset_type = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode operand_1_chipset_read_return = ChipsetReturnCode::Ok;
	switch (operand_1_offset_type)
	{
		case 2:
		{
			operand_1_source_offset_data.offset = data.CurrentCPU.SI.offset;
			operand_1_source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	switch (compare_instruction_absolute_pointer_chipset_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					uint8_t value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(compare_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, compare_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, compare_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.current_data.push_back(operand_0_field_index);
						data.current_data.push_back(signed_mode);
						data.current_data.push_back(value);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 1:
				{
					Word_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(compare_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, compare_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, compare_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.current_data.push_back(operand_0_field_index);
						data.current_data.push_back(signed_mode);
						data.current_data.push_back(value);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 2:
				{
					DWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(compare_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, compare_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, compare_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.current_data.push_back(operand_0_field_index);
						data.current_data.push_back(signed_mode);
						data.current_data.push_back(value);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
				case 3:
				{
					QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(compare_instruction_absolute_pointer_chipset_to_register.operand_1_chipset, compare_instruction_absolute_pointer_chipset_to_register.operand_1_memory_group, compare_instruction_absolute_pointer_chipset_to_register.operand_1_source_pointer, operand_1_source_offset_data, operand_1_chipset_read_return);
					if (operand_1_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
					{
						data.current_data.push_back(signed_mode);
						data.current_data.push_back(value);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					}
					else
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::Base_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionBasePointerRelativeOffsetToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t unused;
		DWord_LE operand_1_relative_offset;
	} &compare_instruction_base_pointer_relative_offset_to_register = reinterpret_cast<CompareInstructionBasePointerRelativeOffsetToRegisterData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { compare_instruction_base_pointer_relative_offset_to_register.operand_1_relative_offset, IndexRegisterType::None };
	switch (compare_instruction_base_pointer_relative_offset_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
					break;
				}
				case 2:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
				case 3:
				{
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::CompareInstruction::Stack_Pointer_Relative_Offset_To_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) CompareInstructionStackPointerRelativeOffsetToRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_0_control;
		Word_LE operand_1_control;
		uint8_t operand_0_register;
		uint8_t unused;
		DWord_LE operand_1_relative_offset;
	} &compare_instruction_stack_pointer_relative_offset_to_register = reinterpret_cast<CompareInstructionStackPointerRelativeOffsetToRegisterData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	uint8_t data_size = data.current_data.back();
	data.current_data.pop_back();
	bool signed_mode = data.current_data.back();
	data.current_data.pop_back();
	uint8_t operand_0_field_index = data.current_data.back();
	data.current_data.pop_back();
	OffsetData operand_1_source_offset_data = { compare_instruction_stack_pointer_relative_offset_to_register.operand_1_relative_offset, IndexRegisterType::None };
	switch (compare_instruction_stack_pointer_relative_offset_to_register.operand_0_register)
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
			switch (data_size)
			{
				case 0:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
					break;
				}
				case 1:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
					break;
				}
				case 2:
				{
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
				case 3:
				{
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<QWord_Pointer_Data_To_Register_ExecuteCycle>;
					break;
				}
			}
			break;
		}
		case 0x10:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x11:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x20:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
		case 0x21:
		{
			switch (data_size)
			{
				case 2:
				{
					if (operand_0_field_index)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					data.current_data.push_back(operand_0_field_index);
					data.current_data.push_back(signed_mode);
					data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, operand_1_source_offset_data));
					data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Register_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
					break;
				}
			}
			break;
		}
	}
}

/*
void ClassicVCom_Nova64::Instruction::Jump(CPU &CurrentCPU, BaseInstructionData &instruction_data)
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
					}
					else
					{
						Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
						ByteField &operand_register_byte_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[source_register]);
						uint8_t new_region_id = (operand_register_byte_field[4] & 0xF);
						if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
						{
							break;
						}
						CurrentCPU.IP.address = operand_register_dword_field[0];
						CurrentCPU.IP.memory_control &= ~(0xF000);
						CurrentCPU.IP.memory_control |= (new_region_id << 12);
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
				break;
			}
			DWord_LE &operand_immediate_value = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
			if (!cross_region_jump)
			{
				CurrentCPU.IP.address = operand_immediate_value;
			}
			else
			{
				Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
				uint8_t region_id = ((operand_control & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
				if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, region_id))
				{
					break;
				}
				CurrentCPU.IP.address = operand_immediate_value;
				CurrentCPU.IP.memory_control &= ~(0xF000);
				CurrentCPU.IP.memory_control |= (region_id << 12);
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
						break;
					}
					Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
					uint8_t source_region_id = extra_data[0];
					DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
					OffsetData offset_data = { 0, false };
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
					}
					else
					{
						uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, source_region_id, source_pointer + 4, offset_data) & 0xF);
						if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id, new_region_id))
						{
							break;
						}
						CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, source_region_id, source_pointer, offset_data);
						CurrentCPU.IP.memory_control &= ~(0xF000);
						CurrentCPU.IP.memory_control |= (new_region_id << 12);
					}
					break;
				}
				case 1:
				{
					if (!perform_jump)
					{
						break;
					}
					Word_LE current_program_id = (CurrentCPU.BP.memory_control & 0xFFF);
					uint8_t current_region_id = ((CurrentCPU.BP.memory_control & 0xF000) >> 12);
					OffsetData offset_data = { 0, false };
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
						CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
					}
					else
					{
						Word_LE current_program_id_2 = (CurrentCPU.IP.memory_control & 0xFFF);
						uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address + 4, offset_data) & 0xF);
						if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id_2, new_region_id))
						{
							break;
						}
						CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.BP.address, offset_data);
						CurrentCPU.IP.memory_control &= ~(0xF000);
						CurrentCPU.IP.memory_control |= (new_region_id << 12);
					}
					break;
				}
				case 2:
				{
					if (!perform_jump)
					{
						break;
					}
					Word_LE current_program_id = (CurrentCPU.SP.memory_control & 0xFFF);
					uint8_t current_region_id ((CurrentCPU.SP.memory_control & 0xF000) >> 12);
					OffsetData offset_data = { 0, false };
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
						CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
					}
					else
					{
						Word_LE current_program_id_2 = (CurrentCPU.IP.memory_control & 0xFFF);
						uint8_t new_region_id = (LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address + 4, offset_data) & 0xF);
						if (!HasRegionFlagSupport<0x04>(CurrentCPU, current_program_id_2, new_region_id))
						{
							break;
						}
						CurrentCPU.IP.address = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, current_region_id, CurrentCPU.SP.address, offset_data);
						CurrentCPU.IP.memory_control &= ~(0xF000);
						CurrentCPU.IP.memory_control |= (new_region_id << 12);
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::JumpInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) JumpInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		std::array<uint8_t, 4> data;
	} &jump_instruction_base = reinterpret_cast<JumpInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t jump_type = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(2, 5)) >> 2);
	bool cross_region_jump = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(7, 1)) >> 7);
	bool perform_jump = true;
	switch (jump_type)
	{
		case 1:
		{
			perform_jump = (data.CurrentCPU.FL & 0x01);
			break;
		}
		case 2:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x01);
			break;
		}
		case 3:
		{
			perform_jump = (data.CurrentCPU.FL & 0x02);
			break;
		}
		case 4:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x02);
			break;
		}
		case 5:
		{
			perform_jump = (data.CurrentCPU.FL & 0x04);
			break;
		}
		case 6:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x04);
			break;
		}
		case 7:
		{
			perform_jump = (data.CurrentCPU.FL & 0x08);
			break;
		}
		case 8:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x08);
			break;
		}
		case 9:
		{
			perform_jump = (data.CurrentCPU.FL & 0x10);
			break;
		}
		case 10:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x10);
			break;
		}
		case 11:
		{
			perform_jump = (data.CurrentCPU.FL & 0x20);
			break;
		}
		case 12:
		{
			perform_jump = !(data.CurrentCPU.FL & 0x20);
			break;
		}
		case 13:
		{
			perform_jump = (data.CurrentCPU.FL & 0x40);
			break;
		}
		case 14:
		{
			perform_jump = (data.CurrentCPU.FL & 0x60);
			break;
		}
		case 15:
		{
			perform_jump = (data.CurrentCPU.FL & 0x80);
			break;
		}
		case 16:
		{
			perform_jump = (data.CurrentCPU.FL & 0xA0);
			break;
		}
	}
	switch (operand_type)
	{
		case 0:
		{
			if (!perform_jump)
			{
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
				break;
			}
			uint8_t &source_register = jump_instruction_base.data[0];
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
					DWordField &operand_register_dword_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[source_register]);
					if (!cross_region_jump)
					{
						data.CurrentCPU.IP.address = operand_register_dword_field[0];
						data.CurrentCPU.SR |= 0x01;
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
					}
					else
					{
						Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
						ByteField &operand_register_byte_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[source_register]);
						uint8_t new_region_id = (operand_register_byte_field[4] & 0xF);
						if (!HasRegionFlagSupport<0x04>(data.CurrentCPU, current_program_id, new_region_id))
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
						}
						else
						{
							data.current_data.push_back(new_region_id);
							data.current_data.push_back(operand_register_dword_field[0]);
							data.callback = CrossRegion_ExecuteCycle;
						}
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
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
				break;
			}
			DWord_LE &operand_immediate_value = reinterpret_cast<DWord_LE &>(jump_instruction_base.data[0]);
			if (!cross_region_jump)
			{
				data.CurrentCPU.IP.address = operand_immediate_value;
				data.CurrentCPU.SR |= 0x01;
				// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
				data.callback = nullptr;
			}
			else
			{
				Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
				uint8_t region_id = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
				if (!HasRegionFlagSupport<0x04>(data.CurrentCPU, current_program_id, region_id))
				{
					// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
					data.callback = nullptr;
				}
				else
				{
					data.current_data.push_back(region_id);
					data.current_data.push_back(operand_immediate_value);
					data.callback = CrossRegion_ExecuteCycle;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t pointer_type = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(10, 2)) >> 10);
			switch (pointer_type)
			{
				case 0:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.current_data.push_back(cross_region_jump);
					data.current_data.push_back(perform_jump);
					data.callback = Absolute_Pointer_Self_ExecuteCycle;
					break;
				}
				case 1:
				{
					if (!perform_jump)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
						data.callback = nullptr;
						break;
					}
					uint8_t offset_type = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(8, 2)) >> 8);
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &offset = reinterpret_cast<DWord_LE &>(jump_instruction_base.data[0]);
							offset_data.offset = offset;
							break;
						}
						case 2:
						{
							offset_data.offset = data.CurrentCPU.SI.offset;
							break;
						}
					}
					if (!cross_region_jump)
					{
						data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, offset_data));
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_NoCrossRegion_ExecuteCycle>;
					}
					else
					{
						data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address + 4, offset_data) & 0xF);
						data.current_data.push_back(current_program_memory_control.region_id);
						data.current_data.push_back(current_program_memory_control.program_id);
						data.current_data.push_back(offset_data.offset);
						data.current_data.push_back(data.CurrentCPU.BP.address);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_CrossRegion_ExecuteCycle_1>;
					}
					break;
				}
				case 2:
				{
					if (!perform_jump)
					{
						// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;;
						data.callback = nullptr;
						break;
					}
					uint8_t offset_type = ((jump_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(8, 2)) >> 8);
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
					OffsetData offset_data = { 0, IndexRegisterType::None };
					switch (offset_type)
					{
						case 1:
						{
							DWord_LE &offset = reinterpret_cast<DWord_LE &>(jump_instruction_base.data[0]);
							offset_data.offset = offset;
							break;
						}
						case 2:
						{
							offset_data.offset = data.CurrentCPU.SI.offset;
							break;
						}
					}
					if (!cross_region_jump)
					{
						data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, offset_data));
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_NoCrossRegion_ExecuteCycle>;
					}
					else
					{
						data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address + 4, offset_data) & 0xF);
						data.current_data.push_back(current_program_memory_control.region_id);
						data.current_data.push_back(current_program_memory_control.program_id);
						data.current_data.push_back(offset_data.offset);
						data.current_data.push_back(data.CurrentCPU.SP.address);
						data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_CrossRegion_ExecuteCycle_1>;
					}
					break;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Absolute_Pointer_Self_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) JumpInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE source_pointer;
		uint8_t source_region_id;
	} &jump_instruction_absolute_pointer_self = reinterpret_cast<JumpInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	bool perform_jump = data.current_data.back();
	data.current_data.pop_back();
	bool cross_region_jump = data.current_data.back();
	data.current_data.pop_back();
	if (!perform_jump)
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
	else
	{
		uint8_t offset_type = ((jump_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(8, 2)) >> 8);
		Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
		OffsetData offset_data = { 0, IndexRegisterType::None };
		switch (offset_type)
		{
			case 2:
			{
				offset_data.offset = data.CurrentCPU.SI.offset;
				break;
			}
		}
		if (!cross_region_jump)
		{
			data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, jump_instruction_absolute_pointer_self.source_region_id, jump_instruction_absolute_pointer_self.source_pointer, offset_data));
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_NoCrossRegion_ExecuteCycle>;
		}
		else
		{
			data.current_data.push_back(LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_id, jump_instruction_absolute_pointer_self.source_region_id, jump_instruction_absolute_pointer_self.source_pointer + 4, offset_data) & 0xF);
			data.current_data.push_back(current_program_id);
			data.current_data.push_back(offset_data.offset);
			data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Absolute_Pointer_Self_CrossRegion_ExecuteCycle_1>;
		}
	}
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Pointer_NoCrossRegion_ExecuteCycle(InstructionCallbackData &data)
{
	data.CurrentCPU.IP.address = data.current_data.back();
	data.current_data.pop_back();
	data.CurrentCPU.SR |= 0x01;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Absolute_Pointer_Self_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data)
{
	struct alignas(8) JumpInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE source_pointer;
		uint8_t source_region_id;
	} &jump_instruction_absolute_pointer_self = reinterpret_cast<JumpInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	OffsetData offset_data = { data.current_data.back(), IndexRegisterType::None };
	data.current_data.pop_back();
	Word_LE current_program_id = data.current_data.back();
	data.current_data.pop_back();
	uint8_t new_region_id = data.current_data.back();
	if (!HasRegionFlagSupport<0x04>(data.CurrentCPU, current_program_id, new_region_id))
	{
		data.current_data.pop_back();
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
	else
	{
		data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_id, jump_instruction_absolute_pointer_self.source_region_id, jump_instruction_absolute_pointer_self.source_pointer, offset_data));
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Absolute_Pointer_Self_CrossRegion_ExecuteCycle_2>;
	}
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Absolute_Pointer_Self_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data)
{
	data.callback = CrossRegion_ExecuteCycle;
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Pointer_Register_CrossRegion_ExecuteCycle_1(InstructionCallbackData &data)
{
	DWord_LE pointer_address = data.current_data.back();
	data.current_data.pop_back();
	OffsetData offset_data = { data.current_data.back(), IndexRegisterType::None };
	data.current_data.pop_back();
	ProgramMemoryControlData current_program_memory_control;
	current_program_memory_control.program_id = data.current_data.back();
	data.current_data.pop_back();
	current_program_memory_control.region_id = data.current_data.back();
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	uint8_t new_region_id = data.current_data.back();
	if (!HasRegionFlagSupport<0x04>(data.CurrentCPU, current_program_id, new_region_id))
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
	else
	{
		data.current_data.push_back(LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, pointer_address, offset_data));
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_CrossRegion_ExecuteCycle_2>;
	}
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::Pointer_Register_CrossRegion_ExecuteCycle_2(InstructionCallbackData &data)
{
	data.callback = CrossRegion_ExecuteCycle;
}

void ClassicVCom_Nova64::Instruction::JumpInstruction::CrossRegion_ExecuteCycle(InstructionCallbackData &data)
{
	data.CurrentCPU.IP.address = data.current_data.back();
	data.current_data.pop_back();
	SetProgramRegion(data.CurrentCPU.IP, data.current_data.back());
	data.current_data.pop_back();
	data.CurrentCPU.SR |= 0x03;
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::Add(CPU &CurrentCPU, BaseInstructionData &instruction_data)
{
	struct OperandControlData
	{
		Word_LE o_0, o_1;

		Word_LE &operator[](int index)
		{
			switch (index)
			{
				case 0: { return o_0; }
				case 1: { return o_1; }
			}
			return o_0;
		}
	};
	OperandControlData &operand_control = reinterpret_cast<OperandControlData &>(instruction_data.data[0]);
	uint8_t operand_type = (operand_control[0] & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	bool register_field_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(5, 1));
	bool signed_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(6, 1));
	bool carry_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(7, 1));
	uint8_t accumulator_field_index = 0;
	if (register_field_mode)
	{
		accumulator_field_index = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
	}
	switch (operand_type)
	{
		case 0:
		{
			uint8_t operand_register = instruction_data.data[4];
			uint8_t operand_field_index = 0;
			if (register_field_mode)
			{
				operand_field_index = ((operand_control[0] & GenerateFieldBitmask<uint16_t>(12, 4)) >> 12);
			}
			switch (operand_register)
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
							ByteField &operand_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
						case 1:
						{
							WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
							WordField &operand_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
						case 2:
						{
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
						case 3:
						{
							PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], CurrentCPU.GPR_Registers[operand_register], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_mode);
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			switch (data_size)
			{
				case 0:
				{
					ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
					PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], instruction_data.data[4], CurrentCPU.FL, signed_mode, carry_mode);
					break;
				}
				case 1:
				{
					WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
					Word_LE &immediate_value = reinterpret_cast<Word_LE &>(instruction_data.data[4]);
					PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], immediate_value, CurrentCPU.FL, signed_mode, carry_mode);
					break;
				}
				case 2:
				{
					ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
					DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
					DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
					PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], immediate_value, CurrentCPU.FL, signed_mode, carry_mode);
					break;
				}
				case 3:
				{
					ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
					QWord_LE &immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
					PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], immediate_value, CurrentCPU.FL, signed_mode, carry_mode);
					break;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t offset_type = (operand_control[1] & GenerateFieldBitmask<uint16_t>(0, 2));
			uint8_t pointer_type = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
			uint8_t pointer_source = ((operand_control[1] & GenerateFieldBitmask<uint16_t>(4, 2)) >> 4);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_source)
					{
						case 0:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
							uint8_t region_id = instruction_data.data[4];
							DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData source_offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 2:
								{
									source_offset_data.offset = CurrentCPU.SI.offset;
									source_offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
						case 1:
						{
							break;
						}
						case 2:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE &current_memory_group = reinterpret_cast<Word_LE &>(instruction_data.data[4]);
							uint8_t chipset = extra_data[4];
							DWord_LE &source_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData source_offset_data = { 0, IndexRegisterType::None };
							ChipsetReturnCode chipset_read_return = ChipsetReturnCode::Ok;
							switch (offset_type)
							{
								case 2:
								{
									source_offset_data.offset = CurrentCPU.SI.offset;
									source_offset_data.index_register_used = IndexRegisterType::Source;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
									if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									}
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
									if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									}
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
									if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									}
									break;
								}
								case 3:
								{
									QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
									if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
						case 1:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							DWord_LE &source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
						case 1:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							DWord_LE &source_offset = reinterpret_cast<DWord_LE &>(extra_data[0]);
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
									PerformAddition<QWord_LE, int64_t>(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_mode);
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::AddInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		std::array<uint8_t, 2> data;
	} &add_instruction_base = reinterpret_cast<AddInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			bool register_field_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
			bool signed_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
			bool carry_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
			uint8_t accumulator_field_index = 0;
			uint8_t &operand_register = add_instruction_base.data[0];
			uint8_t operand_field_index = 0;
			if (register_field_mode)
			{
				accumulator_field_index = ((add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
				operand_field_index = ((add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(12, 4)) >> 12);
			}
			switch (operand_register)
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0]);
							ByteField &operand_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 1:
						{
							WordField &accumulator_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[0]);
							WordField &operand_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 3:
						{
							PerformAddition<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], data.CurrentCPU.GPR_Registers[operand_register], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
							PerformAddition<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, carry_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			bool register_field_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
			bool signed_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
			bool carry_mode = (add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
			uint8_t accumulator_field_index = 0;
			if (register_field_mode)
			{
				accumulator_field_index = ((add_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
			}
			switch (data_size)
			{
				case 0:
				{
					ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0]);
					PerformAddition<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], add_instruction_base.data[0], data.CurrentCPU.FL, signed_mode, carry_mode);
					// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
					data.callback = nullptr;
					break;
				}
				case 1:
				{
					WordField &accumulator_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[0]);
					Word_LE &immediate_value = reinterpret_cast<Word_LE &>(add_instruction_base.data[0]);
					PerformAddition<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], immediate_value, data.CurrentCPU.FL, signed_mode, carry_mode);
					// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
					data.callback = nullptr;
					break;
				}
				case 2:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.callback = Immediate_Value_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
					break;
				}
				case 3:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.callback = Immediate_Value_To_Accumulator_ExecuteCycle;
					break;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t pointer_type = ((add_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
			uint8_t pointer_source = ((add_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(4, 2)) >> 4);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_source)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 1:
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					uint8_t offset_type = (add_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					uint8_t offset_type = (add_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordMinimumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionImmediateValueToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &add_instruction_immediate_value_to_accumulator_field = reinterpret_cast<AddInstructionImmediateValueToAccumulatorFieldData &>(data.instruction_data[0]);
	bool register_field_mode = (add_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
	bool signed_mode = (add_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool carry_mode = (add_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	uint8_t accumulator_field_index = 0;
	if (register_field_mode)
	{
		accumulator_field_index = ((add_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
	}
	T3 &accumulator_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[0]);
	PerformAddition<T, T2>(accumulator_register_field[accumulator_field_index], add_instruction_immediate_value_to_accumulator_field.immediate_value, data.CurrentCPU.FL, signed_mode, carry_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionImmediateValueToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		QWord_LE immediate_value;
	} &add_instruction_immediate_value_to_accumulator = reinterpret_cast<AddInstructionImmediateValueToAccumulatorData &>(data.instruction_data[0]);
	bool signed_mode = (add_instruction_immediate_value_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool carry_mode = (add_instruction_immediate_value_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	PerformAddition<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], add_instruction_immediate_value_to_accumulator.immediate_value, data.CurrentCPU.FL, signed_mode, carry_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionAbsolutePointerSelfToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t source_region_id;
		uint8_t unused;
		DWord_LE source_pointer;
	} &add_instruction_absolute_pointer_self_to_accumulator_field = reinterpret_cast<AddInstructionAbsolutePointerSelfToAccumulatorFieldData &>(data.instruction_data[0]);
	uint8_t offset_type = (add_instruction_absolute_pointer_self_to_accumulator_field.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_id, add_instruction_absolute_pointer_self_to_accumulator_field.source_region_id, add_instruction_absolute_pointer_self_to_accumulator_field.source_pointer, source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionPointerDataToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
	} &add_instruction_pointer_data_to_accumulator_field = reinterpret_cast<AddInstructionPointerDataToAccumulatorFieldData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	bool register_field_mode = (add_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
	bool signed_mode = (add_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool carry_mode = (add_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	uint8_t accumulator_field_index = 0;
	if (register_field_mode)
	{
		accumulator_field_index = ((add_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
	}
	T3 &accumulator_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[0]);
	PerformAddition<T, T2>(accumulator_register_field[accumulator_field_index], value, data.CurrentCPU.FL, signed_mode, carry_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionAbsolutePointerSelfToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t source_region_id;
		uint8_t unused;
		DWord_LE source_pointer;
	} &add_instruction_absolute_pointer_self_to_accumulator = reinterpret_cast<AddInstructionAbsolutePointerSelfToAccumulatorData &>(data.instruction_data[0]);
	uint8_t offset_type = (add_instruction_absolute_pointer_self_to_accumulator.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, add_instruction_absolute_pointer_self_to_accumulator.source_region_id, add_instruction_absolute_pointer_self_to_accumulator.source_pointer, source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionPointerDataToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
	} &add_instruction_pointer_data_to_accumulator = reinterpret_cast<AddInstructionPointerDataToAccumulatorData &>(data.instruction_data[0]);
	QWord_LE value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	bool signed_mode = (add_instruction_pointer_data_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool carry_mode = (add_instruction_pointer_data_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	PerformAddition<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], value, data.CurrentCPU.FL, signed_mode, carry_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionAbsolutePointerChipsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE memory_group;
		DWord_LE source_pointer;
		uint8_t chipset;
	} &add_instruction_absolute_pointer_chipset_to_accumulator_field = reinterpret_cast<AddInstructionAbsolutePointerChipsetToAccumulatorFieldData &>(data.instruction_data[0]);
	uint8_t offset_type = (add_instruction_absolute_pointer_chipset_to_accumulator_field.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode chipset_read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	T value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<T>(add_instruction_absolute_pointer_chipset_to_accumulator_field.chipset, add_instruction_absolute_pointer_chipset_to_accumulator_field.memory_group, add_instruction_absolute_pointer_chipset_to_accumulator_field.source_pointer, source_offset_data, chipset_read_return);
	if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionAbsolutePointerChipsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE memory_group;
		DWord_LE source_pointer;
		uint8_t chipset;
	} &add_instruction_absolute_pointer_chipset_to_accumulator = reinterpret_cast<AddInstructionAbsolutePointerChipsetToAccumulatorData &>(data.instruction_data[0]);
	uint8_t offset_type = (add_instruction_absolute_pointer_chipset_to_accumulator.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode chipset_read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(add_instruction_absolute_pointer_chipset_to_accumulator.chipset, add_instruction_absolute_pointer_chipset_to_accumulator.memory_group, add_instruction_absolute_pointer_chipset_to_accumulator.source_pointer, source_offset_data, chipset_read_return);
	if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionBasePointerRelativeOffsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &add_instruction_base_pointer_relative_offset_to_accumulator_field = reinterpret_cast<AddInstructionBasePointerRelativeOffsetToAccumulatorFieldData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { add_instruction_base_pointer_relative_offset_to_accumulator_field.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionBasePointerRelativeOffsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &add_instruction_base_pointer_relative_offset_to_accumulator = reinterpret_cast<AddInstructionBasePointerRelativeOffsetToAccumulatorData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { add_instruction_base_pointer_relative_offset_to_accumulator.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::AddInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionStackPointerRelativeOffsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &add_instruction_stack_pointer_relative_offset_to_accumulator_field = reinterpret_cast<AddInstructionStackPointerRelativeOffsetToAccumulatorFieldData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { add_instruction_stack_pointer_relative_offset_to_accumulator_field.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

void ClassicVCom_Nova64::Instruction::AddInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) AddInstructionStackPointerRelativeOffsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &add_instruction_stack_pointer_relative_offset_to_accumulator = reinterpret_cast<AddInstructionStackPointerRelativeOffsetToAccumulatorData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	data.CurrentCPU.data_bus =LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { add_instruction_stack_pointer_relative_offset_to_accumulator.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		std::array<uint8_t, 2> data;
	} &subtract_instruction_base = reinterpret_cast<SubtractInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(0, 2));
	uint8_t data_size = ((subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			bool register_field_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
			bool signed_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
			bool borrow_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
			uint8_t accumulator_field_index = 0;
			uint8_t &operand_register = subtract_instruction_base.data[0];
			uint8_t operand_field_index = 0;
			if (register_field_mode)
			{
				accumulator_field_index = ((subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
				operand_field_index = ((subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(12, 4)) >> 12);
			}
			switch (operand_register)
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0]);
							ByteField &operand_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformSubtraction<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 1:
						{
							WordField &accumulator_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[0]);
							WordField &operand_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformSubtraction<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							PerformSubtraction<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 3:
						{
							PerformSubtraction<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], data.CurrentCPU.GPR_Registers[operand_register], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
							PerformSubtraction<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
							PerformSubtraction<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
							PerformSubtraction<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
							}
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[0]);
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
							PerformSubtraction<DWord_LE, int32_t>(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], data.CurrentCPU.FL, signed_mode, borrow_mode);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			bool register_field_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
			bool signed_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
			bool borrow_mode = (subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
			uint8_t accumulator_field_index = 0;
			if (register_field_mode)
			{
				accumulator_field_index = ((subtract_instruction_base.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8);
			}
			switch (data_size)
			{
				case 0:
				{
					ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[0]);
					PerformSubtraction<uint8_t, int8_t>(accumulator_register_field[accumulator_field_index], subtract_instruction_base.data[0], data.CurrentCPU.FL, signed_mode, borrow_mode);
					// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
					data.callback = nullptr;
					break;
				}
				case 1:
				{
					WordField &accumulator_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[0]);
					Word_LE &immediate_value = reinterpret_cast<Word_LE &>(subtract_instruction_base.data[0]);
					PerformSubtraction<Word_LE, int16_t>(accumulator_register_field[accumulator_field_index], immediate_value, data.CurrentCPU.FL, signed_mode, borrow_mode);
					// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
					data.callback = nullptr;
					break;
				}
				case 2:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.callback = Immediate_Value_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
					break;
				}
				case 3:
				{
					data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
					data.callback = Immediate_Value_To_Accumulator_ExecuteCycle;
					break;
				}
			}
			break;
		}
		case 2:
		{
			uint8_t pointer_type = ((subtract_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(2, 2)) >> 2);
			uint8_t pointer_source = ((subtract_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(4, 2)) >> 4);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_source)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Self_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 1:
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					uint8_t offset_type = (subtract_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					uint8_t offset_type = (subtract_instruction_base.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
					switch (offset_type)
					{
						case 0:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { 0, IndexRegisterType::None });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							switch (data_size)
							{
								case 0:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>;
									break;
								}
								case 1:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>;
									break;
								}
								case 2:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>;
									break;
								}
								case 3:
								{
									data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
									data.callback = Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle;
									break;
								}
							}
							break;
						}
						case 2:
						{
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<uint8_t, int8_t, ByteField>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<Word_LE, int16_t, WordField>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<DWord_LE, int32_t, DWordField>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { data.CurrentCPU.SI.offset, IndexRegisterType::Source });
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordMinimumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Immediate_Value_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionImmediateValueToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		T immediate_value;
	} &subtract_instruction_immediate_value_to_accumulator_field = reinterpret_cast<SubtractInstructionImmediateValueToAccumulatorFieldData &>(data.instruction_data[0]);
	bool register_field_mode = (subtract_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
	bool signed_mode = (subtract_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool borrow_mode = (subtract_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	uint8_t accumulator_field_index = register_field_mode ? ((subtract_instruction_immediate_value_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8) : 0;
	T3 &accumulator_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[0]);
	PerformSubtraction<T, T2>(accumulator_register_field[accumulator_field_index], subtract_instruction_immediate_value_to_accumulator_field.immediate_value, data.CurrentCPU.FL, signed_mode, borrow_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Immediate_Value_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionImmediateValueToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		QWord_LE immediate_value;
	} &subtract_instruction_immediate_value_to_accumulator = reinterpret_cast<SubtractInstructionImmediateValueToAccumulatorData &>(data.instruction_data[0]);
	bool signed_mode = (subtract_instruction_immediate_value_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool borrow_mode = (subtract_instruction_immediate_value_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	PerformSubtraction<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], subtract_instruction_immediate_value_to_accumulator.immediate_value, data.CurrentCPU.FL, signed_mode, borrow_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Absolute_Pointer_Self_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionAbsolutePointerSelfToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t source_region_id;
		uint8_t unused;
		DWord_LE source_pointer;
	} &subtract_instruction_absolute_pointer_self_to_accumulator_field = reinterpret_cast<SubtractInstructionAbsolutePointerSelfToAccumulatorFieldData &>(data.instruction_data[0]);
	uint8_t offset_type = (subtract_instruction_absolute_pointer_self_to_accumulator_field.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_id, subtract_instruction_absolute_pointer_self_to_accumulator_field.source_region_id, subtract_instruction_absolute_pointer_self_to_accumulator_field.source_pointer, source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Pointer_Data_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionPointerDataToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
	} &subtract_instruction_pointer_data_to_accumulator_field = reinterpret_cast<SubtractInstructionPointerDataToAccumulatorFieldData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	bool register_field_mode = (subtract_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(5, 1));
	bool signed_mode = (subtract_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool borrow_mode = (subtract_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	uint8_t accumulator_field_index = register_field_mode ? ((subtract_instruction_pointer_data_to_accumulator_field.operand_control_0 & GenerateFieldBitmask<uint16_t>(8, 4)) >> 8) : 0;
	T3 &accumulator_register_field = reinterpret_cast<T3 &>(data.CurrentCPU.GPR_Registers[0]);
	PerformSubtraction<T, T2>(accumulator_register_field[accumulator_field_index], value, data.CurrentCPU.FL, signed_mode, borrow_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Absolute_Pointer_Self_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionAbsolutePointerSelfToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		uint8_t source_region_id;
		uint8_t unused;
		DWord_LE source_pointer;
	} &subtract_instruction_absolute_pointer_self_to_accumulator = reinterpret_cast<SubtractInstructionAbsolutePointerSelfToAccumulatorData &>(data.instruction_data[0]);
	uint8_t offset_type = (subtract_instruction_absolute_pointer_self_to_accumulator.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_id, subtract_instruction_absolute_pointer_self_to_accumulator.source_region_id, subtract_instruction_absolute_pointer_self_to_accumulator.source_pointer, source_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Pointer_Data_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionPointerDataToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
	} &subtract_instruction_pointer_data_to_accumulator = reinterpret_cast<SubtractInstructionPointerDataToAccumulatorData &>(data.instruction_data[0]);
	QWord_LE value = data.CurrentCPU.data_bus;
	bool signed_mode = (subtract_instruction_pointer_data_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(6, 1));
	bool borrow_mode = (subtract_instruction_pointer_data_to_accumulator.operand_control_0 & GenerateFieldBitmask<uint16_t>(7, 1));
	PerformSubtraction<QWord_LE, int64_t>(data.CurrentCPU.GPR_Registers[0], value, data.CurrentCPU.FL, signed_mode, borrow_mode);
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Absolute_Pointer_Chipset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionAbsolutePointerChipsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE memory_group;
		DWord_LE source_pointer;
		uint8_t chipset;
	} &subtract_instruction_absolute_pointer_chipset_to_accumulator_field = reinterpret_cast<SubtractInstructionAbsolutePointerChipsetToAccumulatorFieldData &>(data.instruction_data[0]);
	uint8_t offset_type = (subtract_instruction_absolute_pointer_chipset_to_accumulator_field.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode chipset_read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	T value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<T>(subtract_instruction_absolute_pointer_chipset_to_accumulator_field.chipset, subtract_instruction_absolute_pointer_chipset_to_accumulator_field.memory_group, subtract_instruction_absolute_pointer_chipset_to_accumulator_field.source_pointer, source_offset_data, chipset_read_return);
	if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Absolute_Pointer_Chipset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionAbsolutePointerChipsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE memory_group;
		DWord_LE source_pointer;
		uint8_t chipset;
	} &subtract_instruction_absolute_pointer_chipset_to_accumulator = reinterpret_cast<SubtractInstructionAbsolutePointerChipsetToAccumulatorData &>(data.instruction_data[0]);
	uint8_t offset_type = (subtract_instruction_absolute_pointer_chipset_to_accumulator.operand_control_1 & GenerateFieldBitmask<uint16_t>(0, 2));
	OffsetData source_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode chipset_read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			source_offset_data.offset = data.CurrentCPU.SI.offset;
			source_offset_data.index_register_used = IndexRegisterType::Source;
			break;
		}
	}
	QWord_LE value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(subtract_instruction_absolute_pointer_chipset_to_accumulator.chipset, subtract_instruction_absolute_pointer_chipset_to_accumulator.memory_group, subtract_instruction_absolute_pointer_chipset_to_accumulator.source_pointer, source_offset_data, chipset_read_return);
	if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Base_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionBasePointerRelativeOffsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &subtract_instruction_base_pointer_relative_offset_to_accumulator_field = reinterpret_cast<SubtractInstructionBasePointerRelativeOffsetToAccumulatorFieldData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { subtract_instruction_base_pointer_relative_offset_to_accumulator_field.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Base_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionBasePointerRelativeOffsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &subtract_instruction_base_pointer_relative_offset_to_accumulator = reinterpret_cast<SubtractInstructionBasePointerRelativeOffsetToAccumulatorData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, { subtract_instruction_base_pointer_relative_offset_to_accumulator.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordMaximumRequired T, std::signed_integral T2, ClassicVCom_Nova64::QWordAlignmentRequired T3>
void ClassicVCom_Nova64::Instruction::SubtractInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_Field_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionStackPointerRelativeOffsetToAccumulatorFieldData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &subtract_instruction_stack_pointer_relative_offset_to_accumulator_field = reinterpret_cast<SubtractInstructionStackPointerRelativeOffsetToAccumulatorFieldData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { subtract_instruction_stack_pointer_relative_offset_to_accumulator_field.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_Field_ExecuteCycle<T, T2, T3>>;
}

void ClassicVCom_Nova64::Instruction::SubtractInstruction::Stack_Pointer_Relative_Offset_To_Accumulator_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SubtractInstructionStackPointerRelativeOffsetToAccumulatorData
	{
		Word_LE instruction_type;
		Word_LE operand_control_0;
		Word_LE operand_control_1;
		Word_LE unused;
		DWord_LE relative_offset;
	} &subtract_instruction_stack_pointer_relative_offset_to_accumulator = reinterpret_cast<SubtractInstructionStackPointerRelativeOffsetToAccumulatorData &>(data.instruction_data[0]);
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, { subtract_instruction_stack_pointer_relative_offset_to_accumulator.relative_offset, IndexRegisterType::None });
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Data_To_Accumulator_ExecuteCycle>;
}

/*
void ClassicVCom_Nova64::Instruction::IncrementDecrement(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
{
	Word_LE &operand_control = reinterpret_cast<Word_LE &>(instruction_data.data[0]);
	uint8_t operand_type = (operand_control & GenerateFieldBitmask<uint16_t>(0, 1));
	uint8_t inc_dec_control = ((operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
	uint8_t data_size = ((operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			bool register_field_mode = (operand_control & GenerateFieldBitmask<uint16_t>(5, 1));
			uint8_t operand_field_index = 0;
			if (register_field_mode)
			{
				operand_field_index = ((operand_control & GenerateFieldBitmask<uint16_t>(6, 4)) >> 6);
			}
			uint8_t operand_register = instruction_data.data[2];
			switch (operand_register)
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &operand_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<uint8_t>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
						case 1:
						{
							WordField &operand_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<Word_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
						case 2:
						{
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
						case 3:
						{
							IncrementDecrementValue<QWord_LE>(CurrentCPU.GPR_Registers[operand_register], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								++cycles_processed;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								++cycles_processed;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								++cycles_processed;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								++cycles_processed;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], CurrentCPU.FL, inc_dec_control);
							++cycles_processed;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			uint8_t offset_type = ((operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
			uint8_t pointer_type = ((operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_destination = ((operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_destination)
					{
						case 0:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE current_program_id = (CurrentCPU.IP.memory_control & 0xFFF);
							uint8_t region_id = instruction_data.data[2];
							DWord_LE &destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							switch (offset_type)
							{
								case 2:
								{
									destination_offset_data.offset = CurrentCPU.DI.offset;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									if (offset_type == 2)
									{
										destination_offset_data.index_register_used = IndexRegisterType::Destination;
									}
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									if (offset_type == 2)
									{
										destination_offset_data.index_register_used = IndexRegisterType::Destination;
									}
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									if (offset_type == 2)
									{
										destination_offset_data.index_register_used = IndexRegisterType::Destination;
									}
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									if (offset_type == 2)
									{
										destination_offset_data.index_register_used = IndexRegisterType::Destination;
									}
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, destination_pointer, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
						case 1:
						{
							++cycles_processed;
							break;
						}
						case 2:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							Word_LE &destination_memory_group = reinterpret_cast<Word_LE &>(instruction_data.data[2]);
							uint8_t destination_chipset = instruction_data.data[4];
							DWord_LE &destination_pointer = reinterpret_cast<DWord_LE &>(extra_data[0]);
							OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							ChipsetReturnCode destination_chipset_read_return = ChipsetReturnCode::Ok;
							ChipsetReturnCode destination_chipset_write_return = ChipsetReturnCode::Ok;
							switch (offset_type)
							{
								case 2:
								{
									destination_offset_data.offset = CurrentCPU.DI.offset;
									break;
								}
							}
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<uint8_t>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, destination_chipset_read_return);
									if (destination_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
										if (offset_type == 2)
										{
											destination_offset_data.index_register_used = IndexRegisterType::Destination;
										}
										CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<uint8_t>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, data, destination_chipset_write_return);
										if (destination_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
										{
											cycles_processed += 4;
										}
										else
										{
											cycles_processed += 3;
										}
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 1:
								{
									Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, destination_chipset_read_return);
									if (destination_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
										if (offset_type == 2)
										{
											destination_offset_data.index_register_used = IndexRegisterType::Destination;
										}
										CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<Word_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, data, destination_chipset_write_return);
										if (destination_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
										{
											cycles_processed += 4;
										}
										else
										{
											cycles_processed += 3;
										}
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 2:
								{
									DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, destination_chipset_read_return);
									if (destination_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
										if (offset_type == 2)
										{
											destination_offset_data.index_register_used = IndexRegisterType::Destination;
										}
										CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<DWord_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, data, destination_chipset_write_return);
										if (destination_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
										{
											cycles_processed += 4;
										}
										else
										{
											cycles_processed += 3;
										}
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
								case 3:
								{
									QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, destination_chipset_read_return);
									if (destination_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
									{
										IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
										if (offset_type == 2)
										{
											destination_offset_data.index_register_used = IndexRegisterType::Destination;
										}
										CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<QWord_LE>(destination_chipset, destination_memory_group, destination_pointer, destination_offset_data, data, destination_chipset_write_return);
										if (destination_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
										{
											cycles_processed += 4;
										}
										else
										{
											cycles_processed += 3;
										}
									}
									else
									{
										cycles_processed += 2;
									}
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.BP);
					switch (offset_type)
					{
						case 0:
						{
							constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
						case 1:
						{
							DWord_LE &destination_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							OffsetData destination_offset_data = { destination_offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
						case 2:
						{
							OffsetData destination_offset_data = { CurrentCPU.DI.offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(CurrentCPU.SP);
					switch (offset_type)
					{
						case 0:
						{
							constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
						case 1:
						{
							DWord_LE &destination_offset = reinterpret_cast<DWord_LE &>(instruction_data.data[2]);
							OffsetData destination_offset_data = { destination_offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
						case 2:
						{
							OffsetData destination_offset_data = { CurrentCPU.DI.offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									uint8_t data = LoadDataFromSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<uint8_t>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<uint8_t>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 1:
								{
									Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<Word_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 2:
								{
									DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<DWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
								case 3:
								{
									QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data);
									IncrementDecrementValue<QWord_LE>(data, CurrentCPU.FL, inc_dec_control);
									destination_offset_data.index_register_used = IndexRegisterType::Destination;
									StoreDataToSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, destination_offset_data, data);
									cycles_processed += 4;
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Base_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionBaseData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		std::array<uint8_t, 4> data;
	} &increment_decrement_instruction_base = reinterpret_cast<IncrementDecrementInstructionBaseData &>(data.instruction_data[0]);
	uint8_t operand_type = (increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(0, 1));
	// uint8_t inc_dec_control = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
	uint8_t data_size = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(2, 3)) >> 2);
	switch (operand_type)
	{
		case 0:
		{
			uint8_t inc_dec_control = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
			uint8_t &operand_register = increment_decrement_instruction_base.data[0];
			bool register_field_mode = (increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 1));
			uint8_t operand_field_index = 0;
			if (register_field_mode)
			{
				operand_field_index = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(6, 4)) >> 6);
			}
			switch (operand_register)
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &operand_register_field = reinterpret_cast<ByteField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<uint8_t>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							data.callback = nullptr;
							break;
						}
						case 1:
						{
							WordField &operand_register_field = reinterpret_cast<WordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<Word_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.GPR_Registers[operand_register]);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							data.callback = nullptr;
							break;
						}
						case 3:
						{
							IncrementDecrementValue<QWord_LE>(data.CurrentCPU.GPR_Registers[operand_register], data.CurrentCPU.FL, inc_dec_control);
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x10:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SI);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x11:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.DI);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x20:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.BP);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
				case 0x21:
				{
					switch (data_size)
					{
						case 2:
						{
							if (operand_field_index)
							{
								// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
								data.callback = nullptr;
								break;
							}
							DWordField &operand_register_field = reinterpret_cast<DWordField &>(data.CurrentCPU.SP);
							IncrementDecrementValue<DWord_LE>(operand_register_field[operand_field_index], data.CurrentCPU.FL, inc_dec_control);
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 1:
		{
			// uint8_t offset_type = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
			uint8_t pointer_type = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(7, 2)) >> 7);
			uint8_t pointer_destination = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(9, 2)) >> 9);
			switch (pointer_type)
			{
				case 0:
				{
					switch (pointer_destination)
					{
						case 0:
						{
							data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Self_ExecuteCycle_1<QWord_LE>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
							data.callback = nullptr;
							break;
						}
						case 2:
						{
							data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
							switch (data_size)
							{
								case 0:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<uint8_t>;
									break;
								}
								case 1:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<Word_LE>;
									break;
								}
								case 2:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<DWord_LE>;
									break;
								}
								case 3:
								{
									data.callback = Absolute_Pointer_Chipset_ExecuteCycle_1<QWord_LE>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					uint8_t offset_type = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
					switch (offset_type)
					{
						case 0:
						{
							constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Base_Pointer_No_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Base_Pointer_No_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Base_Pointer_No_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Base_Pointer_No_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							DWord_LE &destination_relative_offset = reinterpret_cast<DWord_LE &>(increment_decrement_instruction_base.data[0]);
							OffsetData destination_offset_data = { destination_relative_offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Base_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Base_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Base_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Base_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
						case 2:
						{
							OffsetData destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 2:
				{
					uint8_t offset_type = ((increment_decrement_instruction_base.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
					switch (offset_type)
					{
						case 0:
						{
							constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Stack_Pointer_No_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Stack_Pointer_No_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Stack_Pointer_No_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Stack_Pointer_No_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
						case 1:
						{
							DWord_LE &destination_relative_offset = reinterpret_cast<DWord_LE &>(increment_decrement_instruction_base.data[0]);
							OffsetData destination_offset_data = { destination_relative_offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Stack_Pointer_Relative_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Stack_Pointer_Relative_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Stack_Pointer_Relative_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Stack_Pointer_Relative_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
						case 2:
						{
							OffsetData destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::None };
							switch (data_size)
							{
								case 0:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<uint8_t>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<uint8_t, Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<uint8_t>>>;
									break;
								}
								case 1:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<Word_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<Word_LE, Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<Word_LE>>>;
									break;
								}
								case 2:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<DWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<DWord_LE, Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<DWord_LE>>>;
									break;
								}
								case 3:
								{
									data.CurrentCPU.data_bus = LoadDataFromSystemMemory<QWord_LE>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data);
									data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Pointer_Register_ExecuteCycle<QWord_LE, Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle<QWord_LE>>>;
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_1(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		uint8_t region_id;
		std::array<uint8_t, 3> unused;
		DWord_LE destination_pointer;
	} &increment_decrement_instruction_absolute_pointer_self = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	uint8_t offset_type = ((increment_decrement_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			destination_offset_data.offset = data.CurrentCPU.DI.offset;
			break;
		}
	}
	data.CurrentCPU.data_bus = LoadDataFromSystemMemory<T>(data.CurrentCPU, current_program_id, increment_decrement_instruction_absolute_pointer_self.region_id, increment_decrement_instruction_absolute_pointer_self.destination_pointer, destination_offset_data);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Absolute_Pointer_Self_ExecuteCycle_2<T>>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_2(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
	} &increment_decrement_instruction_absolute_pointer_self = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	uint8_t offset_type = ((increment_decrement_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	IncrementDecrementValue<T>(value, data.CurrentCPU.FL, ((increment_decrement_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1));
	data.CurrentCPU.data_bus = value;
	data.callback = Absolute_Pointer_Self_ExecuteCycle_3<T>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Self_ExecuteCycle_3(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerSelfData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		uint8_t region_id;
		std::array<uint8_t, 3> unused;
		DWord_LE destination_pointer;
	} &increment_decrement_instruction_absolute_pointer_self = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerSelfData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	uint8_t offset_type = ((increment_decrement_instruction_absolute_pointer_self.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	Word_LE current_program_id = (data.CurrentCPU.IP.memory_control & 0xFFF);
	OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	switch (offset_type)
	{
		case 2:
		{
			destination_offset_data.offset = data.CurrentCPU.DI.offset;
			destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_id, increment_decrement_instruction_absolute_pointer_self.region_id, increment_decrement_instruction_absolute_pointer_self.destination_pointer, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_1(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		Word_LE destination_memory_group;
		uint8_t destination_chipset;
		uint8_t unused;
		DWord_LE destination_pointer;
	} &increment_decrement_instruction_absolute_pointer_chipset = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerChipsetData &>(data.instruction_data[0]);
	uint8_t offset_type = ((increment_decrement_instruction_absolute_pointer_chipset.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode destination_chipset_read_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			destination_offset_data.offset = data.CurrentCPU.DI.offset;
			break;
		}
	}
	T value = data.CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<T>(increment_decrement_instruction_absolute_pointer_chipset.destination_chipset, increment_decrement_instruction_absolute_pointer_chipset.destination_memory_group, increment_decrement_instruction_absolute_pointer_chipset.destination_pointer, destination_offset_data, destination_chipset_read_return);
	if (destination_chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
	{
		data.CurrentCPU.data_bus = value;
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<Absolute_Pointer_Chipset_ExecuteCycle_2<T>>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_2(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
	} &increment_decrement_instruction_absolute_pointer_chipset = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerChipsetData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	IncrementDecrementValue<T>(value, data.CurrentCPU.FL, ((increment_decrement_instruction_absolute_pointer_chipset.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1));
	data.CurrentCPU.data_bus = value;
	data.callback = Absolute_Pointer_Chipset_ExecuteCycle_3<T>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Absolute_Pointer_Chipset_ExecuteCycle_3(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionAbsolutePointerChipsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		Word_LE destination_memory_group;
		uint8_t destination_chipset;
		uint8_t unused;
		DWord_LE destination_pointer;
	} &increment_decrement_instruction_absolute_pointer_chipset = reinterpret_cast<IncrementDecrementInstructionAbsolutePointerChipsetData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	uint8_t offset_type = ((increment_decrement_instruction_absolute_pointer_chipset.operand_control & GenerateFieldBitmask<uint16_t>(5, 2)) >> 5);
	OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	ChipsetReturnCode destination_chipset_write_return = ChipsetReturnCode::Ok;
	switch (offset_type)
	{
		case 2:
		{
			destination_offset_data.offset = data.CurrentCPU.DI.offset;
			destination_offset_data.index_register_used = IndexRegisterType::Destination;
			break;
		}
	}
	data.CurrentCPU.CurrentMotherboard->ChipsetWriteToMemoryGroup<T>(increment_decrement_instruction_absolute_pointer_chipset.destination_chipset, increment_decrement_instruction_absolute_pointer_chipset.destination_memory_group, increment_decrement_instruction_absolute_pointer_chipset.destination_pointer, destination_offset_data, value, destination_chipset_write_return);
	if (destination_chipset_write_return == ChipsetReturnCode::MemoryGroupWriteSuccessful)
	{
		data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

template <ClassicVCom_Nova64::DWordCompatible T, void (*callback)(ClassicVCom_Nova64::InstructionCallbackData &)>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Pointer_Register_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionPointerRegisterData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
	} &increment_decrement_instruction_pointer_register = reinterpret_cast<IncrementDecrementInstructionPointerRegisterData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	IncrementDecrementValue<T>(value, data.CurrentCPU.FL, ((increment_decrement_instruction_pointer_register.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1));
	data.CurrentCPU.data_bus = value;
	data.callback = callback;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Base_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Base_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionBasePointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE destination_relative_offset;
	} &increment_decrement_instruction_base_pointer_relative_offset = reinterpret_cast<IncrementDecrementInstructionBasePointerRelativeOffsetData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData destination_offset_data = { increment_decrement_instruction_base_pointer_relative_offset.destination_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Base_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.BP);
	OffsetData destination_offset_data = { data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.BP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Stack_Pointer_No_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	constexpr OffsetData destination_offset_data = { 0, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Stack_Pointer_Relative_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) IncrementDecrementInstructionStackPointerRelativeOffsetData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE destination_relative_offset;
	} &increment_decrement_instruction_stack_pointer_relative_offset = reinterpret_cast<IncrementDecrementInstructionStackPointerRelativeOffsetData &>(data.instruction_data[0]);
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData destination_offset_data = { increment_decrement_instruction_stack_pointer_relative_offset.destination_relative_offset, IndexRegisterType::None };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

template <ClassicVCom_Nova64::DWordCompatible T>
void ClassicVCom_Nova64::Instruction::IncrementDecrementInstruction::Stack_Pointer_Destination_Index_Register_Offset_ExecuteCycle(InstructionCallbackData &data)
{
	T value = data.CurrentCPU.data_bus;
	data.CurrentCPU.data_bus = 0;
	ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(data.CurrentCPU.SP);
	OffsetData destination_offset_data = {data.CurrentCPU.DI.offset, IndexRegisterType::Destination };
	StoreDataToSystemMemory<T>(data.CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, data.CurrentCPU.SP.address, destination_offset_data, value);
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<CommonExecuteCycles::Complete_ExecuteCycle>;
}

/*
void ClassicVCom_Nova64::Instruction::SetClear(CPU &CurrentCPU, BaseInstructionData &instruction_data)
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
}
*/

void ClassicVCom_Nova64::Instruction::SetClearInstruction::Flags1_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SetClearBaseInstructionData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE flags_1;
	} &set_clear_base_instruction = reinterpret_cast<SetClearBaseInstructionData &>(data.instruction_data[0]);
	uint8_t toggle_state = (set_clear_base_instruction.operand_control & GenerateFieldBitmask<uint16_t>(0, 1));
	uint8_t flags_size = ((set_clear_base_instruction.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
	if (toggle_state)
	{
		data.CurrentCPU.FL |= static_cast<QWord_LE>(set_clear_base_instruction.flags_1);
	}
	else
	{
		data.CurrentCPU.FL &= ~(static_cast<QWord_LE>(set_clear_base_instruction.flags_1));
	}
	if (flags_size >= 1)
	{
		data.instruction_data[1] = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
		data.callback = Flags2_ExecuteCycle;
	}
	else
	{
		// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::SetClearInstruction::Flags2_ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) SetClearFlags2InstructionData
	{
		Word_LE instruction_type;
		Word_LE operand_control;
		DWord_LE flags_1;
		DWord_LE flags_2;
	} &set_clear_flags2_instruction = reinterpret_cast<SetClearFlags2InstructionData &>(data.instruction_data[0]);
	uint8_t toggle_state = (set_clear_flags2_instruction.operand_control & GenerateFieldBitmask<uint16_t>(0, 1));
	// uint8_t flags_size = ((set_clear_flags2_instruction.operand_control & GenerateFieldBitmask<uint16_t>(1, 1)) >> 1);
	if (toggle_state)
	{
		data.CurrentCPU.FL |= (static_cast<QWord_LE>(set_clear_flags2_instruction.flags_2) << 32);
	}
	else
	{
		data.CurrentCPU.FL &= ~(static_cast<QWord_LE>(set_clear_flags2_instruction.flags_2) << 32);
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? nullptr : ShadowFetchAndExecuteInstruction::ExecuteCycle_3;
	data.callback = nullptr;
}

/*
void ClassicVCom_Nova64::Instruction::ChipCall(CPU &CurrentCPU, BaseInstructionData &instruction_data)
{
	struct alignas(8) ChipCallInstruction
	{
		Word_LE instruction_type;
		Word_LE virtual_chipset_function_id;
		uint8_t virtual_chipset_port;
	} chipcall_instruction = reinterpret_cast<ChipCallInstruction &>(instruction_data);
	switch (chipcall_instruction.virtual_chipset_port)
	{
		case 0x07:
		{
			FloatingPoint *CurrentFloatingPoint = CurrentCPU.CurrentMotherboard->GetFloatingPoint();
			(*CurrentFloatingPoint)(chipcall_instruction.virtual_chipset_function_id, CurrentCPU.GPR_Registers);
			break;
		}
	}
}
*/

void ClassicVCom_Nova64::Instruction::ChipCallInstruction::ExecuteCycle(InstructionCallbackData &data)
{
	struct alignas(8) ChipCallInstructionData
	{
		Word_LE instruction_type;
		Word_LE virtual_chipset_function_id;
		uint8_t virtual_chipset_port;
	} chipcall_instruction = reinterpret_cast<ChipCallInstructionData &>(data.instruction_data[0]);
	switch (chipcall_instruction.virtual_chipset_port)
	{
		case 0x00:
		{
			GPU *CurrentGPU = data.CurrentCPU.CurrentMotherboard->GetGPU();
			(*CurrentGPU)(chipcall_instruction.virtual_chipset_function_id, data.CurrentCPU.GPR_Registers);
			break;
		}
		case 0x03:
		{
			Input *CurrentInput = data.CurrentCPU.CurrentMotherboard->GetInput();
			(*CurrentInput)(chipcall_instruction.virtual_chipset_function_id, data.CurrentCPU.GPR_Registers);
			break;
		}
		case 0x04:
		{
			Timer *CurrentTimer = data.CurrentCPU.CurrentMotherboard->GetTimer();
			(*CurrentTimer)(chipcall_instruction.virtual_chipset_function_id, data.CurrentCPU.GPR_Registers);
			break;
		}
		case 0x07:
		{
			FloatingPoint *CurrentFloatingPoint = data.CurrentCPU.CurrentMotherboard->GetFloatingPoint();
			(*CurrentFloatingPoint)(chipcall_instruction.virtual_chipset_function_id, data.CurrentCPU.GPR_Registers);
			break;
		}
	}
	// data.callback = !(data.CurrentCPU.SR & 0x08) ? CommonExecuteCycles::Dummy_ExecuteCycle<nullptr> : CommonExecuteCycles::Dummy_ExecuteCycle<ShadowFetchAndExecuteInstruction::ExecuteCycle_3>;
	data.callback = CommonExecuteCycles::Dummy_ExecuteCycle<nullptr>;
}

/*
void ClassicVCom_Nova64::Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_1(InstructionCallbackData &data)
{
	struct alignas(8) ShadowFetchAndExecuteInstructionData
	{
		Word_LE instruction_type;
		uint8_t region;
		uint8_t unused;
		DWord_LE pointer;
	} &shadow_fetch_and_execute_instruction = reinterpret_cast<ShadowFetchAndExecuteInstructionData &>(data.instruction_data[0]);
	if (shadow_fetch_and_execute_instruction.pointer % sizeof(QWord_LE) == 0 && shadow_fetch_and_execute_instruction.region < 15)
	{
		data.current_data.push_back(std::bit_cast<QWord_LE>(data.CurrentCPU.IP));
		data.CurrentCPU.IP.address = shadow_fetch_and_execute_instruction.pointer;
		SetProgramRegion(data.CurrentCPU.IP, shadow_fetch_and_execute_instruction.region);
		QWord_LE target_instruction = data.CurrentCPU.FastFetchExtraData<QWord_LE>();
		data.current_data.push_back(target_instruction);
		data.callback = ExecuteCycle_2;
	}
	else
	{
		data.callback = nullptr;
	}
}

void ClassicVCom_Nova64::Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_2(InstructionCallbackData &data)
{
	struct alignas(8) TargetInstructionData
	{
		Word_LE instruction_type;
	} target_instruction = std::bit_cast<TargetInstructionData>(data.current_data.back());
	data.current_data.pop_back();
	MPRegisterData tmp = std::bit_cast<MPRegisterData>(data.current_data.back());
	if (static_cast<InstructionType>(static_cast<uint16_t>(target_instruction.instruction_type)) == InstructionType::ShadowFetchAndExecute)
	{
		data.current_data.pop_back();
		data.CurrentCPU.IP = tmp;
		data.callback = nullptr;
	}
	else
	{
		data.CurrentCPU.SR |= 0x08;
		data.instruction_data[0] = std::bit_cast<QWord_LE>(target_instruction);
		data.callback = data.CurrentCPU.instruction_base_callback_table[target_instruction.instruction_type].callback;
	}
}

void ClassicVCom_Nova64::Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_3(InstructionCallbackData &data)
{
	MPRegisterData tmp = std::bit_cast<MPRegisterData>(data.current_data.back());
	data.current_data.pop_back();
	data.CurrentCPU.SR &= ~(0x08);
	if (!(data.CurrentCPU.SR & 0x01))
	{
		data.CurrentCPU.IP = tmp;
	}
	else
	{
		if (!(data.CurrentCPU.SR & 0x02))
		{
			data.CurrentCPU.IP.memory_control = tmp.memory_control;
		}
	}
	data.callback = nullptr;
}
*/
