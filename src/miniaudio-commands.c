// =============================================================================
// Rebol/MiniAudio extension commands
// =============================================================================


#include "miniaudio-rebol-extension.h"
#include <stdio.h>
#include <stdlib.h> // malloc

#define COMMAND int

#define FRM_IS_HANDLE(n, t)   (RXA_TYPE(frm,n) == RXT_HANDLE && RXA_HANDLE_TYPE(frm, n) == t)
#define ARG_Is_MAEngine(n)    FRM_IS_HANDLE(n, Handle_MAEngine)
#define ARG_Is_MASound(n)     FRM_IS_HANDLE(n, Handle_MASound)
#define ARG_Is_MANoise(n)     FRM_IS_HANDLE(n, Handle_MANoise)
#define ARG_Is_MAWaveform(n)  FRM_IS_HANDLE(n, Handle_MAWaveform)
#define ARG_MASound(n)        (ARG_Is_MASound(n)    ? (ma_sound*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_MANoise(n)        (ARG_Is_MANoise(n)    ? (ma_noise*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_MAWaveform(n)     (ARG_Is_MAWaveform(n) ? (ma_waveform*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_Is_DataSource(n)  (RXA_TYPE(frm,n) == RXT_HANDLE && (RXA_HANDLE_TYPE(frm, n) == Handle_MANoise || RXA_HANDLE_TYPE(frm, n) == Handle_MAWaveform) )
#define ARG_DataSource(n)     (ma_data_source*)(RXA_HANDLE_CONTEXT(frm, n)->handle)
#define ARG_Double(n)         RXA_DEC64(frm,n)
#define ARG_Float(n)          (float)RXA_DEC64(frm,n)
#define ARG_Int32(n)          RXA_INT32(frm,n)

#define RETURN_HANDLE(hob)                   \
	RXA_HANDLE(frm, 1)       = hob;          \
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;     \
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;   \
	RXA_TYPE(frm, 1) = RXT_HANDLE;           \
	return RXR_VALUE

#define APPEND_STRING(str, ...) \
	len = snprintf(NULL,0,__VA_ARGS__);\
	if (len > SERIES_REST(str)-SERIES_LEN(str)) {\
		RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
		SERIES_TAIL(str) -= len;\
	}\
	len = snprintf( \
		SERIES_TEXT(str)+SERIES_TAIL(str),\
		SERIES_REST(str)-SERIES_TAIL(str),\
		__VA_ARGS__\
	);\
	SERIES_TAIL(str) += len;

#define RETURN_ERROR(err)  do {RXA_SERIES(frm, 1) = err; return RXR_ERROR;} while(0)
#define ASSERT_ENGINE()    if(pEngine == NULL) RETURN_ERROR("No playback device initialized!");


my_engine* pEngine = NULL;
ma_context gContext;
ma_resource_manager gResourceManager;

REBHOB* pEngineHob = NULL;


static ma_uint64 abs_sound_frames(RXIARG *arg, ma_sound *sound) {
	ma_engine *engine = ma_sound_get_engine(sound);
	return arg->uint64 + ma_engine_get_time_in_pcm_frames(engine);
}
static ma_uint64 abs_sound_time_to_frames(RXIARG *arg, ma_sound *sound) {
	ma_engine *engine = ma_sound_get_engine(sound);
	ma_uint64 frames = (arg->uint64 * ma_engine_get_sample_rate(engine)) / 1000000000;
	return frames + ma_engine_get_time_in_pcm_frames(engine);
}

static void onSoundEnd(void* hob, ma_sound* pSound) {
	trace("sound end ");
}

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	(void)pInput;
	/*
	Since we're managing the underlying device ourselves, we need to read from the engine directly.
	To do this we need access to the `ma_engine` object which we passed in to the user data. One
	advantage of this is that you could do your own audio processing in addition to the engine's
	standard processing.
	*/
	ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, NULL);

}


