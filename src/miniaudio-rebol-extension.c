// =============================================================================
// Rebol/MiniAudio extension
// =============================================================================

 #define MA_DEBUG_OUTPUT
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio-rebol-extension.h"
#include "miniaudio.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//==== Globals ===============================================================//
extern MyCommandPointer Command[];
REBCNT Handle_MASound;
REBCNT Handle_MANoise;
REBCNT Handle_MAWaveform;
//============================================================================//

static const char* init_block = MINIAUDIO_EXT_INIT_CODE;

int MASound_free(void* hndl);
int MASound_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MASound_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MANoise_free(void* hndl);
int MANoise_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MANoise_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MAWaveform_free(void* hndl);
int MAWaveform_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MAWaveform_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
	RL = lib;
	REBYTE ver[8];
	RL_VERSION(ver);
	debug_print(
		"RXinit miniaudio-extension; Rebol v%i.%i.%i\n",
		ver[1], ver[2], ver[3]);

	if (MIN_REBOL_VERSION > VERSION(ver[1], ver[2], ver[3])) {
		debug_print(
			"Needs at least Rebol v%i.%i.%i!\n",
		     MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD);
		return 0;
	}
	if (!CHECK_STRUCT_ALIGN) {
		trace("CHECK_STRUCT_ALIGN failed!");
		return 0;
	}


	REBHSP spec;

	spec.size      = sizeof(ma_sound);
	spec.flags     = 0;
	spec.free      = MASound_free;
	spec.get_path  = MASound_get_path;
	spec.set_path  = MASound_set_path;
	Handle_MASound  = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-sound", &spec);

	spec.size      = sizeof(ma_noise);
	spec.flags     = 0;
	spec.free      = MANoise_free;
	spec.get_path  = MANoise_get_path;
	spec.set_path  = MANoise_set_path;
	Handle_MANoise  = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-noise", &spec);

	spec.size      = sizeof(ma_waveform);
	spec.flags     = 0;
	spec.free      = MAWaveform_free;
	spec.get_path  = MAWaveform_get_path;
	spec.set_path  = MAWaveform_set_path;
	Handle_MAWaveform = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-waveform", &spec);

	return init_block;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	return Command[cmd](frm, ctx);
}