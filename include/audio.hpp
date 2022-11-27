#ifndef _AUDIO_HPP_
#define _AUDIO_HPP_

#include "chipset.hpp"
#include <array>

namespace ClassicVCom_Nova64
{
	enum class AudioChipsetFunction
	{
		InitializeVoiceModules = 0,
		InitializeVoiceModuleConnectionNodes = 1
	};

	struct WaveformTypeData
	{
		unsigned char waveform[4];
	};

	struct WaveformControlData
	{
		unsigned char flags;
		unsigned char configuration;
	};

	struct AttackDecayData
	{
		unsigned short attack;
		unsigned short decay;
	};

	struct ReleaseData
	{
		unsigned short release;
	};

	struct AudioControlSettingsData
	{
		unsigned char sample_rate_data[3];
		unsigned char audio_channel_mode;
	};

	class VoiceModule
	{
		public:
			VoiceModule();
			~VoiceModule();
			void Initialize();
		private:
			unsigned char flags;
			unsigned char voice_modulation_type;
			unsigned char bit_depth;
			unsigned char voice_channel_mode;
			unsigned int voice_channel_control;
			unsigned int voice_type_control_1;
			unsigned int voice_type_control_2;
			unsigned long long voice_type_control_3;
			unsigned long long voice_type_control_4;
			unsigned int voice_type_control_5;
			unsigned long long voice_type_control_6;
			unsigned int voice_type_control_7;
			double voice_output_control;
	};

	/*
	struct AudioChipsetData
	{
		AudioControlSettingsData audio_control_settings;
		double main_volume;
		unsigned long long voice_module_gate;
		VoiceModule VoiceModules[64];
		unsigned char voice_module_connection_node[64];
	};
	*/

	class Audio
	{
		public:
			Audio();
			~Audio();
			void operator()(unsigned short function, std::array<unsigned long long, 8> args);
			template <HasChipsetId T>
			std::string GetId(T &chipset);
		private:
			const std::string id = "ClassicVPulseSynth Nova64";
			AudioControlSettingsData audio_control_settings;
			double main_volume;
			unsigned long long voice_module_gate;
			std::array<VoiceModule, 64> VoiceModules;
			std::array<unsigned char, 64> voice_module_connection_node;
	};

	// using Audio = Chipset<AudioChipsetData, 0>;
}

#endif