int Common_mold(REBHOB *hob, REBSER *str) {
	int len;
	if (!str) return 0;
	SERIES_TAIL(str) = 0;
	APPEND_STRING(str, "0#%lx", (unsigned long)hob->data);
	return len;
}


int MAEngine_free(void* hndl) {
	RXIARG  arg;
	REBSER *blk;
	REBHOB *hob;
	my_engine *engine;

	if (!hndl) return 0;
	hob = (REBHOB *)hndl;

	printf("release engine: %p is referenced: %i\n", hob->data, IS_MARK_HOB(hob) != 0);
	UNMARK_HOB(hob);
	blk = hob->series;
	if (blk) {
		int i = blk->tail;
		while (i-->0) {
			if (RXT_HANDLE == RL_GET_VALUE(blk, i, &arg)) {
				RL_FREE_HANDLE_CONTEXT(arg.handle.ptr);
			}
		}
		RESET_SERIES(blk);
		hob->series = NULL;
	}
	engine = (my_engine*)hob->data;
	ma_engine_uninit(&engine->engine);
	ma_device_uninit(&engine->device);
	return 0;
}
int MAEngine_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_engine* engine = (ma_engine*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	switch (word) {
	case W_ARG_RESOURCES:
		*type = RXT_BLOCK;
		arg->series = hob->series;
		arg->index = 0;
		break;
	case W_ARG_VOLUME:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_engine_get_volume(engine);
		break;
	case W_ARG_FRAMES:
		*type = RXT_INTEGER;
		arg->uint64 = ma_engine_get_time_in_pcm_frames(engine);
		break;
	case W_ARG_TIME:
		*type = RXT_TIME;
		arg->int64 = ma_engine_get_time_in_milliseconds(engine) * 1000000;
		break;
	case W_ARG_CHANNELS:
		*type = RXT_INTEGER;
		arg->uint64 = ma_engine_get_channels(engine);
		break;
	case W_ARG_SAMPLE_RATE:
		*type = RXT_INTEGER;
		arg->uint64 = ma_engine_get_sample_rate(engine);
		break;
	case W_ARG_GAIN_DB:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_engine_get_gain_db(engine);
		break;
	default:
		return PE_BAD_SELECT;	
	}
	return PE_USE;
}
int MAEngine_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_engine* engine = (ma_engine*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	switch (word) {
	case W_ARG_VOLUME:
		switch (*type) {
		case RXT_DECIMAL:
		case RXT_PERCENT:
			ma_engine_set_volume(engine, arg->dec64);
			break;
		case RXT_INTEGER:
			ma_engine_set_volume(engine, (float)arg->int64);
			break;
		default: 
			return PE_BAD_SET_TYPE;
		}
		break;
	case W_ARG_FRAMES:
		if (*type != RXT_INTEGER) return PE_BAD_SET_TYPE;
		ma_engine_set_time_in_pcm_frames(engine, arg->uint64);
		break;
	case W_ARG_TIME:
		if (*type != RXT_TIME) return PE_BAD_SET_TYPE;
		if (arg->uint64 < 0) return PE_BAD_SET;
		ma_engine_set_time_in_milliseconds(engine, arg->uint64 / 1000000);
		break;
	case W_ARG_GAIN_DB:
		if      (*type == RXT_DECIMAL) ma_engine_set_gain_db(engine, (float)arg->dec64);
		else if (*type == RXT_INTEGER) ma_engine_set_gain_db(engine, (float)arg->int64);
		else return PE_BAD_SET_TYPE;
		break;
	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}


int MASound_free(void* hndl) {
	REBHOB *hob;
	if (!hndl) return 0;
	hob = (REBHOB *)hndl;
	printf("release sound: %p is referenced: %i\n", hob->data, IS_MARK_HOB(hob) != 0);

	ma_sound *sound = (ma_sound*)hob->data;

	// Don't release it, if not referenced but still playing...
	if(!IS_MARK_HOB(hob) && ma_sound_is_playing(sound)) {
		puts("preventing sound release?");
		//MARK_HOB(hob);
		//return 0;
	}

	UNMARK_HOB(hob);
	ma_sound_uninit(sound);
	if (hob->series) {
		RESET_SERIES(hob->series);
		hob->series = NULL;
	}
	return 0;
}
int MASound_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_sound* sound = (ma_sound*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	ma_uint64 frames;
	ma_uint32 sampleRate;
	ma_vec3f pos;

	switch (word) {
	case W_ARG_VOLUME:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_volume(sound);
		break;
	case W_ARG_POSITION:
		*type = RXT_PAIR;
		pos = ma_sound_get_position(sound);
		arg->dec32a = pos.x;
		arg->dec32b = pos.y;
		break;
	case W_ARG_PAN:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_pan(sound);
		break;
	case W_ARG_PITCH:
		*type = RXT_DECIMAL;
		arg->dec64 = ma_sound_get_pitch(sound);
		break;
	case W_ARG_CURSOR:
		*type = RXT_INTEGER;
		ma_sound_get_cursor_in_pcm_frames(sound, &frames);
		arg->int64 = frames;
		break;
	case W_ARG_DURATION:
		*type = RXT_TIME;
		ma_sound_get_length_in_pcm_frames(sound, &frames);
		sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
		arg->uint64 = (frames * 1000000000) / sampleRate;
		break;
	case W_ARG_FRAMES:
		*type = RXT_INTEGER;
		ma_sound_get_length_in_pcm_frames(sound, &frames);
		arg->int64 = frames;
		break;
	case W_ARG_SAMPLE_RATE:
		*type = RXT_INTEGER;
		arg->int64 = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
		break;
	case W_ARG_SPATIALIZE:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_spatialization_enabled(sound);
		break;
	case W_ARG_IS_LOOPING:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_looping(sound);
		break;
	case W_ARG_AT_END:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_at_end(sound);
		break;
	case W_ARG_IS_PLAYING:
		*type = RXT_LOGIC;
		arg->int32a = ma_sound_is_playing(sound);
		break;
	case W_ARG_SOURCE:
		if (hob->series) {
			*type = RL_GET_VALUE(hob->series, 0, arg);
		}
		else *type = RXT_NONE;
		break;

	case W_ARG_START:
		*type = RXT_INTEGER;
		arg->uint64 = ma_node_get_state_time(sound, ma_node_state_started);
		break;	
	case W_ARG_STOP:
		*type = RXT_INTEGER;
		arg->uint64 = ma_node_get_state_time(sound, ma_node_state_stopped);
		break;

	case W_ARG_X:
		*type = RXT_DECIMAL;
		pos = ma_sound_get_position(sound);
		arg->dec64 = pos.x;
		break;
	case W_ARG_Y:
		*type = RXT_DECIMAL;
		pos = ma_sound_get_position(sound);
		arg->dec64 = pos.y;
		break;
	case W_ARG_Z:
		*type = RXT_DECIMAL;
		pos = ma_sound_get_position(sound);
		arg->dec64 = pos.z;
		break;

	default:
		return PE_BAD_SELECT;	
	}

	return PE_USE;
}
int MASound_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_sound* sound = (ma_sound*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	ma_uint64 frames;
	ma_vec3f pos;

	switch (word) {
	case W_ARG_VOLUME:
		switch (*type) {
		case RXT_DECIMAL:
		case RXT_PERCENT:
			ma_sound_set_volume(sound, arg->dec64);
			break;
		case RXT_INTEGER:
			ma_sound_set_volume(sound, (float)arg->int64);
			break;
		default: 
			return PE_BAD_SET_TYPE;
		}
		break;
	case W_ARG_POSITION:
		if (*type != RXT_PAIR) return PE_BAD_SET_TYPE;
		pos = ma_sound_get_position(sound);
		ma_sound_set_position(sound, arg->dec32a, arg->dec32b, pos.z);
		break;
	case W_ARG_PAN:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_sound_set_pan(sound, arg->dec64);
		break;
	case W_ARG_PITCH:
		if (*type != RXT_DECIMAL) return PE_BAD_SET_TYPE;
		ma_sound_set_pitch(sound, arg->dec64);
		break;
	case W_ARG_SPATIALIZE:
		if (*type != RXT_LOGIC) return PE_BAD_SET_TYPE;
		ma_sound_set_spatialization_enabled(sound, arg->int32a);
		break;
	case W_ARG_IS_LOOPING:
		if (*type != RXT_LOGIC) return PE_BAD_SET_TYPE;
		ma_sound_set_looping(sound, arg->int32a);
		break;

	case W_ARG_START:
	case W_ARG_STOP:
		if (arg->int64 < 0) return PE_BAD_SET; // allow only positive time
		if   (*type == RXT_INTEGER) frames = abs_sound_frames(arg, sound);
		else if (*type == RXT_TIME) frames = abs_sound_time_to_frames(arg, sound);
		else return PE_BAD_SET_TYPE;
		if (word == W_ARG_START)
			ma_sound_set_start_time_in_pcm_frames(sound, frames);
		else
			ma_sound_set_stop_time_in_pcm_frames(sound, frames);
		break;

	case W_ARG_X:
	case W_ARG_Y:
	case W_ARG_Z:
		*type = RXT_DECIMAL;
		pos = ma_sound_get_position(sound);
		switch(word) {
		case W_ARG_X: pos.x = (float)arg->dec64; break; 
		case W_ARG_Y: pos.y = (float)arg->dec64; break; 
		case W_ARG_Z: pos.z = (float)arg->dec64; break; 
		}
		ma_sound_set_position(sound, pos.x, pos.y, pos.z);

		break;

	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}

int MANoise_free(void* hndl) {
	if (hndl != NULL) {
		printf("release noise: %p\n", hndl);
		ma_noise_uninit((ma_noise*)hndl, NULL);
	}
	return 0;
}
int MANoise_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_noise* noise = (ma_noise*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
	switch (word) {
	case W_ARG_AMPLITUDE:
		*type = RXT_DECIMAL;
		arg->dec64 = noise->config.amplitude;
		break;
	case W_ARG_TYPE:
		*type = RXT_WORD;
		arg->int64 = type_words[W_TYPE_WHITE + noise->config.type];
		break;
	case W_ARG_FORMAT:
		*type = RXT_INTEGER;
		arg->int64 = noise->config.format;
		break;
	default:
		return PE_BAD_SELECT;	
	}
	return PE_USE;
}
int MANoise_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_noise* noise = (ma_noise*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
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
		printf("release waveform: %p\n", hndl);
		ma_waveform_uninit((ma_waveform*)hndl);
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
		*type = RXT_WORD;
		arg->int64 = type_words[W_TYPE_SINE + waveform->config.type];
		break;
		break;
	case W_ARG_FORMAT:
		*type = RXT_WORD;
		arg->int64 = type_words[W_TYPE_F32 + waveform->config.format];
		break;
	default:
		return PE_BAD_SELECT;	
	}
	return PE_USE;
}
int MAWaveform_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	ma_waveform* waveform = (ma_waveform*)hob->data;
	word = RL_FIND_WORD(arg_words, word);
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
	ma_result result;
	ma_resource_manager_config resourceManagerConfig;

	arg_words  = RL_MAP_WORDS(RXA_SERIES(frm,1));
	type_words = RL_MAP_WORDS(RXA_SERIES(frm,2));


	resourceManagerConfig = ma_resource_manager_config_init();
	resourceManagerConfig.decodedFormat     = ma_format_f32;    /* ma_format_f32 should almost always be used as that's what the engine (and most everything else) uses for mixing. */
	resourceManagerConfig.decodedChannels   = 0;                /* Setting the channel count to 0 will cause sounds to use their native channel count. */
	resourceManagerConfig.decodedSampleRate = 44100;            /* Using a consistent sample rate is useful for avoiding expensive resampling in the audio thread. This will result in resampling being performed by the loading thread(s). */

	result = ma_resource_manager_init(&resourceManagerConfig, &gResourceManager);
	if (result != MA_SUCCESS) {
		RETURN_ERROR("Failed to initialize resource manager.");
	}

	if (ma_context_init(NULL, 0, NULL, &gContext) != MA_SUCCESS) {
		RETURN_ERROR("Failed to initialize context.");
	}
	
	return RXR_TRUE;
}

