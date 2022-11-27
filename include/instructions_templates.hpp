#ifndef _INSTRUCTIONS_TEMPLATES_HPP_
#define _INSTRUCTIONS_TEMPLATES_HPP_

#include "types.hpp"
#include "bits.hpp"
#include "cpu.hpp"
#include "instructions.hpp"
#include <cstdint>
#include <concepts>

namespace ClassicVCom_Nova64
{
	namespace Instruction
	{
		template <void (*ArithmeticFuncByte)(uint8_t &, uint8_t &, QWord_LE &, bool, bool), void (*ArithmeticFuncWord)(Word_LE &, Word_LE &, QWord_LE &, bool, bool), void (*ArithmeticFuncDWord)(DWord_LE &, DWord_LE &, QWord_LE &, bool, bool), void (*ArithmeticFuncQWord)(QWord_LE &, QWord_LE &, QWord_LE &, bool, bool)>
		void AddSubtract(CPU &CurrentCPU, BaseInstructionData &instruction_data, uint32_t &cycles_processed)
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
			bool carry_borrow_mode = (operand_control[0] & GenerateFieldBitmask<uint16_t>(7, 1));
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
									ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
									++cycles_processed;
									break;
								}
								case 1:
								{
									WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
									WordField &operand_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[operand_register]);
									ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
									++cycles_processed;
									break;
								}
								case 2:
								{
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[operand_register]);
									ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
									++cycles_processed;
									break;
								}
								case 3:
								{
									ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], CurrentCPU.GPR_Registers[operand_register], CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SI);
									ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.DI);
									ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.BP);
									ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
									DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
									DWordField &operand_register_field = reinterpret_cast<DWordField &>(CurrentCPU.SP);
									ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], operand_register_field[operand_field_index], CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
					switch (data_size)
					{
						case 0:
						{
							ByteField &accumulator_register_field = reinterpret_cast<ByteField &>(CurrentCPU.GPR_Registers[0]);
							ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], instruction_data.data[4], CurrentCPU.FL, signed_mode, carry_borrow_mode);
							++cycles_processed;
							break;
						}
						case 1:
						{
							WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
							Word_LE &immediate_value = reinterpret_cast<Word_LE &>(instruction_data.data[4]);
							ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], immediate_value, CurrentCPU.FL, signed_mode, carry_borrow_mode);
							++cycles_processed;
							break;
						}
						case 2:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
							DWord_LE &immediate_value = reinterpret_cast<DWord_LE &>(extra_data[0]);
							ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], immediate_value, CurrentCPU.FL, signed_mode, carry_borrow_mode);
							++cycles_processed;
							break;
						}
						case 3:
						{
							ExtraQWordData<1> extra_data = CurrentCPU.FetchExtraData<1>();
							QWord_LE &immediate_value = reinterpret_cast<QWord_LE &>(extra_data[0]);
							ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], immediate_value, CurrentCPU.FL, signed_mode, carry_borrow_mode);
							++cycles_processed;
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_id, region_id, source_pointer, source_offset_data);
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
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
												ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<Word_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
											if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
											{
												ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<DWord_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
											if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
											{
												ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
											QWord_LE data = CurrentCPU.CurrentMotherboard->ChipsetReadFromMemoryGroup<QWord_LE>(chipset, current_memory_group, source_pointer, source_offset_data, chipset_read_return);
											if (chipset_read_return == ChipsetReturnCode::MemoryGroupReadSuccessful)
											{
												ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);

											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.BP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { 0, IndexRegisterType::None });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { source_offset, IndexRegisterType::None });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
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
											ArithmeticFuncByte(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 1:
										{
											WordField &accumulator_register_field = reinterpret_cast<WordField &>(CurrentCPU.GPR_Registers[0]);
											Word_LE data = LoadDataFromSystemMemory<Word_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 2:
										{
											DWordField &accumulator_register_field = reinterpret_cast<DWordField &>(CurrentCPU.GPR_Registers[0]);
											DWord_LE data = LoadDataFromSystemMemory<DWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncDWord(accumulator_register_field[accumulator_field_index], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
											cycles_processed += 2;
											break;
										}
										case 3:
										{
											QWord_LE data = LoadDataFromSystemMemory<QWord_LE>(CurrentCPU, current_program_memory_control.program_id, current_program_memory_control.region_id, CurrentCPU.SP.address, { CurrentCPU.SI.offset, IndexRegisterType::Source });
											ArithmeticFuncQWord(CurrentCPU.GPR_Registers[0], data, CurrentCPU.FL, signed_mode, carry_borrow_mode);
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
		}
	}
}

#endif
