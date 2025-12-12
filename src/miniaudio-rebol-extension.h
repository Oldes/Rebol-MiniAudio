//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/MiniAudio extension
// SPDX-License-Identifier: MIT
// =============================================================================
// NOTE: auto-generated file, do not modify!

#include "rebol-extension.h"
#include "miniaudio.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 14
#define MIN_REBOL_UPD 1
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

typedef struct MAContext {
	ma_engine* engine;
	ma_device* device;
	RXICBI    callback;
} MAContext;

extern REBCNT Handle_MAEngine;
extern REBCNT Handle_MASound;
extern REBCNT Handle_MANoise;
extern REBCNT Handle_MAWaveform;
extern REBCNT Handle_MADelay;
extern REBCNT Handle_MAGroup;

extern u32* arg_words;
extern u32* type_words;

enum ext_commands {
	CMD_MINIAUDIO_INIT_WORDS,
	CMD_MINIAUDIO_VERSION,
	CMD_MINIAUDIO_GET_DEVICES,
	CMD_MINIAUDIO_INIT_PLAYBACK,
	CMD_MINIAUDIO_LOAD,
	CMD_MINIAUDIO_PLAY,
	CMD_MINIAUDIO_PAUSE,
	CMD_MINIAUDIO_START,
	CMD_MINIAUDIO_STOP,
	CMD_MINIAUDIO_FADE,
	CMD_MINIAUDIO_SEEK,
	CMD_MINIAUDIO_MAKE_NOISE_NODE,
	CMD_MINIAUDIO_MAKE_WAVEFORM_NODE,
	CMD_MINIAUDIO_MAKE_DELAY_NODE,
	CMD_MINIAUDIO_MAKE_GROUP_NODE,
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
int cmd_version(RXIFRM *frm, void *ctx);
int cmd_get_devices(RXIFRM *frm, void *ctx);
int cmd_init_playback(RXIFRM *frm, void *ctx);
int cmd_load(RXIFRM *frm, void *ctx);
int cmd_play(RXIFRM *frm, void *ctx);
int cmd_pause(RXIFRM *frm, void *ctx);
int cmd_start(RXIFRM *frm, void *ctx);
int cmd_stop(RXIFRM *frm, void *ctx);
int cmd_fade(RXIFRM *frm, void *ctx);
int cmd_seek(RXIFRM *frm, void *ctx);
int cmd_make_noise_node(RXIFRM *frm, void *ctx);
int cmd_make_waveform_node(RXIFRM *frm, void *ctx);
int cmd_make_delay_node(RXIFRM *frm, void *ctx);
int cmd_make_group_node(RXIFRM *frm, void *ctx);
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
	W_ARG_OUTPUTS,
	W_ARG_OUTPUT,
	W_ARG_RESOURCES,
	W_ARG_CHANNELS,
	W_ARG_GAIN_DB,
	W_ARG_AMPLITUDE,
	W_ARG_FORMAT,
	W_ARG_TYPE,
	W_ARG_FREQUENCY,
	W_ARG_DELAY,
	W_ARG_DECAY,
	W_ARG_DRY,
	W_ARG_WET
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
	"REBOL [Title: \"Rebol MiniAudio Extension\" Name: miniaudio Type: module Version: 0.11.22 Needs: 3.14.1 Author: Oldes Date: 12-Dec-2025/11:43:17 License: MIT Url: https://github.com/Oldes/Rebol-MiniAudio]\n"\
	"init-words: command [args [block!] type [block!]]\n"\
	"version: command [\"Native MiniAudio version\"]\n"\
	"get-devices: command [\"Retrive playback/capture device names\"]\n"\
	"init-playback: command [\"Initialize a playback device\" index [integer!] /pause \"Don't start it automatically\" /channels \"The number of channels to use for playback\" number [integer!] {When set to 0 the device's native channel count will be used} /period \"Hint for making up the device's entire buffer\" size [integer!] \"The desired size of a period in milliseconds\" /callback {On-data callback (two args.. buffer frames, and engine total frames)} context [object!] \"The function's context\" word [word!] \"The function's name\"]\n"\
	"load: command [\"Loads a file and returns sound's handle\" sound [file!] /group {Group of sounds which have their own effect processing and volume control} node [handle!] \"ma-group handle\"]\n"\
	"play: command [{Loads a file (if not already loaded) and starts playing it. Returns a sound handle.} sound [file! handle!] \"Source file or a ma-sound handle\" /stream \"Do not load the entire sound into memory\" /loop \"Turn looping on\" /volume vol [percent! decimal!] /fade in [integer! time!] \"PCM frames or time\" /group {Group of sounds which have their own effect processing and volume control} node [handle!] \"ma-group handle\"]\n"\
	"pause: command [\"Pause sound playback\" sound [handle!]]\n"\
	"start: command [\"Start sound or device playback\" handle [handle!] \"ma-sound or ma-engine handle\" /loop \"Turn looping on (only for sounds)\" /seek \"Starting position\" frames [integer! time!] \"PCM frames or time\" /fade \"Fade in the sound\" in [integer! time!] \"PCM frames or time\" /at {Absolute engine time when the sound should be started} time [integer! time!] \"PCM frames or time\"]\n"\
	"stop: command [\"Stop sound or device playback\" handle [handle!] \"ma-sound or ma-engine handle\" /fade out [integer! time!] \"PCM frames or time (only for sounds)\"]\n"\
	"fade: command [\"Fade sound volume\" sound [handle!] frames [integer! time!] start [percent! decimal!] end [percent! decimal!]]\n"\
	"seek: command [\"Seek to specified position\" sound [handle!] frames [integer! time!] /relative \"Relative to the current sound position\"]\n"\
	"make-noise-node: command [\"Creates a noise node for generating random noise\" type [integer!] \"The type of noise to generate (0 - 2)\" amplitude [decimal!] \"The peak amplitude of the noise\" /seed \"Optional random seed\" val [integer!] /format {The sample format (default is 2 = signed 16bit float)} frm [integer!] \"Value betweem 1 - 5\"]\n"\
	"make-waveform-node: command [\"Creates a sound waveform node\" type [integer!] \"The type of waveform to generate (0 - 3)\" amplitude [decimal!] \"The peak amplitude of the waveform\" frequency [decimal!] \"The frequency of the waveform in Hertz (Hz)\" /format {The sample format (default is 2 = signed 16bit float)} frm [integer!] \"Value betweem 1 - 5\"]\n"\
	"make-delay-node: command [\"Creates a delay (echo) sound node\" delay [decimal! integer! time!] {The time before the echo is heard. Seconds, PCM frames or time.} decay [decimal! percent!] {Feedback decay (0.0 - 1.0). Affects how quickly or gradually the echoes fade away. 0 means no feedback.} /dry \"The mix level of the dry (original) sound\" d [decimal! percent!] /wet \"The mix level of the wet (delayed) sound\" w [decimal! percent!]]\n"\
	"make-group-node: command [\"Creates a sound group node\"]\n"\
	"volume: command [\"Set the volume\" sound [handle!] volume [percent! decimal!]]\n"\
	"volume?: command [\"Get the volume\" sound [handle!]]\n"\
	"pan: command [\"Set the pan\" sound [handle!] pan [decimal!]]\n"\
	"pan?: command [\"Get the pan\" sound [handle!]]\n"\
	"pitch: command [\"Set the pitch\" sound [handle!] pitch [decimal!]]\n"\
	"pitch?: command [\"Get the pitch\" sound [handle!]]\n"\
	"looping: command [\"Set the looping\" sound [handle!] value [logic!]]\n"\
	"looping?: command [\"Get the looping\" sound [handle!]]\n"\
	"end?: command [\"Return true if sound ended\" sound [handle!]]\n"\
	"init-words [volume pan pitch position cursor time duration frames sample-rate spatialize is-looping is-playing at-end start stop x y z source outputs output resources channels gain-db amplitude format type frequency delay decay dry wet][white pink brownian sine square triangle sawtooth f32 s16 s24 s32 u8]\n"\
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