COMMAND cmd_test(RXIFRM *frm, void *ctx) {
	ASSERT_ENGINE();

	// just a test for an issue with the noise type
	ma_noise *noise = malloc(sizeof(ma_noise));
	ma_sound *sound = malloc(sizeof(ma_sound));

	ma_noise_config config = ma_noise_config_init(ma_format_u8, 1, 2, 0, 1.0);
	if (MA_SUCCESS != ma_noise_init(&config, NULL, noise)) return RXR_FALSE;
	printf("type: %i\n", noise->config.type); // here it is OK (2)

	if (MA_SUCCESS != ma_sound_init_from_data_source(&pEngine->engine, noise, 0, NULL, sound)) return RXR_FALSE;
	printf("type: %i\n", noise->config.type); // here it is zero!
	ma_sound_start(sound);
	printf("type: %i\n", noise->config.type); // here it is zero! 

return RXR_TRUE;
}

COMMAND cmd_get_devices(RXIFRM *frm, void *ctx) {
	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_uint32 iDevice, iData;

	REBSER *result;
	REBSER *blk;
	RXIARG arg;
	

	if (MA_SUCCESS != ma_context_get_devices(&gContext, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount)) {
		RETURN_ERROR("Failed to retrieve device information.");
	}
	REBLEN len;
	REBSER *str = RL_MAKE_STRING(1000, FALSE); // 1024 bytes, latin1 (must be large enough!)
	RXA_SERIES(frm, 1) = str;
	RXA_TYPE(frm, 1) = RXT_STRING;
	RXA_INDEX(frm, 1) = 0;

	APPEND_STRING(str, "\nplayback:  [\n");

	//printf("Playback Devices\n");
	for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
		//printf("    %u: %s %u\n", iDevice, pPlaybackDeviceInfos[iDevice].name, pPlaybackDeviceInfos[iDevice].nativeDataFormatCount);
		APPEND_STRING(str, " \"%s\"\n", pPlaybackDeviceInfos[iDevice].name);
		for (iData = 0; iData < pPlaybackDeviceInfos[iDevice].nativeDataFormatCount; ++iData) {
		//	printf("        %u\n",          pPlaybackDeviceInfos[iDevice].nativeDataFormats[iData].sampleRate);
		}
	}	

	//printf("\n");

	APPEND_STRING(str, "]\ncapture: [\n");
	//printf("Capture Devices\n");
	for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
		APPEND_STRING(str, " \"%s\"\n", pCaptureDeviceInfos[iDevice].name);
		//printf("    %u: %s\n", iDevice, pCaptureDeviceInfos[iDevice].name);
	}

	APPEND_STRING(str, "]\n");


	return RXR_VALUE;
}

