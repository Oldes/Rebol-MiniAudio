REBOL [
	title:  "Rebol/MiniAudio module builder"
	type:    module
	date:    21-Aug-2023
	home:    https://github.com/Oldes/Rebol-MiniAudio
	version: 1.0.0
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	init-words: [sound [block!] noise [block!]]
	test:  ["..."]
	load:  [sound [file!]]
	;copy:  [sound [handle!]]
	play:  [
		"Loads a file (if not already loaded) and starts playing it. Returns a sound handle."
		sound [file! handle!] "Source file or a ma-sound handle"
		/stream "Do not load the entire sound into memory"
		/loop   "Turn looping on"
	]
	pause: [sound [handle!]]
	
	start: [sound [handle!] /loop /seek frames [integer! time!]]
	stop:  [sound [handle!] /fade out [integer! time!] "PCM frames or time"]
	seek:  [sound [handle!] frames [integer! time!] /relative]
	volume:  ["Set the sound volume" sound [handle!] volume [percent! decimal!]]
	volume?: ["Get the sound volume" sound [handle!]]
	pan:     ["Set the sound pan"    sound [handle!] pan [decimal!]]
	pan?:    ["Get the sound pan"    sound [handle!]]
	pitch:   ["Set the sound pitch"  sound [handle!] pitch [decimal!]]
	pitch?:  ["Get the sound pitch"  sound [handle!]]
	looping: [sound [handle!] value [logic!]]
	looping?:    [sound [handle!]]
	end?:        [sound [handle!]]

	noise-node:  ["Create a noise node data source" type [integer!] amplitude [decimal!] /seed value [integer!]]
	waveform-node: [type [integer!] amplitude [decimal!] frequency [decimal!]]

	;shutdown: ["Uninitialize the engine"]
]

sound-words: [
	volume
	pan
	pitch
	duration
	cursor
	frames
	sample-rate
	spatialization
	loop
	end?
	playing?
	file
]
arg-words: [
	type
	amplitude
	frequency
]

;-------------------------------------- ----------------------------------------
reb-code: {REBOL [Title: {Rebol MiniAudio Extension} Type: module]}
enu-commands:  "" ;; command name enumerations
cmd-declares:  "" ;; command function declarations
cmd-dispatch:  "" ;; command functionm dispatcher
ma-sound-words: "enum ma_sound_words {W_SOUND_0"
ma-arg-words: "enum ma_arg_words {W_ARG_0"

;- generate C and Rebol code from the command specifications -------------------
foreach [name spec] commands [
	append reb-code ajoin [lf name ": command "]
	new-line/all spec false
	append/only reb-code mold spec

	name: form name
	replace/all name #"-" #"_"
	replace/all name #"?" #"q"
	
	append enu-commands ajoin ["^/^-CMD_MINIAUDIO_" uppercase copy name #","]

	append cmd-declares ajoin ["^/int cmd_" name "(RXIFRM *frm, void *ctx);"]
	append cmd-dispatch ajoin ["^-cmd_" name ",^/"]
]

;- additional Rebol initialization code ----------------------------------------
foreach word sound-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	replace/all word #"?" #"Q"
	append ma-sound-words ajoin [",^/^-W_SOUND_" word]
]

foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-arg-words ajoin [",^/^-W_ARG_" word]
]

append ma-sound-words "^/};"
append ma-arg-words "^/};"
append reb-code ajoin [{
init-words } mold/flat sound-words mold/flat arg-words {
protect/hide 'init-words
type_sine:     0
type_square:   1
type_triangle: 2
type_sawtooth: 3
}
]

;append reb-code {}

;print reb-code

;- convert Rebol code to C-string ----------------------------------------------
init-code: copy ""
foreach line split reb-code lf [
	replace/all line #"^"" {\"}
	append init-code ajoin [{\^/^-"} line {\n"}] 
]

;-- C file templates -----------------------------------------------------------
header: {
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

extern u32* sound_words;
extern u32* arg_words;

enum ext_commands {$enu-commands
};

$cmd-declares
$ma-sound-words
$ma-arg-words

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE $init-code

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

}
;;------------------------------------------------------------------------------
ctable: {
//
// auto-generated file, do not modify!
//
#include "miniaudio-rebol-extension.h"
MyCommandPointer Command[] = {
$cmd-dispatch};
}

;- output generated files ------------------------------------------------------
write %miniaudio-rebol-extension.h reword :header self
write %miniaudio-commands-table.c  reword :ctable self

