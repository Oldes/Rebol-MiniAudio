// =============================================================================
// Rebol/MiniAudio extension commands
// =============================================================================


#include "miniaudio-rebol-extension.h"
#include <stdio.h>
#include <stdlib.h> // malloc

#define COMMAND        int

#define FRM_IS_HANDLE(n, t)   (RXA_TYPE(frm,n) == RXT_HANDLE && RXA_HANDLE_TYPE(frm, n) == t)
#define ARG_Is_MASound(n)     FRM_IS_HANDLE(n, Handle_MASound)
#define ARG_Is_MANoise(n)     FRM_IS_HANDLE(n, Handle_MANoise)
#define ARG_Is_MAWaveform(n)  FRM_IS_HANDLE(n, Handle_MAWaveform)
#define ARG_MASound(n)        (ARG_Is_MASound(n)    ? (ma_sound*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_MANoise(n)        (ARG_Is_MANoise(n)    ? (ma_noise*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_MAWaveform(n)     (ARG_Is_MAWaveform(n) ? (ma_waveform*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_Is_DataSource(n)  (RXA_TYPE(frm,n) == RXT_HANDLE && (RXA_HANDLE_TYPE(frm, n) == Handle_MANoise || RXA_HANDLE_TYPE(frm, n) == Handle_MAWaveform) )
#define ARG_DataSource(n)     (ma_data_source*)(RXA_HANDLE_CONTEXT(frm, n)->handle)
#define ARG_Double(n)         RXA_DEC64(frm,n)
#define ARG_Int32(n)          RXA_INT32(frm,n)

#define RETURN_HANDLE(hob)                   \
	RXA_HANDLE(frm, 1)       = hob;          \
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;     \
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;   \
	RXA_TYPE(frm, 1) = RXT_HANDLE;           \
	return RXR_VALUE


ma_engine* pEngine = NULL;

static int assert_engine(void) {
	if (pEngine == NULL) {
		pEngine = malloc(sizeof(*pEngine));
		if (MA_SUCCESS != ma_engine_init(NULL, pEngine)) {
			return 0;
		}
	}
	return 1;
}

static void onSoundEnd(void* hob, ma_sound* pSound) {
	trace("sound end");
	if(hob) {
		// allow Rebol to release the sound, if not referenced
		((REBHOB*)hob)->flags &= ~HANDLE_CONTEXT_LOCKED;
	}
}

int MASound_free(void* hndl) {
	if (hndl != NULL) {
		trace("release sound");
		ma_sound *sound = (ma_sound*)hndl;
		ma_sound_uninit(sound);
	}
	return 0;
}
int MASound_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_sound* sound = (ma_sound*)hob->data;
	word = RL_FIND_WORD(sound_words, word);
	ma_uint64 frames;
	//printf("XTestContext_get_path word: %u\n", word);
	switch (word) {
	case W_SOUND_VOLUME:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_volume(sound);
		break;
	case W_SOUND_PAN:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_pan(sound);
		break;
	case W_SOUND_PITCH:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_pitch(sound);
		break;
	case W_SOUND_CURSOR:
		*type = RXT_INTEGER;
		ma_sound_get_cursor_in_pcm_frames(sound, &frames);
		arg->int64 = frames;
		break;
	case W_SOUND_FRAMES:
		*type = RXT_INTEGER;
		ma_sound_get_length_in_pcm_frames(sound, &frames);
		arg->int64 = frames;
		break;
	case W_SOUND_SAMPLE_RATE:
		*type = RXT_INTEGER;
		arg->int64 = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
		break;
	case W_SOUND_SPATIALIZATION:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_spatialization_enabled(sound);
		break;
	case W_SOUND_LOOP:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_looping(sound);
		break;
	case W_SOUND_ENDQ:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_at_end(sound);
		break;
	case W_SOUND_PLAYINGQ:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_playing(sound);
		break;
	case W_SOUND_FILE:
		if (hob->series) {
			*type = RXT_FILE;
			arg->series = hob->series;
			arg->index = 0;
		}
		else *type = RXT_NONE;
		break;

	default:
		return PE_BAD_SELECT;	
	}

	return PE_USE;
}
int MASound_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_sound* sound = (ma_sound*)hob->data;
	word = RL_FIND_WORD(sound_words, word);
	//printf("XTestContext_set_path word: %u\n", word);
	switch (word) {
	case W_SOUND_VOLUME:
		if (!(*type == RXT_DECIMAL || *type == RXT_PERCENT)) return PE_BAD_SET_TYPE;
		ma_sound_set_volume(sound, arg->dec64);
		break;
	case W_SOUND_PAN:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_sound_set_pan(sound, arg->dec64);
		break;
	case W_SOUND_PITCH:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_sound_set_pitch(sound, arg->dec64);
		break;
	case W_SOUND_SPATIALIZATION:
		if (*type != RXT_LOGIC) return PE_BAD_SET_TYPE;
		ma_sound_set_spatialization_enabled(sound, arg->int32a);
		break;
	case W_SOUND_LOOP:
		if (*type != RXT_LOGIC) return PE_BAD_SET_TYPE;
		ma_sound_set_looping(sound, arg->int32a);
		break;
	//case W_ID:
	//	if (*type != RXT_INTEGER) return PE_BAD_SET_TYPE;
	//	xtest->id = arg->int64;
	//	break;
	//case W_DATA:
	//	if (*type != RXT_BINARY) return PE_BAD_SET_TYPE;
	//	hob->series = arg->series;
	//	xtest->num = SERIES_TAIL(hob->series);
	//	break;
	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}