COMMAND cmd_init_playback(RXIFRM *frm, void *ctx) {
	REBHOB* hob;
	ma_result result;
	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_uint32 iChosenDevice;
	ma_device_config deviceConfig;
	ma_engine_config engineConfig;
	my_engine *engine;

	result = ma_context_get_devices(&gContext, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
	if (result != MA_SUCCESS) {
		RETURN_ERROR("Failed to enumerate playback devices.");
	}

	iChosenDevice = RXA_INT64(frm, 1) - 1;
	if (iChosenDevice < 0 || iChosenDevice >= playbackDeviceCount) {
		RETURN_ERROR("Invalid device index value.");
	}

	hob = RL_MAKE_HANDLE_CONTEXT(Handle_MAEngine);
	if (hob == NULL) return RXR_NONE;
	engine = (my_engine*)hob->data;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[iChosenDevice].id;
	deviceConfig.playback.format    = gResourceManager.config.decodedFormat;
	deviceConfig.playback.channels  = 0;
	deviceConfig.sampleRate         = gResourceManager.config.decodedSampleRate;
	deviceConfig.dataCallback       = data_callback;
	deviceConfig.pUserData          = engine;

	result = ma_device_init(&gContext, &deviceConfig, &engine->device);
	if (result != MA_SUCCESS) {
		debug_print("Failed to initialize device for %s.\n", pPlaybackDeviceInfos[iChosenDevice].name);
		RETURN_ERROR("Failed to initialize device.");
	}

	engineConfig = ma_engine_config_init();
	engineConfig.pDevice          = &engine->device;
	engineConfig.pResourceManager = &gResourceManager;
	engineConfig.noAutoStart      = RXA_REF(frm, 2);

	result = ma_engine_init(&engineConfig, &engine->engine);
	if (result != MA_SUCCESS) {
		debug_print("Failed to initialize engine for %s.\n", pPlaybackDeviceInfos[iChosenDevice].name);
		ma_device_uninit(&engine->device);
		RETURN_ERROR("Failed to initialize engine.");
	}

	pEngine = engine;
	pEngineHob = hob;
	hob->series = RL_MAKE_BLOCK(10); // for keeping references to sound handles

	RETURN_HANDLE(hob);
}


COMMAND cmd_play(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	ma_uint32 flags = 0;
	REBHOB* hob = NULL;
	REBI64 frames;

	ASSERT_ENGINE();

	if (RXA_TYPE(frm, 1) == RXT_FILE) {
		const char* file = (const char*)((REBSER*)RXA_ARG(frm, 1).series)->data;

		hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
		if (hob == NULL) return RXR_NONE;
		sound = (ma_sound*)hob->data;

		if (RXA_REF(frm, 2)) flags |= MA_SOUND_FLAG_STREAM;
		if (MA_SUCCESS != ma_sound_init_from_file(&pEngine->engine, file, flags, NULL, NULL, sound))
			RETURN_ERROR("Failed to initialize the sound from file.");

		hob->series = RL_MAKE_BLOCK(1);
		RL_SET_VALUE(hob->series, 0, RXA_ARG(frm, 1), RXT_FILE);

	} else if (ARG_Is_MASound(1)) {
		sound = ARG_MASound(1);
	} else if (ARG_Is_DataSource(1)) {
		ma_data_source *source = ARG_DataSource(1);

		hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
		if (hob == NULL) return RXR_NONE;
		sound = (ma_sound*)hob->data;
		if (MA_SUCCESS != ma_sound_init_from_data_source(&pEngine->engine, source, 0, NULL, sound))
			RETURN_ERROR("Failed to initialize the sound from a data source.");

		// store the datasource in the sound, so it it markable from GC
		hob->series = RL_MAKE_BLOCK(1);
		RL_SET_VALUE(hob->series, 0, RXA_ARG(frm, 1), RXT_HANDLE);
	} else {
		return RXR_NONE;
	}

	if (RXA_REF(frm, 4)) ma_sound_set_volume(sound, ARG_Float(5));
	ma_sound_start(sound);
	ma_sound_set_looping(sound, RXA_REF(frm, 3));

	if (RXA_REF(frm, 6)) { // fade
		if (RXA_TYPE(frm, 7) == RXT_INTEGER) {
			frames = RXA_INT64(frm, 7);
		} else {
			ma_uint64 sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
			frames = (RXA_TIME(frm, 7) * sampleRate) / 1000000000;
		}
		if (frames > 0) {
			ma_sound_set_fade_in_pcm_frames(sound, 0, 1, frames);
		}
	}

	if (hob) {
		RXA_HANDLE(frm, 1)       = hob;
		RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
		RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
		RXA_TYPE(frm, 1) = RXT_HANDLE;

		REBSER *blk = pEngineHob->series;
		RL_SET_VALUE(blk, blk->tail, RXA_ARG(frm, 1), RXT_HANDLE);
	}
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
	ma_sound  *sound;
	my_engine *engine;
	ma_uint64 sampleRate;
	ma_uint64 frame = 0;
	REBHOB  *hob = RXA_HANDLE_CONTEXT(frm, 1);
	if (!IS_USED_HOB(hob)) return RXR_FALSE; // already released handle!

	if (RXA_HANDLE_TYPE(frm, 1) == Handle_MASound) {
		sound = ARG_MASound(1);
		if (sound == NULL) return RXR_FALSE;

		ma_sound_set_looping(sound, RXA_REF(frm, 2));

		if (RXA_REF(frm, 3)) {
			if (RXA_INT64(frm, 4) < 0) RXA_INT64(frm, 4) = 0;
			if (RXA_TYPE(frm, 4) == RXT_INTEGER) {
				frame = RXA_UINT64(frm, 4);
			} else {
				sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
				frame = (RXA_TIME(frm, 4) * sampleRate) / 1000000000; // rate is per second
			}
		}
		ma_sound_seek_to_pcm_frame(sound, frame);

		if (RXA_REF(frm, 7)) { // /at
			if (RXA_INT64(frm, 8) < 0) RXA_INT64(frm, 8) = 0;
			if (RXA_TYPE(frm, 8) == RXT_INTEGER) {
				frame = RXA_UINT64(frm, 8);
			} else {
				sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
				frame = (RXA_TIME(frm, 8) * sampleRate) / 1000000000; // rate is per second
			}
			ma_sound_set_start_time_in_pcm_frames(sound, frame);
		}

		ma_sound_start(sound);

		if (RXA_REF(frm, 5)) { // /fade
			ma_uint64 fade;
			switch(RXA_TYPE(frm, 6)) {
			case RXT_INTEGER:
				fade = (ma_uint64)RXA_INT64(frm, 6);
				if (fade > 0) ma_sound_set_fade_in_pcm_frames(sound, 0, 1, fade);
				break;
			case RXT_TIME:
				fade = RXA_TIME(frm, 6) / 1000000; // time in ms
				if (fade > 0) ma_sound_set_fade_in_milliseconds(sound, 0, 1, fade);
				break;
			}
		}
		return RXR_VALUE;
	}
	else if (RXA_HANDLE_TYPE(frm, 1) == Handle_MAEngine) {
		engine = (my_engine*)hob->handle;
		if (RXA_REF(frm, 3)) {
			if (RXA_INT64(frm, 4) < 0) RXA_INT64(frm, 4) = 0; // only positive values
			if (RXA_TYPE(frm, 4) == RXT_INTEGER) {
				frame = RXA_INT64(frm, 4);
			} else {
				sampleRate = ma_engine_get_sample_rate(&engine->engine);
				frame = (RXA_TIME(frm, 4) * sampleRate) / 1000000000; // rate is per second
			}
			ma_engine_set_time_in_pcm_frames(&engine->engine, (ma_uint64)frame);
		}
		ma_device_start(&engine->device);
		return RXR_VALUE;
	}
	return RXR_FALSE;
}

COMMAND cmd_stop(RXIFRM *frm, void *ctx) {
	ma_sound  *sound;
	my_engine *engine;
	REBINT type;
	REBI64 fade;
	REBHOB  *hob = RXA_HANDLE_CONTEXT(frm, 1);
	if (!IS_USED_HOB(hob)) return RXR_FALSE; // already released handle!

	if (RXA_HANDLE_TYPE(frm, 1) == Handle_MASound) {
		sound = (ma_sound*)hob->handle;
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
	else if (RXA_HANDLE_TYPE(frm, 1) == Handle_MAEngine) {
		engine = (my_engine*)hob->handle;
		ma_device_stop(&engine->device);
		return RXR_VALUE;
	}
	return RXR_FALSE;
}

COMMAND cmd_fade(RXIFRM *frm, void *ctx) {
	ma_sound *sound;
	REBI64 frames;
	float volumeBeg, volumeEnd;

	sound  = ARG_MASound(1);
	if (sound == NULL) return RXR_FALSE;

	if (RXA_TYPE(frm, 2) == RXT_INTEGER) {
		frames = RXA_INT64(frm, 2);
	} else {
		ma_uint64 sampleRate = ma_engine_get_sample_rate(ma_sound_get_engine(sound));
		frames = (RXA_TIME(frm, 2) * sampleRate) / 1000000000;
	}

	volumeBeg = ARG_Double(3);
	volumeEnd = ARG_Double(4);

	if (frames < 0) frames = 1;
	//if (volumeBeg == 0.0) ma_sound_set_volume(sound, 0.01);
	//printf("fade from %f to %f in %llu\n", ma_sound_get_volume(sound), volumeEnd, frames);
	ma_sound_set_fade_in_pcm_frames(sound, volumeBeg, volumeEnd, frames);

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

	ASSERT_ENGINE();

	hob = RL_MAKE_HANDLE_CONTEXT(Handle_MASound);
	if (hob == NULL) return RXR_NONE;
	sound = (ma_sound*)hob->data;

	const char* file = (const char*)((REBSER*)RXA_ARG(frm, 1).series)->data;

	if (MA_SUCCESS != ma_sound_init_from_file(&pEngine->engine, file, 0, NULL, NULL, sound))
		RETURN_ERROR("Failed to initialize the sound from a file.");

	hob->series = RL_MAKE_BLOCK(1);
	RL_SET_VALUE(hob->series, 0, RXA_ARG(frm, 1), RXT_FILE);

	RXA_HANDLE(frm, 1)       = hob;
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;

	REBSER *blk = pEngineHob->series;
	RL_SET_VALUE(blk, blk->tail, RXA_ARG(frm, 1), RXT_HANDLE);

	return RXR_VALUE;
}


COMMAND cmd_noise_node(RXIFRM *frm, void *ctx) {
	ma_noise *noise;
	REBHOB* hob = RL_MAKE_HANDLE_CONTEXT(Handle_MANoise);
	if (hob == NULL) return RXR_NONE;
	noise = (ma_noise*)hob->data;

	REBCNT format = ARG_Int32(6);
	if (!format || format >= ma_format_count) format = ma_format_s16;

	ma_noise_config config = ma_noise_config_init(
		format,        //= format
		1,
		ARG_Int32(1),  //= type
		ARG_Int32(4),  //= seed
		ARG_Double(2)  //= apmplitude
	);

	if (MA_SUCCESS != ma_noise_init(&config, NULL, noise)) return RXR_FALSE;

	hob->series = NULL;
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

	ASSERT_ENGINE();

	REBCNT format = ARG_Int32(5);
	if (!format || format >= ma_format_count) format = ma_format_s16;

	ma_waveform_config config = ma_waveform_config_init(
		format,        //= format
		1,             //= channels
		ma_engine_get_sample_rate(&pEngine->engine),
		ARG_Int32(1),  //= type
		ARG_Double(2), //= apmplitude
		ARG_Double(3)  //= frequency
	);

	if (MA_SUCCESS != ma_waveform_init(&config, waveform)) return RXR_FALSE;

	hob->series = NULL;
	RXA_HANDLE(frm, 1)       = hob;
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;

	return RXR_VALUE;
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





