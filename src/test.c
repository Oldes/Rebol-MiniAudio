// just a test for an issue with the noise type
//#define MA_DEBUG_OUTPUT
//#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <stdlib.h> // malloc

#if !defined(_WIN32)
# include <unistd.h>
# define Sleep(t) sleep(t)
#endif

ma_engine* pEngine = NULL;

int main(int argc, char** argv)
{

	pEngine = malloc(sizeof(*pEngine));
	if (MA_SUCCESS != ma_engine_init(NULL, pEngine)) return -1;

	ma_noise *pNoise = malloc(sizeof(ma_noise));
	ma_sound *pSound = malloc(sizeof(ma_sound));

	ma_noise_config config = ma_noise_config_init(ma_format_u8, 1, 2, 0, 1.0);
	if (MA_SUCCESS != ma_noise_init(&config, NULL, pNoise)) return -2;
	printf("type: %i\n", pNoise->config.type); // here it is OK (2)

	if (MA_SUCCESS != ma_sound_init_from_data_source(pEngine, pNoise, 0, NULL, pSound)) return -3;
	printf("type: %i\n", pNoise->config.type); // here it is zero!
	ma_sound_start(pSound);
	printf("type: %i\n", pNoise->config.type); // here it is zero!

	//ma_sound_set_volume(pSound, 0.01);
	ma_sound_set_fade_in_pcm_frames(pSound, 0, 1.0, 44100);
	
	Sleep(2);

	ma_sound_uninit(pSound);
	ma_noise_uninit(pNoise, NULL);
	ma_engine_uninit(pEngine);
	free(pEngine);
	free(pNoise);
	free(pSound);

	return 0;
}