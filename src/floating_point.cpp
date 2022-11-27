#include "floating_point.hpp"
#include <cmath>

ClassicVCom_Nova64::FloatingPoint::FloatingPoint()
{
	const SinglePrecisionFloatingPointData data = { 0.0f, 0.0f };
	for (uint32_t i = 0; i < 8; ++i)
	{
		Registers.floating_point_registers[i] = std::bit_cast<double>(data);
	}
}

ClassicVCom_Nova64::FloatingPoint::~FloatingPoint()
{
}

void ClassicVCom_Nova64::FloatingPoint::operator()(Word_LE &function, std::array<QWord_LE, 8> &args)
{
	switch (static_cast<FloatingPointChipsetFunction>(static_cast<uint16_t>(function)))
	{
		case FloatingPointChipsetFunction::Add:
		case FloatingPointChipsetFunction::Subtract:
		case FloatingPointChipsetFunction::Multiply:
		case FloatingPointChipsetFunction::Divide:
		{
			struct OperandControlData
			{
				uint8_t control_flag;
				std::array<uint8_t, 7> operand;
			} operand_control = std::bit_cast<OperandControlData>(args[0]);
			uint8_t operand_count = (((operand_control.control_flag & 0x06) >> 1) + 2);
			if (!(operand_control.control_flag & 0x01))
			{
				struct FloatingPointRegisterOperandData
				{
					uint8_t floating_point_register;
					uint8_t field;
				};
				struct SourceFloatingPointRegisterOperandData
				{
					FloatingPointRegisterOperandData o_0, o_1, o_2, o_3, o_4;
					
					FloatingPointRegisterOperandData &operator[](int index)
					{
						switch (index)
						{
							case 0: { return o_0; }
							case 1: { return o_1; }
							case 2: { return o_2; }
							case 3: { return o_3; }
							case 4: { return o_4; }
						}
						return o_0;
					}
				};
				SourceFloatingPointRegisterOperandData source_floating_point_register_operands =
				{
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[0] & 0x07), static_cast<uint8_t>((operand_control.operand[0] & 0x08) >> 3) },
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[1] & 0x07), static_cast<uint8_t>((operand_control.operand[1] & 0x08) >> 3) },
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[2] & 0x07), static_cast<uint8_t>((operand_control.operand[2] & 0x08) >> 3) },
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[3] & 0x07), static_cast<uint8_t>((operand_control.operand[3] & 0x08) >> 3) },
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[4] & 0x07), static_cast<uint8_t>((operand_control.operand[4] & 0x08) >> 3) }
				};
				FloatingPointRegisterOperandData destination_floating_point_register_operand =
				{
					static_cast<uint8_t>(operand_control.operand[5] & 0x07), static_cast<uint8_t>((operand_control.operand[5] & 0x08) >> 3)
				};
				if (Registers.floating_point_control_flag_register & (0x01 << (operand_control.operand[0] & 0x07)))
				{
					break;
				}
				SinglePrecisionFloatingPointData CurrentSourceData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[source_floating_point_register_operands[0].floating_point_register]);
				SinglePrecisionFloatingPointData CurrentDestinationData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[destination_floating_point_register_operand.floating_point_register]);
				float result = CurrentSourceData.field[source_floating_point_register_operands[0].field];
				bool fail = false;
				for (uint8_t i = 1; i < operand_count; ++i)
				{
					if (Registers.floating_point_control_flag_register & (0x01 << (operand_control.operand[i] & 0x07)))
					{
						fail = true;
						break;
					}
					CurrentSourceData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[source_floating_point_register_operands[i].floating_point_register]);
					switch (static_cast<FloatingPointChipsetFunction>(static_cast<uint16_t>(function)))
					{
						case FloatingPointChipsetFunction::Add:
						{
							result += CurrentSourceData.field[source_floating_point_register_operands[i].field];
							break;
						}
						case FloatingPointChipsetFunction::Subtract:
						{
							result -= CurrentSourceData.field[source_floating_point_register_operands[i].field];
							break;
						}
						case FloatingPointChipsetFunction::Multiply:
						{
							result *= CurrentSourceData.field[source_floating_point_register_operands[i].field];
							break;
						}
						case FloatingPointChipsetFunction::Divide:
						{
							result /= CurrentSourceData.field[source_floating_point_register_operands[i].field];
							break;
						}
					}
				}
				if (fail)
				{
					break;
				}
				CurrentDestinationData.field[destination_floating_point_register_operand.field] = result;
				Registers.floating_point_registers[destination_floating_point_register_operand.floating_point_register] = std::bit_cast<double>(CurrentDestinationData);
			}
			else
			{
				if (!(Registers.floating_point_control_flag_register & (0x01 << (operand_control.operand[0] & 0x07))))
				{
					break;
				}
				double result = Registers.floating_point_registers[operand_control.operand[0]];
				bool fail = false;
				for (uint8_t i = 1; i < operand_count; ++i)
				{
					if (!(Registers.floating_point_control_flag_register & (0x01 << (operand_control.operand[i] & 0x07))))
					{
						fail = true;
						break;
					}
					switch (static_cast<FloatingPointChipsetFunction>(static_cast<uint16_t>(function)))
					{
						case FloatingPointChipsetFunction::Add:
						{
							result += Registers.floating_point_registers[operand_control.operand[i] & 0x07];
							break;
						}
						case FloatingPointChipsetFunction::Subtract:
						{
							result -= Registers.floating_point_registers[operand_control.operand[i] & 0x07];
							break;
						}
						case FloatingPointChipsetFunction::Multiply:
						{
							result *= Registers.floating_point_registers[operand_control.operand[i] & 0x07];
							break;
						}
						case FloatingPointChipsetFunction::Divide:
						{
							result /= Registers.floating_point_registers[operand_control.operand[i] & 0x07];
							break;
						}
					}
				}
				if (fail)
				{
					break;
				}
				Registers.floating_point_registers[operand_control.operand[5] & 0x07] = result;
			}
			break;
		}
		case FloatingPointChipsetFunction::Exponent:
		{
			struct OperandControlData
			{
				uint8_t control_flag;
				std::array<uint8_t, 7> operand;
			} operand_control = std::bit_cast<OperandControlData>(args[0]);
			if (!(operand_control.control_flag & 0x01))
			{
				struct FloatingPointRegisterOperandData
				{
					uint8_t floating_point_register;
					uint8_t field;
				};
				FloatingPointRegisterOperandData base_floating_point_register_operand =
				{
					static_cast<uint8_t>(operand_control.operand[0] & 0x07), static_cast<uint8_t>((operand_control.operand[0] & 0x08) >> 3)
				};
				FloatingPointRegisterOperandData exponent_floating_point_register_operand =
				{
					static_cast<uint8_t>(operand_control.operand[1] & 0x07), static_cast<uint8_t>((operand_control.operand[1] & 0x08) >> 3)
				};
				FloatingPointRegisterOperandData destination_floating_point_register_operand =
				{
					static_cast<uint8_t>(operand_control.operand[2] & 0x07), static_cast<uint8_t>((operand_control.operand[2] & 0x08) >> 3)
				};
				SinglePrecisionFloatingPointData BaseData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[base_floating_point_register_operand.floating_point_register]);
				SinglePrecisionFloatingPointData ExponentData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[exponent_floating_point_register_operand.floating_point_register]);
				SinglePrecisionFloatingPointData CurrentDestinationData = std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[destination_floating_point_register_operand.floating_point_register]);
				CurrentDestinationData.field[destination_floating_point_register_operand.field] = std::pow(BaseData.field[base_floating_point_register_operand.field], ExponentData.field[exponent_floating_point_register_operand.field]);
				Registers.floating_point_registers[destination_floating_point_register_operand.floating_point_register] = std::bit_cast<double>(CurrentDestinationData);
			}
			else
			{
				Registers.floating_point_registers[operand_control.operand[2] & 0x07] = std::pow(Registers.floating_point_registers[operand_control.operand[0] & 0x07], Registers.floating_point_registers[operand_control.operand[1] & 0x07]);
			}
			break;
		}
		case FloatingPointChipsetFunction::Compare:
		{
			struct OperandControlData
			{
				uint8_t control_flag;
				std::array<uint8_t, 7> operand;
			} operand_control = std::bit_cast<OperandControlData>(args[0]);
			Registers.floating_point_status_flag_register &= ~(0x07);
			if (!(operand_control.control_flag & 0x01))
			{
				struct FloatingPointRegisterOperandData
				{
					uint8_t floating_point_register;
					uint8_t field;
				};
				struct DualFloatingPointRegisterOperandData
				{
					FloatingPointRegisterOperandData o_0, o_1;

					FloatingPointRegisterOperandData &operator[](int index)
					{
						switch (index)
						{
							case 0: { return o_0; }
							case 1: { return o_1; }
						}
						return o_0;
					}
				};
				struct DualFloatingPointData
				{
					SinglePrecisionFloatingPointData fp_0, fp_1;

					SinglePrecisionFloatingPointData &operator[](int index)
					{
						switch (index)
						{
							case 0: { return fp_0; }
							case 1: { return fp_1; }
						}
						return fp_0;
					}
				};
				DualFloatingPointRegisterOperandData floating_point_register_operands =
				{
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[0] & 0x07), static_cast<uint8_t>((operand_control.operand[0] & 0x08) >> 3) },
					FloatingPointRegisterOperandData { static_cast<uint8_t>(operand_control.operand[1] & 0x07), static_cast<uint8_t>((operand_control.operand[1] & 0x08) >> 3) }
				};
				if ((Registers.floating_point_control_flag_register & ((0x01 << operand_control.operand[0]) & 0x07)) || (Registers.floating_point_control_flag_register & ((0x01 << operand_control.operand[1]) & 0x07)))
				{
					break;
				}
				DualFloatingPointData FloatingPointData = {
					std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[floating_point_register_operands[0].floating_point_register]),
					std::bit_cast<SinglePrecisionFloatingPointData>(Registers.floating_point_registers[floating_point_register_operands[1].floating_point_register])
				};
				if (FloatingPointData[0].field[floating_point_register_operands[0].field] == FloatingPointData[1].field[floating_point_register_operands[1].field])
				{
					Registers.floating_point_status_flag_register |= 0x01;
				}
				else if (FloatingPointData[0].field[floating_point_register_operands[0].field] < FloatingPointData[1].field[floating_point_register_operands[1].field])
				{
					Registers.floating_point_status_flag_register |= 0x02;
				}
				else
				{
					Registers.floating_point_status_flag_register |= 0x04;
				}
			}
			else
			{
				if (Registers.floating_point_registers[operand_control.operand[0] & 0x07] == Registers.floating_point_registers[operand_control.operand[1] & 0x07])
				{
					Registers.floating_point_status_flag_register |= 0x01;
				}
				else if (Registers.floating_point_registers[operand_control.operand[0] & 0x07] < Registers.floating_point_registers[operand_control.operand[1] & 0x07])
				{
					Registers.floating_point_status_flag_register |= 0x02;
				}
				else
				{
					Registers.floating_point_status_flag_register |= 0x04;
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::FloatingPoint::SetFloatingPointControlFlag(uint64_t floating_point_control_flag)
{
	const SinglePrecisionFloatingPointData data = { 0.0f, 0.0f };
	for (uint32_t i = 0; i < 8; ++i)
	{
		uint64_t flag_state = (floating_point_control_flag & (0x01 << i));
		if (flag_state != (Registers.floating_point_control_flag_register & (0x01 << i)))
		{
			Registers.floating_point_registers[i] = 0.0;
		}
		else
		{
			Registers.floating_point_registers[i] = std::bit_cast<double>(data);
		}
	}
	Registers.floating_point_control_flag_register = floating_point_control_flag;
}
