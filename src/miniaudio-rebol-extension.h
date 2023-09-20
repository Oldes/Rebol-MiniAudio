
//
// auto-generated file, do not modify!
//

#include "rebol-extension.h"
#include "miniaudio.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 13
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

extern u32* arg_words;
extern u32* type_words;

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

enum ma_arg_words {W_ARG_0,
	W_ARG_VOLUME,
	W_ARG_PAN,
	W_ARG_PITCH,
	W_ARG_POSITION,
	W_ARG_CURSOR,
	W_ARG_TIME,
	W_ARG_DURATION,
	W_ARG_FRAMES,
	W_ARG_SAMPLE_RATE,
	W_ARG_SPATIALIZE,
	W_ARG_IS_LOOPING,
	W_ARG_IS_PLAYING,
	W_ARG_AT_END,
	W_ARG_START,
	W_ARG_STOP,
	W_ARG_X,
	W_ARG_Y,
	W_ARG_Z,
	W_ARG_SOURCE,
	W_ARG_RESOURCES,
	W_ARG_CHANNELS,
	W_ARG_GAIN_DB,
	W_ARG_AMPLITUDE,
	W_ARG_FORMAT,
	W_ARG_TYPE,
	W_ARG_FREQUENCY
};
enum ma_type_words {W_TYPE_0,
	W_TYPE_WHITE,
	W_TYPE_PINK,
	W_TYPE_BROWNIAN,
	W_TYPE_SINE,
	W_TYPE_SQUARE,
	W_TYPE_TRIANGLE,
	W_TYPE_SAWTOOTH,
	W_TYPE_F32,
	W_TYPE_S16,
	W_TYPE_S24,
	W_TYPE_S32,
	W_TYPE_U8
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE \
	"REBOL [Title: {Rebol MiniAudio Extension} Type: module Version: 0.11.18.1]\n"\
	"init-words: command [args [block!] type [block!]]\n"\
	"get-devices: command [\"Retrive playback/capture device names\"]\n"\
	"init-playback: command [\"Initialize a playback device\" index [integer!] /pause \"Don't start it automatically\" /channels \"The number of channels to use for playback\" number [integer!] {When set to 0 the device's native channel count will be used}]\n"\
	"load: command [\"Loads a file and returns sound's handle\" sound [file!]]\n"\
	"play: command [{Loads a file (if not already loaded) and starts playing it. Returns a sound handle.} sound [file! handle!] \"Source file or a ma-sound handle\" /stream \"Do not load the entire sound into memory\" /loop \"Turn looping on\" /volume vol [percent! decimal!] /fade in [integer! time!] \"PCM frames or time\"]\n"\
	"pause: command [\"Pause sound playback\" sound [handle!]]\n"\
	"start: command [\"Start sound or device playback\" handle [handle!] \"ma-sound or ma-engine handle\" /loop \"Turn looping on (only for sounds)\" /seek \"Starting position\" frames [integer! time!] \"PCM frames or time\" /fade \"Fade in the sound\" in [integer! time!] \"PCM frames or time\" /at {Absolute engine time when the sound should be started} time [integer! time!] \"PCM frames or time\"]\n"\
	"stop: command [\"Stop sound or device playback\" handle [handle!] \"ma-sound or ma-engine handle\" /fade out [integer! time!] \"PCM frames or time (only for sounds)\"]\n"\
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
	"init-words [volume pan pitch position cursor time duration frames sample-rate spatialize is-looping is-playing at-end start stop x y z source resources channels gain-db amplitude format type frequency][white pink brownian sine square triangle sawtooth f32 s16 s24 s32 u8]\n"\
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

