#include "timer.hpp"
#include <cstring>

ClassicVCom_Nova64::Timer::Timer()
{
	for (uint8_t i = 0; i < interrupt_descriptor_table.size(); ++i)
	{
		interrupt_descriptor_table[i] = ChipsetInterruptDescriptorTableEntryData{ { 0, 0, 0 }, 0 };
	}
	Registers.timer_rtc_flags = 0x0000000000000000;
	for (uint8_t i = 0; i < 4; ++i)
	{
		Registers.TimerTickRates[i].timer_tick_rate = { 1000000, 1000000 };
	}
	Registers.timer_start = { 0, 0, 0, 0 };
	Registers.timer_counter = { 0, 0, 0, 0 };
	for (uint8_t i = 0; i < 8; ++i)
	{
		timer_sync[i] = { 0, 0 };
	}
}

ClassicVCom_Nova64::Timer::~Timer()
{
}

void ClassicVCom_Nova64::Timer::operator()(Word_LE &function, std::array<QWord_LE, 8> &args)
{
	switch (static_cast<TimerChipsetFunction>(static_cast<uint16_t>(function)))
	{
		case TimerChipsetFunction::ResetTimerCounters:
		{
			struct alignas(8) OperandControl
			{
				uint8_t timers;
			} operand_control = std::bit_cast<OperandControl>(args[0]);
			for (uint8_t t = 0; t < 8; ++t)
			{
				if (operand_control.timers & (0x01 << t))
				{
					if (!(Registers.timer_rtc_flags & (0x10000 << (t % 4))) && t < 4)
					{
						Registers.timer_counter[t] = Registers.timer_start[t];
					}
					else
					{
						uint8_t current_timer_register = t % 4;
						uint8_t current_timer = t % 2;
						Timer32BitModeData &start_data = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[current_timer_register]);
						Timer32BitModeData &counter_data = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[current_timer_register]);
						counter_data[current_timer] = start_data[current_timer];
					}
				}
			}
			break;
		}
	}
}

void ClassicVCom_Nova64::Timer::SetupTimerSync(uint8_t timer, double cycles_per_second)
{
	if (timer < 8)
	{
		uint8_t timer_tick_rate_index = timer % 4;
		uint8_t timer_index = timer / 4;
		timer_sync[timer].cycles_per_tick = cycles_per_second / static_cast<double>(Registers.TimerTickRates[timer_tick_rate_index].timer_tick_rate[timer_index]);
		timer_sync[timer].cycle_counter = 0;
	}
}

/*
void ClassicVCom_Nova64::Timer::RunTimers(CPU &cpu)
{
	for (uint8_t t = 0; t < 2; ++t)
	{
		if (Registers.timer_rtc_flags & (0x01 << t))
		{
			++timer_sync[t].cycle_counter;
			if (timer_sync[t].cycle_counter == timer_sync[t].cycles_per_tick)
			{
				timer_sync[t].cycle_counter = 0;
				if (!(Registers.timer_rtc_flags & (0x10000 << (t % sizeof(DWord_LE)))))
				{
					if (t >= 4)
					{
						continue;
					}
					QWord_LE tmp = Registers.timer_counter[t];
					--Registers.timer_counter[t];
					if (tmp < Registers.timer_counter[t])
					{
						ChipsetInterruptDescriptorTableEntryData &idt_entry = interrupt_descriptor_table[t];
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							Registers.timer_rtc_flags &= ~(0x01 << t);
							cpu.IssueInterruptRequest(0x04, t);
						}
						else
						{
							Registers.timer_counter[t] = Registers.timer_start[t];
							if (!(Registers.timer_rtc_flags & (0x100 << t)))
							{
								Registers.timer_rtc_flags &= ~(0x01 << t);
							}
						}
					}
				}
				else
				{
					Timer32BitModeData &current_timer_counter = reinterpret_cast<Timer32BitModeData &>(Registers.timer_counter[t % sizeof(DWord_LE)]);
					uint8_t current_timer_counter_index = t / sizeof(DWord_LE);
					DWord_LE tmp = current_timer_counter[current_timer_counter_index];
					--current_timer_counter[current_timer_counter_index];
					if (tmp < current_timer_counter[current_timer_counter_index])
					{
						ChipsetInterruptDescriptorTableEntryData &idt_entry = interrupt_descriptor_table[t];
						if (idt_entry.ISR_control.interrupt_flags & 0x01)
						{
							Registers.timer_rtc_flags &= ~(0x01 << t);
							cpu.IssueInterruptRequest(0x04, t);
						}
						else
						{
							Timer32BitModeData &current_timer_start = reinterpret_cast<Timer32BitModeData &>(Registers.timer_start[t % sizeof(DWord_LE)]);
							current_timer_counter[current_timer_counter_index] = current_timer_start[current_timer_counter_index];
							if (!(Registers.timer_rtc_flags & (0x100 << t)))
							{
								Registers.timer_rtc_flags &= ~(0x01 << t);
							}
						}
					}
				}
			}
		}
	}
}
*/