int MANoise_free(void* hndl) {
	if (hndl != NULL) {
		trace("release noise");
	//	ma_sound *sound = (ma_sound*)hndl;
	//	ma_sound_uninit(sound);
	}
	return 0;
}
int MANoise_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_noise* noise = (ma_noise*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	printf("MANoise_get_path word: %u\n", word);
	switch (word) {
	case W_ARG_AMPLITUDE:
		*type = RXT_DECIMAL;
		arg->dec64 = noise->config.amplitude;
		break;
	case W_ARG_TYPE:
		*type = RXT_INTEGER;
		arg->int64 = noise->config.type;
		printf("type: %i %f %p\n", noise->config.type, noise->config.amplitude, &noise->config);
		break;
	default:
		return PE_BAD_SELECT;	
	}
	return PE_USE;
}
int MANoise_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_noise* noise = (ma_noise*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	//printf("MANoise_set_path word: %u\n", word);
	switch (word) {
	case W_ARG_AMPLITUDE:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_noise_set_amplitude(noise, arg->dec64);
		break;
	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}


int MAWaveform_free(void* hndl) {
	if (hndl != NULL) {
		trace("release waveform");
	//	ma_sound *sound = (ma_sound*)hndl;
	//	ma_sound_uninit(sound);
	}
	return 0;
}
int MAWaveform_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_waveform* waveform = (ma_waveform*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	//printf("MAWaveform_get_path word: %u\n", word);
	switch (word) {
	case W_ARG_AMPLITUDE:
		*type = RXT_DECIMAL;
		arg->dec64 = waveform->config.amplitude;
		break;
	case W_ARG_FREQUENCY:
		*type = RXT_DECIMAL;
		arg->dec64 = waveform->config.frequency;
		break;
	case W_ARG_TYPE:
		*type = RXT_INTEGER;
		arg->int64 = waveform->config.type;
		//printf("type: %i %f %p\n", noise->config.type, noise->config.amplitude, &noise->config);
		break;
	default:
		return PE_BAD_SELECT;	
	}
	return PE_USE;
}
int MAWaveform_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_waveform* waveform = (ma_waveform*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	//printf("MANoise_set_path word: %u\n", word);
	switch (word) {
	case W_ARG_AMPLITUDE:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_waveform_set_amplitude(waveform, arg->dec64);
		break;
	case W_ARG_FREQUENCY:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_waveform_set_frequency(waveform, arg->dec64);
		break;
	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}

COMMAND cmd_init_words(RXIFRM *frm, void *ctx) {
	sound_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
	arg_words = RL_MAP_WORDS(RXA_SERIES(frm,2));
	return RXR_TRUE;
}

COMMAND cmd_test(RXIFRM *frm, void *ctx) {
	if (!assert_engine()) return RXR_ERROR;

	// just a test for an issue with the noise type
	ma_noise *noise = malloc(sizeof(ma_noise));
	ma_sound *sound = malloc(sizeof(ma_sound));

	ma_noise_config config = ma_noise_config_init(ma_format_u8, 1, 2, 0, 1.0);
	if (MA_SUCCESS != ma_noise_init(&config, NULL, noise)) return RXR_FALSE;
	printf("type: %i\n", noise->config.type); // here it is OK (2)

	if (MA_SUCCESS != ma_sound_init_from_data_source(pEngine, noise, 0, NULL, sound)) return RXR_FALSE;
	printf("type: %i\n", noise->config.type); // here it is zero!
	ma_sound_start(sound);
	printf("type: %i\n", noise->config.type); // here it is zero! 

return RXR_TRUE;

#ifdef unused
	ma_result result;
	ma_context context;
	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_uint32 iDevice, iData;


	if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
		printf("Failed to initialize context.\n");
		return -2;
	}

	result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
	if (result != MA_SUCCESS) {
		printf("Failed to retrieve device information.\n");
		return -3;
	}

	printf("Playback Devices\n");
	for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
		printf("    %u: %s %u\n", iDevice, pPlaybackDeviceInfos[iDevice].name, pPlaybackDeviceInfos[iDevice].nativeDataFormatCount);
		for (iData = 0; iData < pPlaybackDeviceInfos[iDevice].nativeDataFormatCount; ++iData) {
			printf("        %u\n",          pPlaybackDeviceInfos[iDevice].nativeDataFormats[iData].sampleRate);
		}
	}	

	printf("\n");

	printf("Capture Devices\n");
	for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
		printf("    %u: %s\n", iDevice, pCaptureDeviceInfos[iDevice].name);
	}


	ma_context_uninit(&context);

	return RXR_TRUE;
