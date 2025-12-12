REBOL [
	title:  "Rebol/MiniAudio module builder"
	type:    module
	date:    12-Dec-2025
	home:    https://github.com/Oldes/Rebol-MiniAudio
	version: 0.11.23.0
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	init-words:    [args [block!] type [block!]] ;; used internaly only!
	;test:  ["..."]

	version: ["Native MiniAudio version"]

	get-devices:   ["Retrive playback/capture device names"]
	init-playback: [
		"Initialize a playback device"
		index [integer!]
		/pause    "Don't start it automatically"
		/channels "The number of channels to use for playback" 
		 number [integer!] "When set to 0 the device's native channel count will be used"
		/period            "Hint for making up the device's entire buffer"
		 size   [integer!] "The desired size of a period in milliseconds"
		/callback "On-data callback (two args.. buffer frames, and engine total frames)"
		 context [object!] "The function's context"
		 word    [word!]   "The function's name"
	]

	load:  [
		"Loads a file and returns sound's handle"
		sound [file!]
		/group "Group of sounds which have their own effect processing and volume control"
		 node [handle!] "ma-group handle"
	]
	;copy:  [sound [handle!]]


	play:  [
		"Loads a file (if not already loaded) and starts playing it. Returns a sound handle."
		sound [file! handle!] "Source file or a ma-sound handle"
		/stream "Do not load the entire sound into memory"
		/loop   "Turn looping on"
		/volume vol [percent! decimal!]
		/fade   in  [integer! time!] "PCM frames or time"
		/group  "Group of sounds which have their own effect processing and volume control"
		 node [handle!] "ma-group handle"
	]
	pause: [
		"Pause sound playback"
		sound [handle!]
	]
	
	start: [
		"Start sound or device playback"
		handle  [handle!] "ma-sound or ma-engine handle"
		/loop   "Turn looping on (only for sounds)"
		/seek   "Starting position"
		 frames [integer! time!] "PCM frames or time"
		/fade   "Fade in the sound"
		 in     [integer! time!] "PCM frames or time"
		/at     "Absolute engine time when the sound should be started"
		 time   [integer! time!] "PCM frames or time"
	]
	stop:  [
		"Stop sound or device playback"
		handle [handle!] "ma-sound or ma-engine handle"
		/fade out [integer! time!] "PCM frames or time (only for sounds)"
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

	make-noise-node:  [
		"Creates a noise node for generating random noise"
		type      [integer!] "The type of noise to generate (0 - 2)"
		amplitude [decimal!] "The peak amplitude of the noise"
		/seed                "Optional random seed"
		 val [integer!]
		/format              "The sample format (default is 2 = signed 16bit float)"
		 frm [integer!]      "Value betweem 1 - 5"
	]
	make-waveform-node: [
		"Creates a sound waveform node"
		type      [integer!] "The type of waveform to generate (0 - 3)"
		amplitude [decimal!] "The peak amplitude of the waveform"
		frequency [decimal!] "The frequency of the waveform in Hertz (Hz)"
		/format "The sample format (default is 2 = signed 16bit float)"
		 frm [integer!] "Value betweem 1 - 5"
	]
	make-delay-node: [
		"Creates a delay (echo) sound node"
		delay [decimal! integer! time!] "The time before the echo is heard. Seconds, PCM frames or time."
		decay [decimal! percent!] "Feedback decay (0.0 - 1.0). Affects how quickly or gradually the echoes fade away. 0 means no feedback."
		/dry "The mix level of the dry (original) sound"
		 d [decimal! percent!]
		/wet "The mix level of the wet (delayed) sound"
		 w [decimal! percent!]
	]
	make-group-node: [
		"Creates a sound group node"
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

handles: make map! [
	ma-sound: [
		"MiniAudio sound object"
		;NAME          GET       SET                 DESCRIPTION
		volume         decimal!  [integer! decimal! percent!] "Sound volume"
		pan            decimal!   decimal!           "Stereo panning (from -1.0 to 1.0)"
		pitch          decimal!   decimal!           "Sound pitch"
		position       pair!      pair!              "Sound position (x and y for now) relative to the listener"
		cursor         integer!  [integer! time!]    "Sound playback position in PCM frames"
		time           time!      time!              "Sound playback position as time"
		duration       time!      none               "Sound duration in time"
		frames         integer!   none               "Sound length in PCM frames"
		sample-rate    integer!   none               "Number of samples per second"
		spatialize     logic!     logic!             "3D spatialization state"
		is-looping     logic!     logic!             "Whether sound is looping"
		is-playing     logic!     logic!             "Whether sound is playing"
		at-end         logic!     none               "Whether sound is at end"
		start          integer!  [integer! time!]    "Absolute timer when the sound should be started (frames or time)"
		stop           integer!  [integer! time!]    "Absolute timer when the sound should be stopped (frames or time)"
		x              decimal!  [integer! decimal!] "Sound X position"
		y              decimal!  [integer! decimal!] "Sound Y position"
		z              decimal!  [integer! decimal!] "Sound Z position"
		source        [file! handle!] none           "Sound source as a loaded file or data source node"
		outputs        integer!   none               "Number of output buses"
		output         handle!   [handle! none!]     "Output bus node"
	]
	ma-group: [
		"MiniAudio sound group"
		;NAME          GET       SET                 DESCRIPTION
		volume         decimal!  [integer! decimal! percent!] "Sound volume"
		pan            decimal!   decimal!           "Stereo panning (from -1.0 to 1.0)"
		pitch          decimal!   decimal!           "Sound group pitch"
		position       pair!      pair!              "Sound group position (x and y for now) relative to the listener"
		time           time!      time!              "Sound group playback position as time"
		duration       time!      none               "Sound group duration in time"
		sample-rate    integer!   none               "Number of samples per second"
		spatialize     logic!     logic!             "3D spatialization state"
		is-playing     logic!     logic!             "Whether sound is playing"
		start          integer!  [integer! time!]    "Absolute timer when the sound should be started (frames or time)"
		stop           integer!  [integer! time!]    "Absolute timer when the sound should be stopped (frames or time)"
		x              decimal!  [integer! decimal!] "Sound group X position"
		y              decimal!  [integer! decimal!] "Sound group Y position"
		z              decimal!  [integer! decimal!] "Sound group Z position"
		outputs        integer!   none               "Number of output buses"
		output         handle!   [handle! none!]     "Output bus node"
		resources      block!     none               "Used group resources (sounds, nodes..)"
	]
	ma-engine: [
		"MiniAudio device engine"
		volume         decimal!  [integer! decimal! percent!] "Global volume"
		cursor         integer!  [integer! time!]    "Engine playback position in PCM frames"
		time           time!      time!              "Engine playback position as time"
		resources      block!     none               "Used engine resources (sounds, nodes..)"
		channels       integer!   none               "Number of output channels"
		sample-rate    integer!   none               "Ouput device sample rate per second"
		gain-db        decimal!  [integer! decimal!] "The amplification factor in decibels"
	]
	ma-noise: [
		"MiniAudio noise generator"
		amplitude      decimal!   decimal!           "Maximum value of the noise signal"
		format         word!      none               "f32, s16, s24, s32, u8"
		type           word!      none               "white, pink or brownian"
	]
	ma-waweform: [
		"MiniAudio sine, square, triangle and sawtooth waveforms generator"
		amplitude      decimal!   decimal!           "Signal amplitude"
		frequency      decimal!   decimal!           "Signal frequency in hertzs"
		format         word!      none               "f32, s16, s24, s32, u8"
		type           word!      none               "sine, square, triangle or sawtooth"
	]
	ma-delay: [
		"MiniAudio delay node"
		delay          integer!  none                "PCM frames"
		decay          decimal!  [decimal! percent!] "Value between 0.0 and 1.0"
		dry            decimal!  [decimal! percent!] "The mix level of the dry (original) sound"
		wet            decimal!  [decimal! percent!] "The mix level of the wet (delayed) sound"
	]
]

arg-words:   copy []
handles-doc: copy {}

foreach [name spec] handles [
	append handles-doc ajoin [
		LF LF "#### __" uppercase form name "__ - " spec/1 LF
		LF "```rebol"
		LF ";Refinement       Gets                Sets                          Description"
	]
	foreach [name gets sets desc] next spec [
		append handles-doc rejoin [
			LF
			#"/" pad name 17
			pad mold gets 20
			pad mold sets 30
			#"^"" desc #"^""
		]
		append arg-words name
	]
	append handles-doc "^/```"
]
;print handles-doc
arg-words: unique arg-words

type-words: [
	;@@ Order is important!
	;- noise types
	white
	pink
	brownian
	;- waveform types
	sine
	square
	triangle
	sawtooth
	;- format types
	f32
	s16
	s24
	s32
	u8
]


;-------------------------------------- ----------------------------------------
reb-code: ajoin [
	{REBOL [Title: "Rebol MiniAudio Extension"}
	{ Name: miniaudio Type: module}
	{ Version: 0.11.23}
	{ Needs: 3.14.1}
	{ Author: Oldes}
	{ Date: } now/utc
	{ License: MIT}
	{ Url: https://github.com/Oldes/Rebol-MiniAudio}
	#"]"
]

logo: next {
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
}

enu-commands:  "" ;; command name enumerations
cmd-declares:  "" ;; command function declarations
cmd-dispatch:  "" ;; command functionm dispatcher

ma-arg-words: "enum ma_arg_words {W_ARG_0"
ma-type-words: "enum ma_type_words {W_TYPE_0"

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

foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	replace/all word #"?" #"Q"
	append ma-arg-words ajoin [",^/^-W_ARG_" word]
]

foreach word type-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-type-words ajoin [",^/^-W_TYPE_" word]
]

append ma-arg-words "^/};"
append ma-type-words "^/};"
append reb-code ajoin [{
init-words } mold/flat arg-words mold/flat type-words {
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
header: {$logo
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
extern u32 rebol_version;

enum ext_commands {$enu-commands
};

$cmd-declares

$ma-arg-words
$ma-type-words

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
ctable: {$logo
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
		LF "## Used handles and its getters / setters" 
		handles-doc
		LF LF
		LF "## Other extension values:"
		LF "```rebol"
		trim/tail ext-values
		LF "```"
		LF
	]
	write %../README.md head readme
] :print


