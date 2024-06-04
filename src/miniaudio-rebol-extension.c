//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// SPDX-License-Identifier: MIT
// =============================================================================
// Rebol/MiniAudio extension
// =============================================================================

#include "miniaudio-rebol-extension.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//==== Globals ===============================================================//
extern MyCommandPointer Command[];
REBCNT Handle_MAEngine;
REBCNT Handle_MASound;
REBCNT Handle_MANoise;
REBCNT Handle_MAWaveform;
REBCNT Handle_MADelay;
REBCNT Handle_MAGroup;

u32* arg_words;
u32* type_words;
//============================================================================//

static const char* init_block = MINIAUDIO_EXT_INIT_CODE;

int Common_mold(REBHOB *hob, REBSER *ser);

int MAEngine_free(void* hndl);
int MAEngine_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MAEngine_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MASound_free(void* hndl);
int MASound_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MASound_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MANoise_free(void* hndl);
int MANoise_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MANoise_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MAWaveform_free(void* hndl);
int MAWaveform_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MAWaveform_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MADelay_free(void* hndl);
int MADelay_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MADelay_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int MAGroup_free(void* hndl);
int MAGroup_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int MAGroup_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

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
	spec.mold = Common_mold;

	spec.size      = sizeof(my_engine); // It is MY_, not MA_! 
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
	spec.free      = MAEngine_free;
	spec.get_path  = MAEngine_get_path;
	spec.set_path  = MAEngine_set_path;
	Handle_MAEngine  = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-engine", &spec);

	spec.size      = sizeof(ma_sound);
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
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

	spec.size      = sizeof(ma_delay_node);
	spec.flags     = 0;
	spec.free      = MADelay_free;
	spec.get_path  = MADelay_get_path;
	spec.set_path  = MADelay_set_path;
	Handle_MADelay = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-delay", &spec);

	spec.size      = sizeof(ma_sound_group);
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
	spec.free      = MAGroup_free;
	spec.get_path  = MAGroup_get_path;
	spec.set_path  = MAGroup_set_path;
	Handle_MAGroup = RL_REGISTER_HANDLE_SPEC((REBYTE*)"ma-group", &spec);

	return init_block;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	return Command[cmd](frm, ctx);
}
