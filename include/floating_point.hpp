#ifndef _FLOATING_POINT_HPP_
#define _FLOATING_POINT_HPP_

#include "chipset.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include <cstring>
#include <array>
#include <bit>
#include <numbers>

namespace ClassicVCom_Nova64
{
	enum class FloatingPointChipsetFunction
	{
		Add = 0,
		Subtract = 1,
		Multiply = 2,
		Divide = 3,
		Exponent = 4,
		Compare = 32
	};

	struct SinglePrecisionFloatingPointData
	{
		std::array<float, 2> field;
	};

	struct alignas(8) DoublePrecisionFloatingPointRegisterData
	{
		double f_0, f_1, f_2, f_3, f_4, f_5, f_6, f_7;

		double &operator[](int index)
		{
			switch (index)
			{
				case 0:	{ return f_0; }
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

	class FloatingPoint
	{
		public:
			FloatingPoint();
			~FloatingPoint();
			void operator()(Word_LE &function, std::array<QWord_LE, 8> &args);

			template <WordMaximumRequired T>
			T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T result = 0;
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						size_t data_read = sizeof(T);
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
						if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
						{
							data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
						}
						switch (current_register)
						{
							case 0:
							{
								const uint8_t *FloatingPointControlFlagRegister = reinterpret_cast<const uint8_t *>(&Registers.floating_point_control_flag_register);
								memcpy(&result, &FloatingPointControlFlagRegister[current_register_offset], data_read);
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
					case 2:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return result;
			}

			template <DWordRequired T>
			T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T result = 0;
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 0:
							{
								size_t data_read = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								const uint8_t *FloatingPointControlFlagRegister = reinterpret_cast<const uint8_t *>(&Registers.floating_point_control_flag_register);
								memcpy(&result, &FloatingPointControlFlagRegister[current_register_offset], data_read);
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 1:
							case 2:
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
							case 8:
							{
								if (current_address % sizeof(DWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t current_floating_point_register = current_register - 1;
								if (!(Registers.floating_point_control_flag_register & (0x01 << current_floating_point_register)))
								{
									std::array<DWord_Native, 2> floating_point_register_data = std::bit_cast<std::array<DWord_Native, 2>>(Registers.floating_point_registers[current_floating_point_register]);
									result = floating_point_register_data[(current_address / sizeof(DWord_LE)) % 2];
								}
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address % sizeof(DWord_LE))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						DWord_LE constant_index = current_address / sizeof(DWord_LE);
						switch (constant_index)
						{
							case 0:
							{
								constexpr DWord_Native pi = std::bit_cast<DWord_Native>(static_cast<float>(std::numbers::pi));
								result = pi;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 1:
							{
								constexpr DWord_Native e = std::bit_cast<DWord_Native>(static_cast<float>(std::numbers::e));
								result = e;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 2:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return result;
			}

			template <QWordRequired T>
			T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
			{
				T result = 0;
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 0:
							{
								size_t data_read = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_read -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								const uint8_t *FloatingPointControlFlagRegister = reinterpret_cast<const uint8_t *>(&Registers.floating_point_control_flag_register);
								memcpy(&result, &FloatingPointControlFlagRegister[current_register_offset], data_read);
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 1:
							case 2:
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
							case 8:
							{
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t current_floating_point_register = current_register - 1;
								QWord_Native floating_point_register_data = std::bit_cast<QWord_Native>(Registers.floating_point_registers[current_floating_point_register]);
								result = floating_point_register_data;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					case 1:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
					case 2:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address % sizeof(QWord_LE))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						DWord_LE constant_index = current_address / sizeof(QWord_LE);
						switch (constant_index)
						{
							case 0:
							{
								constexpr QWord_Native pi = std::bit_cast<QWord_Native>(std::numbers::pi);
								result = pi;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 1:
							{
								constexpr QWord_Native e = std::bit_cast<QWord_Native>(std::numbers::e);
								result = e;
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							default:
							{
								read_return = ChipsetReturnCode::MemoryGroupReadFailed;
								break;
							}
						}
						break;
					}
					default:
					{
						read_return = ChipsetReturnCode::MemoryGroupReadFailed;
						break;
					}
				}
				return result;
			}

			template <WordMaximumRequired T>
			void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
			{
				switch (group)
				{
					case 0:
					{
						break;
					}
				}
			}

			template <DWordRequired T>
			void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
			{
				switch (group)
				{
					case 0:
					{
						break;
					}
				}
			}

			template <QWordRequired T>
			void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
			{
				switch (group)
				{
					case 0:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= sizeof(Registers))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t current_register = static_cast<uint8_t>(current_address / sizeof(QWord_LE));
						switch (current_register)
						{
							case 0:
							{
								size_t data_write = sizeof(T);
								uint8_t current_register_offset = static_cast<uint8_t>(current_address % sizeof(QWord_LE));
								if (current_register_offset + sizeof(T) > sizeof(QWord_LE))
								{
									data_write -= (current_register_offset + sizeof(T)) - sizeof(QWord_LE);
								}
								QWord_LE old_floating_point_control_flag_register = Registers.floating_point_control_flag_register;
								uint8_t *FloatingPointControlFlagRegister = reinterpret_cast<uint8_t *>(&Registers.floating_point_control_flag_register);
								memcpy(&FloatingPointControlFlagRegister[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								if (Registers.floating_point_control_flag_register != old_floating_point_control_flag_register)
								{
									const SinglePrecisionFloatingPointData initial_data = { 0.0, 0.0 };
									for (uint32_t i = 0; i < 8; ++i)
									{
										uint64_t flag_state = (Registers.floating_point_control_flag_register & (0x01 << i));
										if (flag_state != (old_floating_point_control_flag_register & (0x01 << i)))
										{
											Registers.floating_point_registers[i] = flag_state ? 0.0 : std::bit_cast<double>(initial_data);	
										}
									}
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							case 1:
							case 2:
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
							case 8:
							{
								if (current_address % sizeof(QWord_LE))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								QWord_Native floating_point_data = data;
								Registers.floating_point_registers[current_register - 1] = std::bit_cast<double>(floating_point_data);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							default:
							{
								write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
								break;
							}
						}
						break;
					}
					default:
					{
						write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
						break;
					}
				}
			}

			void SetFloatingPointControlFlag(uint64_t floating_point_control_flag);

			template <HasChipsetId T>
			friend std::string GetId(T &chipset);
		private:
			const std::string id = "NovaFloat-64";
			struct
			{
				QWord_LE floating_point_control_flag_register;
				DoublePrecisionFloatingPointRegisterData floating_point_registers;
				QWord_LE floating_point_status_flag_register;
			} Registers;
	};
}

#endif
