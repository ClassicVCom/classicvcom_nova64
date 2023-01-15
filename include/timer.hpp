#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include "chipset.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <array>
#include <thread>
#include <fmt/core.h>

namespace ClassicVCom_Nova64
{
	enum class TimerChipsetFunction
	{
		ResetTimerCounters = 0
	};

	struct TimerTickRateData
	{
		std::array<DWord_LE, 2> timer_tick_rate;	
	};

	struct Timer32BitModeData
	{
		uint32_t timer_0, timer_1;

		uint32_t &operator[](int index)
		{
			switch (index)
			{
				case 0: { return timer_0; }
				case 1: { return timer_1; }
			}
			return timer_0;
		}
	};

	struct TimerSyncData
	{
		DWord_LE cycles_per_tick;
		DWord_LE cycle_counter;
	};

	class Timer
	{
		public:
			Timer();
			~Timer();
			void operator()(Word_LE &function, std::array<QWord_LE, 8> &args);

			template <WordMaximumRequired T>
			inline T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
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
								const uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<const uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&result, &timer_rtc_flags_register_ptr[current_register_offset], data_read);
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
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<const uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&result, &interrupt_descriptor_table_ptr[current_address], sizeof(T));
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
				return result;
			}

			template <DWordRequired T>
			inline T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
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
								const uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<const uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&result, &timer_rtc_flags_register_ptr[current_register_offset], data_read);
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
							{
								if (current_address % sizeof(DWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t current_timer_tick_rate_register = current_register - 1;
								uint8_t timer_tick_rate_index = current_timer_tick_rate_register % 2;
								memcpy(&result, &Registers.TimerTickRates[current_timer_tick_rate_register].timer_tick_rate[timer_tick_rate_index], sizeof(T));
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 5:
							case 6:
							case 7:
							case 8:
							{
								uint8_t current_timer_start_register = current_register - 5;
								if (!(Registers.timer_rtc_flags & (0x10000 << current_timer_start_register)))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								if (current_address % sizeof(DWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t timer_start_index = current_timer_start_register % 2;
								Timer32BitModeData &current_timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[current_timer_start_register]);
								memcpy(&result, &current_timer_start[timer_start_index], sizeof(T));
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 9:
							case 10:
							case 11:
							case 12:
							{
								uint8_t current_timer_counter_register = current_register - 9;
								if (!(Registers.timer_rtc_flags & (0x10000 << current_timer_counter_register)))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								if (current_address % sizeof(DWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t timer_counter_index = current_timer_counter_register % 2;
								Timer32BitModeData &current_timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[current_timer_counter_register]);
								memcpy(&result, &current_timer_counter[timer_counter_index], sizeof(T));
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
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<const uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&result, &interrupt_descriptor_table_ptr[current_address], sizeof(T));
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
				return result;
			}

			template <QWordRequired T>
			inline T ReadFromMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, ChipsetReturnCode &read_return)
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
								const uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<const uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&result, &timer_rtc_flags_register_ptr[current_register_offset], data_read);
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
							{
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t current_timer_tick_rate_register = current_register - 1;
								memcpy(&result, &Registers.TimerTickRates[current_timer_tick_rate_register], sizeof(T));
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 5:
							case 6:
							case 7:
							case 8:
							{
								uint8_t current_timer_start_register = current_register - 5;
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								memcpy(&result, &Registers.timer_start[current_timer_start_register], sizeof(T));
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 9:
							case 10:
							case 11:
							case 12:
							{
								uint8_t current_timer_counter_register = current_register - 9;
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								memcpy(&result, &Registers.timer_counter[current_timer_counter_register], sizeof(T));
								if ((cpu.FL & 0x800) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x200) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								read_return = ChipsetReturnCode::MemoryGroupReadSuccessful;
								break;
							}
							case 13:
							{
								if (current_address % sizeof(QWord_LE))
								{
									read_return = ChipsetReturnCode::MemoryGroupReadFailed;
									break;
								}
								uint8_t rtc_read_ctrl_flags = ((Registers.timer_rtc_flags & 0x300000000) >> 32);
								std::chrono::time_point real_time_clock = std::chrono::system_clock::now();
								switch (rtc_read_ctrl_flags)
								{
									case 0:
									{
										std::chrono::seconds rtc_seconds = std::chrono::duration_cast<std::chrono::seconds>(real_time_clock.time_since_epoch());
										result = rtc_seconds.count();
										break;
									}
									case 1:
									{
										std::chrono::milliseconds rtc_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(real_time_clock.time_since_epoch());
										result = rtc_milliseconds.count();
										break;
									}
									case 2:
									{
										std::chrono::microseconds rtc_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(real_time_clock.time_since_epoch());
										result = rtc_microseconds.count();
										break;
									}
									case 3:
									{
										std::chrono::nanoseconds rtc_nanoseconds = real_time_clock.time_since_epoch();
										result = rtc_nanoseconds.count();
										break;
									}
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
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							read_return = ChipsetReturnCode::MemoryGroupReadFailed;
							break;
						}
						const uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<const uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&result, &interrupt_descriptor_table_ptr[current_address], sizeof(T));
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
				return result;
			}

			template <WordMaximumRequired T>
			inline void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
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
								uint64_t old_timer_rtc_flags = Registers.timer_rtc_flags;
								uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&timer_rtc_flags_register_ptr[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								for (uint8_t i = 0; i < 4; ++i)
								{
									if ((old_timer_rtc_flags & (0x10000 << i)) != (Registers.timer_rtc_flags & (0x10000 << i)))
									{
										if (Registers.timer_rtc_flags & (0x01 << i))
										{
											Registers.timer_rtc_flags &= ~(0x01 << i);
										}
										if (Registers.timer_rtc_flags & (0x10 << i))
										{
											Registers.timer_rtc_flags &= ~(0x10 << i);
										}
									}
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
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&interrupt_descriptor_table_ptr[current_address], &data, sizeof(T));
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
			}

			template <DWordRequired T>
			inline void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
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
								uint64_t old_timer_rtc_flags = Registers.timer_rtc_flags;
								uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&timer_rtc_flags_register_ptr[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								for (uint8_t i = 0; i < 4; ++i)
								{
									if ((old_timer_rtc_flags & (0x10000 << i)) != (Registers.timer_rtc_flags & (0x10000 << i)))
									{
										if (Registers.timer_rtc_flags & (0x01 << i))
										{
											Registers.timer_rtc_flags &= ~(0x01 << i);
										}
										if (Registers.timer_rtc_flags & (0x10 << i))
										{
											Registers.timer_rtc_flags &= ~(0x10 << i);
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
							{
								if (current_address % sizeof(DWord_LE))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								uint8_t current_timer_tick_rate = current_register - 1;
								uint8_t current_timer_tick_index = current_address / sizeof(DWord_LE);
								if (data < 1)
								{
									data = 1;
								}
								else if (data > 2000000)
								{
									data = 2000000;
								}
								uint32_t old_tick_rate = Registers.TimerTickRates[current_timer_tick_rate].timer_tick_rate[current_timer_tick_index];
								uint8_t current_timer = (current_timer_tick_index << 2) + current_timer_tick_rate;
								Registers.TimerTickRates[current_timer_tick_rate].timer_tick_rate[current_timer_tick_index] = data;
								timer_sync[current_timer].cycles_per_tick = cpu.cycles_per_second / static_cast<double>(data);
								timer_sync[current_timer].cycle_counter = static_cast<double>(timer_sync[current_timer].cycle_counter) * (static_cast<double>(data) / static_cast<double>(old_tick_rate));
								if (Registers.timer_rtc_flags & (0x01 << current_timer))
								{
									Registers.timer_rtc_flags &= ~(0x01 << current_timer);
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							case 5:
							case 6:
							case 7:
							case 8:
							{
								uint8_t current_timer_start = current_register - 5;
								if (current_address % sizeof(DWord_LE))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								if (!(Registers.timer_rtc_flags & (0x10000 << current_timer_start)))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								uint8_t current_timer_start_index = current_address / sizeof(DWord_LE);
								Timer32BitModeData &timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[current_timer_start]);
								Timer32BitModeData &timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[current_timer_start]);
								timer_start[current_timer_start_index] = data;
								timer_counter[current_timer_start_index] = data;
								timer_sync[(current_timer_start_index << 2) + current_timer_start].cycle_counter = 0;
								if (Registers.timer_rtc_flags & (0x01 << ((current_timer_start_index << 2) + current_timer_start)))
								{
									Registers.timer_rtc_flags &= ~(0x01 << ((current_timer_start_index << 2) + current_timer_start));
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
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&interrupt_descriptor_table_ptr[current_address], &data, sizeof(T));
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
			}


			template <QWordRequired T>
			inline void WriteToMemoryGroup(CPU &cpu, Word_LE group, DWord_LE address, OffsetData offset_data, T data, ChipsetReturnCode &write_return)
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
								uint64_t old_timer_rtc_flags = Registers.timer_rtc_flags;
								uint8_t *timer_rtc_flags_register_ptr = reinterpret_cast<uint8_t *>(&Registers.timer_rtc_flags);
								memcpy(&timer_rtc_flags_register_ptr[current_register_offset], &data, data_write);
								if ((cpu.FL & 0x1000) && offset_data.index_register_used != IndexRegisterType::None)
								{
									IndexRegisterData &target_index_register = (offset_data.index_register_used == IndexRegisterType::Source) ? cpu.SI : cpu.DI;
									target_index_register.offset = ((cpu.FL & 0x400) ? target_index_register.offset - sizeof(T) : target_index_register.offset + sizeof(T));
								}
								for (uint8_t i = 0; i < 4; ++i)
								{
									if ((old_timer_rtc_flags & (0x10000 << i)) != (Registers.timer_rtc_flags & (0x10000 << i)))
									{
										if (Registers.timer_rtc_flags & (0x01 << i))
										{
											Registers.timer_rtc_flags &= ~(0x01 << i);
										}
										if (Registers.timer_rtc_flags & (0x10 << i))
										{
											Registers.timer_rtc_flags &= ~(0x10 << i);
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
							{
								if (current_address % sizeof(QWord_LE))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								uint8_t current_timer_tick_rate = current_register - 1;
								TimerTickRateData &timer_tick_rate_data = reinterpret_cast<TimerTickRateData &>(data);
								for (uint8_t i = 0; i < 2; ++i)
								{
									if (timer_tick_rate_data.timer_tick_rate[i] < 1)
									{
										timer_tick_rate_data.timer_tick_rate[i] = 1;
									}
									else if (timer_tick_rate_data.timer_tick_rate[i] > 2000000)
									{
										timer_tick_rate_data.timer_tick_rate[i] = 2000000;
									}
									uint32_t old_tick_rate = Registers.TimerTickRates[current_timer_tick_rate].timer_tick_rate[i];
									uint8_t current_timer = (i << 2) + current_timer_tick_rate;
									Registers.TimerTickRates[current_timer_tick_rate].timer_tick_rate[i] = timer_tick_rate_data.timer_tick_rate[i];
									timer_sync[current_timer].cycles_per_tick = cpu.cycles_per_second / static_cast<double>(timer_tick_rate_data.timer_tick_rate[i]);
									timer_sync[current_timer].cycle_counter = static_cast<double>(timer_sync[current_timer].cycle_counter) * (static_cast<double>(timer_tick_rate_data.timer_tick_rate[i]) / static_cast<double>(old_tick_rate));
									if (Registers.timer_rtc_flags & (0x01 << current_timer))
									{
										Registers.timer_rtc_flags &= ~(0x01 << current_timer);
									}
								}
								write_return = ChipsetReturnCode::MemoryGroupWriteSuccessful;
								break;
							}
							case 5:
							case 6:
							case 7:
							case 8:
							{
								uint8_t current_timer_start = current_register - 5;
								if (current_address % sizeof(QWord_LE))
								{
									write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
									break;
								}
								if (!(Registers.timer_rtc_flags & (0x10000 << current_timer_start)))
								{
									Registers.timer_start[current_timer_start] = data;
									Registers.timer_counter[current_timer_start] = data;
									if (Registers.timer_rtc_flags & (0x01 << current_timer_start))
									{
										Registers.timer_rtc_flags &= ~(0x01 << current_timer_start);
									}
								}
								else
								{
									Timer32BitModeData &timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[current_timer_start]);
									Timer32BitModeData &timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[current_timer_start]);
									Timer32BitModeData &timer_data = reinterpret_cast<Timer32BitModeData &>(data);
									for (uint8_t i = 0; i < 2; ++i)
									{
										timer_start[i] = timer_data[i];
										timer_counter[i] = timer_data[i];
										timer_sync[(i << 2) + current_timer_start].cycle_counter = 0;
										if (Registers.timer_rtc_flags & (0x01 << ((i << 2) + current_timer_start)))
										{
											Registers.timer_rtc_flags &= ~(0x01 << ((i << 2) + current_timer_start));
										}
									}
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
					case 1:
					{
						DWord_LE current_address = address + static_cast<int>(offset_data.offset);
						if (current_address + sizeof(T) - 1 >= interrupt_descriptor_table.size() * sizeof(ChipsetInterruptDescriptorTableEntryData))
						{
							write_return = ChipsetReturnCode::MemoryGroupWriteFailed;
							break;
						}
						uint8_t *interrupt_descriptor_table_ptr = reinterpret_cast<uint8_t *>(interrupt_descriptor_table.data());
						memcpy(&interrupt_descriptor_table_ptr[current_address], &data, sizeof(T));
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
			}

			template <uint8_t interrupt>
			inline void EndOfInterrupt()
			{
				if (interrupt >= 0x00 && interrupt <= 0x07)
				{
					constexpr uint8_t current_timer_pair = interrupt % 4;
					constexpr uint8_t current_timer_index = interrupt / 4;
					if (!(Registers.timer_rtc_flags & (0x10000 << current_timer_pair)))
					{
						Registers.timer_counter[current_timer_pair] = Registers.timer_start[current_timer_pair];
					}
					else
					{
						Timer32BitModeData &timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[current_timer_pair]);
						Timer32BitModeData &timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[current_timer_pair]);
						timer_counter[current_timer_index] = timer_start[current_timer_index];
					}
					if (Registers.timer_rtc_flags & (0x100 << interrupt))
					{
						Registers.timer_rtc_flags |= (0x01 << interrupt);
					}
				}
			}

			void SetupTimerSync(uint8_t timer, double cycles_per_second);
			// void RunTimers(CPU &cpu);
			
			template <uint8_t timer>
			inline void RunTimer(CPU &cpu)
			{
				if (Registers.timer_rtc_flags & (0x01 << timer))
				{
					++timer_sync[timer].cycle_counter;
					if (timer_sync[timer].cycle_counter == timer_sync[timer].cycles_per_tick)
					{
						timer_sync[timer].cycle_counter = 0;
						if (!(Registers.timer_rtc_flags & (0x10000 << (timer % sizeof(QWord_LE)))))
						{
							/*
							if (timer >= 4)
							{
								return;
							}
							*/
							QWord_LE tmp = Registers.timer_counter[timer];
							--Registers.timer_counter[timer];
							if (tmp < Registers.timer_counter[timer])
							{
								ChipsetInterruptDescriptorTableEntryData &idt_entry = interrupt_descriptor_table[timer];
								if (idt_entry.ISR_control.interrupt_flags & (0x01 << timer))
								{
									Registers.timer_rtc_flags &= ~(0x01 << timer);
									cpu.IssueInterruptRequest<0x04, timer>();
								}
								else
								{
									Registers.timer_counter[timer] = Registers.timer_start[timer];
									if (!(Registers.timer_rtc_flags & (0x100 << timer)))
									{
										Registers.timer_rtc_flags &= ~(0x01 << timer);
									}
								}
							}
						}
						/*
						else
						{
							Timer32BitModeData &current_timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[timer % sizeof(DWord_LE)]);
							DWord_LE tmp = current_timer_counter[timer / sizeof(DWord_LE)];
							--current_timer_counter[timer / sizeof(DWord_LE)];
							if (tmp < current_timer_counter[timer / sizeof(DWord_LE)])
							{
								ChipsetInterruptDescriptorTableEntryData &idt_entry = interrupt_descriptor_table[timer];
								if (idt_entry.ISR_control.interrupt_flags & 0x01)
								{
									Registers.timer_rtc_flags &= ~(0x01 << timer);
									cpu.IssueInterruptRequest(0x04, timer);
								}
								else
								{
									Timer32BitModeData &current_timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[timer % sizeof(DWord_LE)]);
									current_timer_counter[timer / sizeof(DWord_LE)] = current_timer_start[timer / sizeof(DWord_LE)];
									if (!(Registers.timer_rtc_flags & (0x100 << timer)))
									{
										Registers.timer_rtc_flags &= ~(0x01 << timer);
									}
								}
							}
						}
						*/
					}
				}
			}

			template <HasInterruptDescriptorTable T>
			friend void SetInterrupt(T &chipset, uint8_t interrupt, uint64_t isr_addr);
			template <HasInterruptDescriptorTable T>
			friend ChipsetInterruptDescriptorTableEntryData GetInterrupt(T &chipset, uint8_t interrupt);
			template <HasChipsetId T>
			friend std::string GetId(T &chipset);

		private:
			const std::string id = "ClassicVTimer-2";
			struct
			{
				QWord_LE timer_rtc_flags;
				std::array<TimerTickRateData, 4> TimerTickRates;
				std::array<QWord_LE, 4> timer_start;
				std::array<QWord_LE, 4> timer_counter;
			} Registers;
			std::array<ChipsetInterruptDescriptorTableEntryData, 8> interrupt_descriptor_table;
			std::array<TimerSyncData, 8> timer_sync;
	};
}

#endif
