#include "dma.hpp"

ClassicVCom_Nova64::DMA::DMA()
{
}

ClassicVCom_Nova64::DMA::~DMA()
{
}

void ClassicVCom_Nova64::DMA::operator()(Word_LE &function, GPR_Data &args)
{
	switch (static_cast<DMAChipsetFunction>(static_cast<uint16_t>(function)))
	{
		case DMAChipsetFunction::InitializeAllDMAChannels:
		{
			for (size_t i = 0; i < dma_channel.size(); ++i)
			{
				DMAChannelData &current_dma_channel = dma_channel[i];
				current_dma_channel = { 0x00, 0x00, 0x00, 0x00, 0x00 };
			}
			break;
		}
	}
}