#endif
}

COMMAND cmd_play(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	ma_uint32 flags = 0;
	REBHOB* hob = NULL;

	if (!assert_engine()) return RXR_ERROR;

	if (RXA_TYPE(frm, 1) == RXT_FILE) {
		const char* file = (const char*)((REBSER*)RXA_ARG(frm, 1).series)->data;

		hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
		if (hob == NULL) return RXR_NONE;
		sound = (ma_sound*)hob->data;

		hob->series = RXA_ARG(frm, 1).series;

		if (RXA_REF(frm, 2)) flags |= MA_SOUND_FLAG_STREAM;
		if (MA_SUCCESS != ma_sound_init_from_file(pEngine, file, flags, NULL, NULL, sound)) return RXR_FALSE;

		ma_sound_set_end_callback(sound, onSoundEnd, hob);
	} else if (ARG_Is_MASound(1)) {
		sound = ARG_MASound(1);
	} else if (ARG_Is_DataSource(1)) {
		ma_data_source *source = ARG_DataSource(1);

		hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
		if (hob == NULL) return RXR_NONE;
		sound = (ma_sound*)hob->data;
		if (MA_SUCCESS != ma_sound_init_from_data_source(pEngine, source, 0, NULL, sound)) return RXR_FALSE;
	} else {
		return RXR_NONE;
	}

	if (hob) {
		hob->flags |= HANDLE_CONTEXT_LOCKED;
		RXA_HANDLE(frm, 1)       = hob;
		RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
		RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
		RXA_TYPE(frm, 1) = RXT_HANDLE;
	}
	ma_sound_start(sound);
	ma_sound_set_looping(sound, RXA_REF(frm, 3));
	
	return RXR_VALUE;
}

COMMAND cmd_pause(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	ma_sound_stop(sound);
	return RXR_VALUE;
}

COMMAND cmd_start(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	REBI64 frame = 0;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	if (RXA_REF(frm, 3)) {
		if (RXA_TYPE(frm, 4) == RXT_INTEGER) {
			frame = RXA_INT64(frm, 4);
		} else {
			ma_uint64 sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
			frame = (RXA_TIME(frm, 4) * sampleRate) / 1000000000; // rate is per second
		}
		if (frame < 0) frame = 0;
	}
	
	ma_sound_set_looping(sound, RXA_REF(frm, 2));
	ma_sound_seek_to_pcm_frame(sound, frame);
	ma_sound_start(sound);

	RXA_HANDLE_FLAGS(frm, 1) |= HANDLE_CONTEXT_LOCKED;
	ma_sound_set_end_callback(sound, onSoundEnd, RXA_HANDLE(frm, 1));
	return RXR_VALUE;
}

COMMAND cmd_stop(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	REBINT type;
	REBI64 fade;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;
	
	switch(RXA_TYPE(frm, 3)) {
	case RXT_NONE: ma_sound_stop(sound); break;
	case RXT_INTEGER: ma_sound_stop_with_fade_in_pcm_frames(sound, (ma_uint64)RXA_INT64(frm, 3)); break;
	case RXT_TIME:
		fade = RXA_TIME(frm, 3) / 1000000; // time in ms
		if (fade <= 0) ma_sound_stop(sound);
		ma_sound_stop_with_fade_in_milliseconds(sound, (ma_uint64)fade);
		break;
	}
	return RXR_VALUE;
}

COMMAND cmd_seek(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	ma_uint64 cursor = 0;
	REBI64 frame;

	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	if (RXA_TYPE(frm, 2) == RXT_INTEGER) {
		frame = RXA_INT64(frm, 2);
	} else {
		ma_uint64 sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
		frame = (RXA_TIME(frm, 2) * sampleRate) / 1000000000;
	}
	
	if (RXA_REF(frm, 3)) {
		// relative
		ma_sound_get_cursor_in_pcm_frames(sound, &cursor);
		frame += cursor;
	}
	if (frame < 0) frame = 0;

	ma_sound_seek_to_pcm_frame(sound, frame);
	return RXR_VALUE;
}

