
//
// auto-generated file, do not modify!
//

#include "rebol-extension.h"
#include "miniaudio.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 12
#define MIN_REBOL_UPD 1
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

typedef struct my_engine {
	ma_engine engine;
	ma_device device;
} my_engine;

extern REBCNT Handle_MAEngine;
extern REBCNT Handle_MASound;
extern REBCNT Handle_MANoise;
extern REBCNT Handle_MAWaveform;

extern u32* sound_words;
extern u32* arg_words;

enum ext_commands {
	CMD_MINIAUDIO_INIT_WORDS,
	CMD_MINIAUDIO_GET_DEVICES,
	CMD_MINIAUDIO_INIT_PLAYBACK,
	CMD_MINIAUDIO_LOAD,
	CMD_MINIAUDIO_PLAY,
	CMD_MINIAUDIO_PAUSE,
	CMD_MINIAUDIO_START,
	CMD_MINIAUDIO_STOP,
	CMD_MINIAUDIO_FADE,
	CMD_MINIAUDIO_SEEK,
	CMD_MINIAUDIO_NOISE_NODE,
	CMD_MINIAUDIO_WAVEFORM_NODE,
	CMD_MINIAUDIO_VOLUME,
	CMD_MINIAUDIO_VOLUMEQ,
	CMD_MINIAUDIO_PAN,
	CMD_MINIAUDIO_PANQ,
	CMD_MINIAUDIO_PITCH,
	CMD_MINIAUDIO_PITCHQ,
	CMD_MINIAUDIO_LOOPING,
	CMD_MINIAUDIO_LOOPINGQ,
	CMD_MINIAUDIO_ENDQ,
};


int cmd_init_words(RXIFRM *frm, void *ctx);
int cmd_get_devices(RXIFRM *frm, void *ctx);
int cmd_init_playback(RXIFRM *frm, void *ctx);
int cmd_load(RXIFRM *frm, void *ctx);
int cmd_play(RXIFRM *frm, void *ctx);
int cmd_pause(RXIFRM *frm, void *ctx);
int cmd_start(RXIFRM *frm, void *ctx);
int cmd_stop(RXIFRM *frm, void *ctx);
int cmd_fade(RXIFRM *frm, void *ctx);
int cmd_seek(RXIFRM *frm, void *ctx);
int cmd_noise_node(RXIFRM *frm, void *ctx);
int cmd_waveform_node(RXIFRM *frm, void *ctx);
int cmd_volume(RXIFRM *frm, void *ctx);
int cmd_volumeq(RXIFRM *frm, void *ctx);
int cmd_pan(RXIFRM *frm, void *ctx);
int cmd_panq(RXIFRM *frm, void *ctx);
int cmd_pitch(RXIFRM *frm, void *ctx);
int cmd_pitchq(RXIFRM *frm, void *ctx);
int cmd_looping(RXIFRM *frm, void *ctx);
int cmd_loopingq(RXIFRM *frm, void *ctx);
int cmd_endq(RXIFRM *frm, void *ctx);
enum ma_sound_words {W_SOUND_0,
	W_SOUND_VOLUME,
	W_SOUND_PAN,
	W_SOUND_PITCH,
	W_SOUND_DURATION,
	W_SOUND_CURSOR,
	W_SOUND_FRAMES,
	W_SOUND_SAMPLE_RATE,
	W_SOUND_SPATIALIZATION,
	W_SOUND_LOOP,
	W_SOUND_ENDQ,
	W_SOUND_PLAYINGQ,
	W_SOUND_FILE,
	W_SOUND_START,
	W_SOUND_STOP
};
enum ma_arg_words {W_ARG_0,
	W_ARG_VOLUME,
	W_ARG_TYPE,
	W_ARG_AMPLITUDE,
	W_ARG_FREQUENCY,
	W_ARG_FORMAT,
	W_ARG_FRAMES,
	W_ARG_TIME,
	W_ARG_RESOURCES
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE \
	"REBOL [Title: {Rebol MiniAudio Extension} Type: module Version: 1.0.0]\n"\
	"init-words: command [sound [block!] noise [block!]]\n"\
	"get-devices: command [\"Retrive playback/capture device names\"]\n"\
	"init-playback: command [\"Initialize a playback device\" index [integer!]]\n"\
	"load: command [\"Loads a file and returns sound's handle\" sound [file!]]\n"\
	"play: command [{Loads a file (if not already loaded) and starts playing it. Returns a sound handle.} sound [file! handle!] \"Source file or a ma-sound handle\" /stream \"Do not load the entire sound into memory\" /loop \"Turn looping on\" /volume vol [percent! decimal!] /fade in [integer! time!] \"PCM frames or time\"]\n"\
	"pause: command [\"Pause sound playback\" sound [handle!]]\n"\
	"start: command [\"Start sound playback\" sound [handle!] /loop \"Turn looping on\" /seek \"Specify starting position\" frames [integer! time!] /fade in [integer! time!] \"PCM frames or time\"]\n"\
	"stop: command [\"Stop sound playback\" sound [handle!] /fade out [integer! time!] \"PCM frames or time\"]\n"\
	"fade: command [\"Fade sound volume\" sound [handle!] frames [integer! time!] start [percent! decimal!] end [percent! decimal!]]\n"\
	"seek: command [\"Seek to specified position\" sound [handle!] frames [integer! time!] /relative \"Relative to the current sound position\"]\n"\
	"noise-node: command [\"Create a noise node data source\" type [integer!] amplitude [decimal!] /seed \"Optional random seed\" val [integer!] /format {The sample format (default is 2 = signed 16bit float)} frm [integer!] \"Value betweem 1 - 5\"]\n"\
	"waveform-node: command [type [integer!] amplitude [decimal!] frequency [decimal!] /format {The sample format (default is 2 = signed 16bit float)} frm [integer!] \"Value betweem 1 - 5\"]\n"\
	"volume: command [\"Set the volume\" sound [handle!] volume [percent! decimal!]]\n"\
	"volume?: command [\"Get the volume\" sound [handle!]]\n"\
	"pan: command [\"Set the pan\" sound [handle!] pan [decimal!]]\n"\
	"pan?: command [\"Get the pan\" sound [handle!]]\n"\
	"pitch: command [\"Set the pitch\" sound [handle!] pitch [decimal!]]\n"\
	"pitch?: command [\"Get the pitch\" sound [handle!]]\n"\
	"looping: command [\"Set the looping\" sound [handle!] value [logic!]]\n"\
	"looping?: command [\"Get the looping\" sound [handle!]]\n"\
	"end?: command [\"Return true if sound ended\" sound [handle!]]\n"\
	"init-words [volume pan pitch duration cursor frames sample-rate spatialization loop end? playing? file start stop][volume type amplitude frequency format frames time resources]\n"\
	"protect/hide 'init-words\n"\
	"\n"\
	";; Waveform types\n"\
	"type_sine:     0\n"\
	"type_square:   1\n"\
	"type_triangle: 2\n"\
	"type_sawtooth: 3\n"\
	"\n"\
	";; Sample data formats\n"\
	"format_u8:     1\n"\
	"format_s16:    2  ; Seems to be the most widely supported format.\n"\
	"format_s24:    3  ; Tightly packed. 3 bytes per sample.\n"\
	"format_s32:    4\n"\
	"format_f32:    5\n"\
	"\n"\
	"white-noise: does [play noise-node 0 0.5]\n"\
	"\n"

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

