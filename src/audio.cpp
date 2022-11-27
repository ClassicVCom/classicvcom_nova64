#include "audio.hpp"
#include "utility.hpp"
#include <cstring>

ClassicVCom_Nova64::VoiceModule::VoiceModule() : flags(0x00), voice_modulation_type(0), bit_depth(0), voice_channel_mode(0), voice_channel_control(0)
{
}

ClassicVCom_Nova64::VoiceModule::~VoiceModule()
{
}
			
void ClassicVCom_Nova64::VoiceModule::Initialize()
{
	flags = 0x00;
	voice_modulation_type = 0;
	bit_depth = 0;
	voice_channel_mode = 0;
	voice_channel_control = 0;
}

ClassicVCom_Nova64::Audio::Audio() : main_volume(0.5), voice_module_gate(0x0000000000000000)
{
	memset(&audio_control_settings, 0x00, sizeof(audio_control_settings));
	voice_module_connection_node = GenerateIdentityMap<unsigned char, 64>();
}

ClassicVCom_Nova64::Audio::~Audio()
{
}

void ClassicVCom_Nova64::Audio::operator()(unsigned short function, std::array<unsigned long long, 8> args)
{
	switch (static_cast<AudioChipsetFunction>(function))
	{
		case AudioChipsetFunction::InitializeVoiceModules:
		{
			for (unsigned char i = 0; i < 64; ++i)
			{
				VoiceModules[i].Initialize();
			}
			break;
		}
		case AudioChipsetFunction::InitializeVoiceModuleConnectionNodes:
		{
			voice_module_connection_node = GenerateIdentityMap<unsigned char, 64>();
			break;
		}
	}
}


/*
template <>
ClassicVCom_Nova64::Audio::Chipset() : id("ClassicVPulseSynth Nova64")
{
	memset(&ChipsetData.audio_control_settings, 0x00, sizeof(ChipsetData.audio_control_settings));
	ChipsetData.main_volume = 0.5;
	ChipsetData.voice_module_gate = 0x0000000000000000;
	for (unsigned int i = 0; i < 64; ++i)
	{
		ChipsetData.voice_module_connection_node[i] = i;
	}
}

template <>
void ClassicVCom_Nova64::Audio::operator()(unsigned short function, unsigned long long args[8])
{
	switch (static_cast<AudioChipsetFunction>(function))
	{
		case AudioChipsetFunction::InitializeVoiceModules:
		{
			for (unsigned char i = 0; i < 64; ++i)
			{
				ChipsetData.VoiceModules[i].Initialize();
			}
			break;
		}
		case AudioChipsetFunction::InitializeVoiceModuleConnectionNodes:
		{
			for (unsigned char i = 0; i < 64; ++i)
			{
				ChipsetData.voice_module_connection_node[i] = i;
			}
			break;
		}
	}
}
*/