COMMAND cmd_load(RXIFRM *frm, void *ctx) {
	ma_result result;
	ma_sound *sound;
	REBHOB* hob;

	if (!assert_engine()) return RXR_NONE;

	hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
	if (hob == NULL) return RXR_NONE;
	sound = (ma_sound*)hob->data;

	const char* file = (const char*)((REBSER*)RXA_ARG(frm, 1).series)->data;

	hob->series = RXA_ARG(frm, 1).series;

	if (MA_SUCCESS != ma_sound_init_from_file(pEngine, file, 0, NULL, NULL, sound)) return RXR_FALSE;

	RETURN_HANDLE(hob);
}

COMMAND cmd_volume(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	float volume;
	
	sound = ARG_MASound(1);
	volume = ARG_Double(2);

	if (sound == NULL) return RXR_FALSE;
	ma_sound_set_volume(sound, volume);
	return RXR_VALUE;
}

COMMAND cmd_volumeq(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	RXA_DEC64(frm, 1) = ma_sound_get_volume(sound);
	RXA_TYPE(frm, 1) = RXT_DECIMAL;
	return RXR_VALUE;
}

COMMAND cmd_pan(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	float pan;
	
	sound = ARG_MASound(1);
	pan   = ARG_Double(2);

	if (sound == NULL) return RXR_FALSE;
	ma_sound_set_pan(sound, pan);
	return RXR_VALUE;
}

COMMAND cmd_panq(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	RXA_DEC64(frm, 1) = ma_sound_get_pan(sound);
	RXA_TYPE(frm, 1) = RXT_DECIMAL;
	return RXR_VALUE;
}

COMMAND cmd_pitch(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	float pitch;
	
	sound = ARG_MASound(1);
	pitch   = ARG_Double(2);

	if (sound == NULL) return RXR_FALSE;
	ma_sound_set_pitch(sound, pitch);
	return RXR_VALUE;
}

COMMAND cmd_pitchq(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	
	sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	RXA_DEC64(frm, 1) = ma_sound_get_pitch(sound);
	RXA_TYPE(frm, 1) = RXT_DECIMAL;
	return RXR_VALUE;
}

COMMAND cmd_looping(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	
	sound = ARG_MASound(1);

	if (sound == NULL) return RXR_FALSE;
	ma_sound_set_looping(sound, RXA_LOGIC(frm,2));
	return RXR_VALUE;
}

COMMAND cmd_loopingq(RXIFRM *frm, void *ctx) {
	ma_sound *sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;
	return ma_sound_is_looping(sound) ? RXR_TRUE : RXR_FALSE;
}

COMMAND cmd_endq(RXIFRM *frm, void *ctx) {
	ma_sound *sound = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;
	return ma_sound_at_end(sound) ? RXR_TRUE : RXR_FALSE;
}

COMMAND cmd_shutdown(RXIFRM *frm, void *ctx) {
	if (pEngine == NULL) return RXR_FALSE;
	ma_engine_uninit(pEngine);
	free(pEngine);
	pEngine = NULL;
	return RXR_TRUE;
}


COMMAND cmd_noise_node(RXIFRM *frm, void *ctx) {
	ma_noise *noise;
	REBHOB* hob = RL_MAKE_HANDLE_CONTEXT(Handle_MANoise);
	if (hob == NULL) return RXR_NONE;
	noise = (ma_noise*)hob->data;

	ma_noise_config config = ma_noise_config_init(
		ma_format_u8,
		1,
		ARG_Int32(1),  //= type
		ARG_Int32(4),  //= seed
		ARG_Double(2)  //= apmplitude
	);

	printf("type: %i seed: %i\n", ARG_Int32(1), ARG_Int32(4));

	if (MA_SUCCESS != ma_noise_init(&config, NULL, noise)) return RXR_FALSE;
	printf("type: %i %p %p\n", noise->config.type, (void*)&noise->config, &config);

	hob->series = NULL;
	hob->flags |= HANDLE_CONTEXT_LOCKED;
	RXA_HANDLE(frm, 1)       = hob;
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;

	return RXR_VALUE;
}

COMMAND cmd_waveform_node(RXIFRM *frm, void *ctx) {
	ma_waveform *waveform;
	REBHOB* hob = RL_MAKE_HANDLE_CONTEXT(Handle_MAWaveform);
	if (hob == NULL) return RXR_NONE;
	waveform = (ma_waveform*)hob->data;

	if (!assert_engine()) return RXR_ERROR;

	ma_waveform_config config = ma_waveform_config_init(
		ma_format_u8,  //= format
		1,             //= channels
		44100, //ma_engine_get_sample_rate(pEngine),
		ARG_Int32(1),  //= type
		ARG_Double(2), //= apmplitude
		ARG_Double(3)  //= frequency
	);

	if (MA_SUCCESS != ma_waveform_init(&config, waveform)) return RXR_FALSE;

	hob->series = NULL;
	hob->flags |= HANDLE_CONTEXT_LOCKED;
	RXA_HANDLE(frm, 1)       = hob;
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;

	return RXR_VALUE;
}



