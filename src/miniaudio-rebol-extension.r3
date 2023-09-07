REBOL [
	title:  "Rebol/MiniAudio module builder"
	type:    module
	date:    6-Sep-2023
	home:    https://github.com/Oldes/Rebol-MiniAudio
	version: 1.0.0
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	init-words:    [sound [block!] noise [block!]] ;; used internaly only!
	;test:  ["..."]

	get-devices:   ["Retrive playback/capture device names"]
	init-playback: ["Initialize a playback device" index [integer!]]

	

	load:  ["Loads a file and returns sound's handle" sound [file!]]
	;copy:  [sound [handle!]]


	play:  [
		"Loads a file (if not already loaded) and starts playing it. Returns a sound handle."
		sound [file! handle!] "Source file or a ma-sound handle"
		/stream "Do not load the entire sound into memory"
		/loop   "Turn looping on"
		/volume vol [percent! decimal!]
		/fade   in  [integer! time!] "PCM frames or time"
	]
	pause: [
		"Pause sound playback"
		sound [handle!]
	]
	
	start: [
		"Start sound playback"
		sound [handle!]
		/loop "Turn looping on"
		/seek "Specify starting position" frames [integer! time!]
		/fade in [integer! time!] "PCM frames or time"
	]
	stop:  [
		"Stop sound playback"
		sound [handle!]
		/fade out [integer! time!] "PCM frames or time"
	]
	fade:  [
		"Fade sound volume"
		sound [handle!]
		frames [integer! time!] start [percent! decimal!] end [percent! decimal!]
	]
	seek:  [
		"Seek to specified position"
		sound [handle!]
		frames [integer! time!]
		/relative "Relative to the current sound position"
	]

	noise-node:  [
		"Create a noise node data source"
		type [integer!]
		amplitude [decimal!]
		/seed "Optional random seed"
		 val [integer!]
		/format "The sample format (default is 2 = signed 16bit float)"
		 frm [integer!] "Value betweem 1 - 5"
	]
	waveform-node: [
		type [integer!]
		amplitude [decimal!]
		frequency [decimal!]
		/format "The sample format (default is 2 = signed 16bit float)"
		 frm [integer!] "Value betweem 1 - 5"
	]
	
	;; Keep these (s|g)etters?
	volume:   ["Set the volume"  sound [handle!] volume [percent! decimal!]]
	volume?:  ["Get the volume"  sound [handle!]]
	pan:      ["Set the pan"     sound [handle!] pan [decimal!]]
	pan?:     ["Get the pan"     sound [handle!]]
	pitch:    ["Set the pitch"   sound [handle!] pitch [decimal!]]
	pitch?:   ["Get the pitch"   sound [handle!]]
	looping:  ["Set the looping" sound [handle!] value [logic!]]
	looping?: ["Get the looping" sound [handle!]]
	end?:     ["Return true if sound ended" sound [handle!]]
]

ext-values: {
;; Waveform types
type_sine:     0
type_square:   1
type_triangle: 2
type_sawtooth: 3

;; Sample data formats
format_u8:     1
format_s16:    2  ; Seems to be the most widely supported format.
format_s24:    3  ; Tightly packed. 3 bytes per sample.
format_s32:    4
format_f32:    5

white-noise: does [play noise-node 0 0.5]
}

;@@ TODO: use only one block of words and provide some automatic documentation for handles get/setters
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
	start
	stop
]
arg-words: [
	volume
	type
	amplitude
	frequency
	format
	frames
	time

	resources
]

;-------------------------------------- ----------------------------------------
reb-code: {REBOL [Title: {Rebol MiniAudio Extension} Type: module Version: 1.0.0]}
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
} ext-values
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



;; README documentation...
doc: clear ""
hdr: clear ""
arg: clear ""
cmd: desc: a: t: s: readme: r: none
parse commands [
	any [
		quote init-words: skip
		|
		set cmd: set-word! into [
			(clear hdr clear arg r: none)
			(append hdr ajoin [LF LF "#### `" cmd "`"])
			set desc: opt string!
			any [
				set a word!
				set t opt block!
				set s opt string!
				(
					unless r [append hdr ajoin [" `:" a "`"]]
					append arg ajoin [LF "* `" a "`"] 
					if t [append arg ajoin [" `" mold t "`"]]
					if s [append arg ajoin [" " s]]
				)
				|
				set r refinement!
				set s opt string!
				(
					append arg ajoin [LF "* `/" r "`"] 
					if s [append arg ajoin [" " s]]
				)
			]
			(
				append doc hdr
				append doc LF
				append doc any [desc ""]
				append doc arg
			)
		]
	]
]

try/except [
	readme: read/string %../README.md
	readme: clear find/tail readme "## Extension commands:"
	append readme ajoin [
		LF doc
		LF LF
		LF "## Other extension values:"
		LF "```rebol"
		trim/tail ext-values
		LF "```"
		LF
	]
	write %../README.md head readme
] :print


