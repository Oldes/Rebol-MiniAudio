
//
// auto-generated file, do not modify!
//

#include "rebol-extension.h"
#include "miniaudio.h"

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 12
#define MIN_REBOL_UPD 1
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

extern REBCNT Handle_MASound;
extern REBCNT Handle_MANoise;
extern REBCNT Handle_MAWaveform;

u32* sound_words;
u32* arg_words;

enum ext_commands {
	CMD_MINIAUDIO_INIT_WORDS,
	CMD_MINIAUDIO_TEST,
	CMD_MINIAUDIO_LOAD,
	CMD_MINIAUDIO_PLAY,
	CMD_MINIAUDIO_PAUSE,
	CMD_MINIAUDIO_START,
	CMD_MINIAUDIO_STOP,
	CMD_MINIAUDIO_SEEK,
	CMD_MINIAUDIO_VOLUME,
	CMD_MINIAUDIO_VOLUMEQ,
	CMD_MINIAUDIO_PAN,
	CMD_MINIAUDIO_PANQ,
	CMD_MINIAUDIO_PITCH,
	CMD_MINIAUDIO_PITCHQ,
	CMD_MINIAUDIO_LOOPING,
	CMD_MINIAUDIO_LOOPINGQ,
	CMD_MINIAUDIO_ENDQ,
	CMD_MINIAUDIO_NOISE_NODE,
	CMD_MINIAUDIO_WAVEFORM_NODE,
};


int cmd_init_words(RXIFRM *frm, void *ctx);
int cmd_test(RXIFRM *frm, void *ctx);
int cmd_load(RXIFRM *frm, void *ctx);
int cmd_play(RXIFRM *frm, void *ctx);
int cmd_pause(RXIFRM *frm, void *ctx);
int cmd_start(RXIFRM *frm, void *ctx);
int cmd_stop(RXIFRM *frm, void *ctx);
int cmd_seek(RXIFRM *frm, void *ctx);
int cmd_volume(RXIFRM *frm, void *ctx);
int cmd_volumeq(RXIFRM *frm, void *ctx);
int cmd_pan(RXIFRM *frm, void *ctx);
int cmd_panq(RXIFRM *frm, void *ctx);
int cmd_pitch(RXIFRM *frm, void *ctx);
int cmd_pitchq(RXIFRM *frm, void *ctx);
int cmd_looping(RXIFRM *frm, void *ctx);
int cmd_loopingq(RXIFRM *frm, void *ctx);
int cmd_endq(RXIFRM *frm, void *ctx);
int cmd_noise_node(RXIFRM *frm, void *ctx);
int cmd_waveform_node(RXIFRM *frm, void *ctx);
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
	W_SOUND_FILE
};
enum ma_arg_words {W_ARG_0,
	W_ARG_TYPE,
	W_ARG_AMPLITUDE,
	W_ARG_FREQUENCY
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE \
	"REBOL [Title: {Rebol MiniAudio Extension} Type: module]\n"\
	"init-words: command [sound [block!] noise [block!]]\n"\
	"test: command [\"...\"]\n"\
	"load: command [sound [file!]]\n"\
	"play: command [{Loads a file (if not already loaded) and starts playing it. Returns a sound handle.} sound [file! handle!] \"Source file or a ma-sound handle\" /stream \"Do not load the entire sound into memory\" /loop \"Turn looping on\"]\n"\
	"pause: command [sound [handle!]]\n"\
	"start: command [sound [handle!] /loop /seek frames [integer! time!]]\n"\
	"stop: command [sound [handle!] /fade out [integer! time!] \"PCM frames or time\"]\n"\
	"seek: command [sound [handle!] frames [integer! time!] /relative]\n"\
	"volume: command [\"Set the sound volume\" sound [handle!] volume [percent! decimal!]]\n"\
	"volume?: command [\"Get the sound volume\" sound [handle!]]\n"\
	"pan: command [\"Set the sound pan\" sound [handle!] pan [decimal!]]\n"\
	"pan?: command [\"Get the sound pan\" sound [handle!]]\n"\
	"pitch: command [\"Set the sound pitch\" sound [handle!] pitch [decimal!]]\n"\
	"pitch?: command [\"Get the sound pitch\" sound [handle!]]\n"\
	"looping: command [sound [handle!] value [logic!]]\n"\
	"looping?: command [sound [handle!]]\n"\
	"end?: command [sound [handle!]]\n"\
	"noise-node: command [\"Create a noise node data source\" type [integer!] amplitude [decimal!] /seed value [integer!]]\n"\
	"waveform-node: command [type [integer!] amplitude [decimal!] frequency [decimal!]]\n"\
	"init-words [volume pan pitch duration cursor frames sample-rate spatialization loop end? playing? file][type amplitude frequency]\n"\
	"protect/hide 'init-words\n"\
	"type_sine:     0\n"\
	"type_square:   1\n"\
	"type_triangle: 2\n"\
	"type_sawtooth: 3\n"\
	"\n"

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

