Rebol [
	Title:   "Rebol/MiniAudio extension test"
	Needs:   3.22.5
	Purpose: {
		Exercises the extension's command surface and - importantly - every
		path accessor of every registered handle type, because those are
		resolved through generated word enums where a wrong index is silent.

		Usage:
			r3 test.r3            ;; full run, plays audio (~35s)
			r3 test.r3 --quick    ;; skips the audible/timed sections
			r3 test.r3 --null     ;; allow falling back to the null device

		Exits with 1 if any assertion failed, so it can gate CI.
	}
]

;;=============================================================================
;; Minimal self-contained harness
;;=============================================================================

test-count: 0
fail-count: 0
skip-count: 0
failed: copy []

group: func ["Starts a named group of tests" name [string!]][
	print ajoin [lf as-yellow "== " as-yellow name]
]

;; NOTE: plain `func` with explicit locals - `function` would collect the
;; counter set-words as locals and the totals would never move.
--test--: func [
	"Passes when the code block evaluates to true"
	name [string!]
	code [block!]
	/local result
][
	test-count: test-count + 1
	result: try code
	case [
		error? :result [
			fail-count: fail-count + 1
			append failed name
			print [as-red "[FAIL]" name]
			print [as-red "      " mold/flat :result]
		]
		:result = true [
			print [as-green "[ ok ]" name]
		]
		true [
			fail-count: fail-count + 1
			append failed name
			print [as-red "[FAIL]" name "=>" mold/flat/part :result 60]
		]
	]
	:result
]

--skip--: func ["Counts a test which cannot run here" name [string!]][
	skip-count: skip-count + 1
	print [as-purple "[skip]" name]
]

;; Reads `handle/field` for a dynamically supplied field name.
;; Paths evaluate parens before dispatching to the handle's get_path, so this
;; goes down exactly the same code path as a literal `sound/volume` would.
get-field: func [hndl [handle!] name [word!]][ hndl/(name) ]

--reads--: func [
	"Checks that every listed field is readable and has an expected type"
	label  [string!]
	hndl   [any-type!]
	fields [block!] "name [types] pairs"
	/local name types
][
	unless handle? :hndl [
		--skip-- ajoin [label " accessors (no handle)"]
		exit
	]
	foreach [name types] fields [
		;; `find types type? ...` would match by datatype instead of by value,
		;; hence type?/word against a plain block of type words.
		--test-- ajoin [label "/" name]
			compose/deep [
				to logic! find [(types)] type?/word get-field (hndl) (to lit-word! name)
			]
	]
]

--rejects--: func [
	"Checks that reading or writing an unsupported field is refused"
	label [string!]
	code  [block!]
][
	--test-- ajoin [label " is refused"] compose/only [error? try (code)]
]

about?: func ["Compares two numbers with a tolerance" a b tol][tol >= abs a - b]

summary: does [
	print ajoin [lf as-yellow "-----------------------------------------------------------------------------"]
	print [
		"tests:" test-count
		as-green ajoin ["passed: " test-count - fail-count - skip-count]
		either fail-count > 0 [as-red ajoin ["failed: " fail-count]][ajoin ["failed: " fail-count]]
		as-purple ajoin ["skipped: " skip-count]
	]
	if fail-count > 0 [
		print as-red ajoin ["^/Failed tests:^/  " mold/flat failed]
	]
	print ""
	quit/return either fail-count > 0 [1][0]
]

;;=============================================================================
;; Options
;;=============================================================================

args: system/options/args
quick?:      did find args "--quick" ;; skip the long, audible parts
allow-null?: did find args "--null"  ;; accept the null (dummy) device

;; Playback devices which are present but useless for a test run.
ignored-devices: [
	"Speakers (Steam Streaming Speakers)"
]

;;=============================================================================
group "Module surface"
;;=============================================================================

unless value? 'audio [audio: import 'miniaudio]

;; every handle the suite may create, so a failed creation cannot abort it
noise: sound: utf-sound: wave: delay: bus: drums: tone: device: none

--test-- "module imported" [module? audio]
--test-- "native version is a tuple" [tuple? audio/version]

foreach name [
	version get-devices init-playback load play pause start stop fade seek
	make-noise-node make-waveform-node make-delay-node make-group-node
	volume volume? pan pan? pitch pitch? looping looping? end?
][
	--test-- ajoin ["command " name] compose [command? get in audio (to lit-word! name)]
]

;; `init-words` is generated, called once and then protected/hidden -
;; it must not be reachable from the outside.
--test-- "init-words is hidden" [none? in audio 'init-words]

--test-- "mezzanine: waveform type values" [
	all [
		audio/type_sine     = 0
		audio/type_square   = 1
		audio/type_triangle = 2
		audio/type_sawtooth = 3
	]
]
--test-- "mezzanine: sample format values" [
	all [
		audio/format_u8  = 1
		audio/format_s16 = 2
		audio/format_s24 = 3
		audio/format_s32 = 4
		audio/format_f32 = 5
	]
]

;;=============================================================================
group "Device enumeration"
;;=============================================================================

devices: none
--test-- "get-devices returns loadable data" [
	devices: transcode audio/get-devices
	all [block? devices  block? select devices 'playback]
]

playback-index: none
if devices [
	repeat i length? devices/playback [
		name: devices/playback/:i
		if find ignored-devices name [continue]
		if all [name = "Null Audio Device" not allow-null?] [continue]
		playback-index: i
		break
	]
]

either playback-index [
	print ["Using playback device" playback-index "-" mold devices/playback/:playback-index]
][
	;; GitHub Actions on Windows and macOS have no audio device at all.
	print as-purple "^/No usable playback device - only device-independent tests will run."
	print as-purple "Pass --null to allow the dummy device, if the null backend is built in."
]

--test-- "invalid device index is refused" [error? try [audio/init-playback 0]]
--test-- "out of range device index is refused" [error? try [audio/init-playback 9999]]

;;=============================================================================
group "Noise node (needs no engine)"
;;=============================================================================

--test-- "make-noise-node" [handle? noise: audio/make-noise-node 0 0.25] ;; 0 = white

if handle? :noise [
	--reads-- "ma-noise" noise [
		amplitude [decimal!]
		type      [word!]
		format    [word!]
	]
	--test-- "noise/type is white by default" [noise/type = 'white]
	--test-- "noise/format defaults to s16" [noise/format = 's16]
	--test-- "noise/amplitude round trip" [
		noise/amplitude: 0.5
		noise/amplitude = 0.5
	]
	foreach [word value] reduce [
		'u8  audio/format_u8
		's16 audio/format_s16
		's24 audio/format_s24
		's32 audio/format_s32
		'f32 audio/format_f32
	][
		--test-- ajoin ["noise format " word]
			compose [
				n: audio/make-noise-node/format 0 0.1 (value)
				(to lit-word! word) = n/format
			]
	]
	--rejects-- "ma-noise/nonsense" [noise/nonsense]
	--rejects-- "ma-noise/type write" [noise/type: 'pink]
]

unless playback-index [summary]

;;=============================================================================
group "Playback device"
;;=============================================================================

;; Keep the reference to the device handle, else it will be released by GC!
;; Not starting the playback automatically, so the start offset can be tested.
--test-- "init-playback returns a handle" [
	handle? device: audio/init-playback/pause playback-index
]
unless handle? :device [
	print as-red "^/Could not open the playback device - stopping here."
	summary
]

--reads-- "ma-engine" device [
	volume      [decimal!]
	frames      [integer!]
	time        [time!]
	channels    [integer!]
	sample-rate [integer!]
	gain-db     [decimal!]
	resources   [block!]
	listener    [handle!]
	listeners   [block!]
]

listener: device/listener
--test-- "engine reports its listeners" [
	all [
		handle? :listener
		block? device/listeners
		1 <= length? device/listeners
		handle? first device/listeners
	]
]
--reads-- "ma-listener" listener [
	index    [integer!]
	enabled  [logic!]
	position [pair!]
	x        [decimal!]
	y        [decimal!]
	z        [decimal!]
]
--test-- "the first listener has index 0" [listener/index = 0]
--test-- "listener is enabled by default" [listener/enabled]
--test-- "listener position round trip" [
	listener/x: 1.0 listener/y: 2.0 listener/z: 3.0
	all [
		about? listener/x 1.0 0.001
		about? listener/y 2.0 0.001
		about? listener/z 3.0 0.001
		about? listener/position/x 1.0 0.001
		about? listener/position/y 2.0 0.001
	]
]
--test-- "listener position accepts a pair" [
	listener/position: 4x5
	all [about? listener/x 4.0 0.001  about? listener/y 5.0 0.001]
]
--test-- "setting one axis keeps the others" [
	listener/position: 0x0
	listener/z: 7.0
	listener/y: 8.0
	all [about? listener/z 7.0 0.001  about? listener/y 8.0 0.001]
]
--test-- "listener can be disabled" [
	listener/enabled: false
	all [not listener/enabled  true = (listener/enabled: true)]
]
--test-- "two handles address the same listener" [
	other: device/listener
	other/x: 9.0
	about? listener/x 9.0 0.001
]
--rejects-- "ma-listener/enabled with a bad type" [listener/enabled: 1]
--rejects-- "ma-listener/index write" [listener/index: 1]
--rejects-- "ma-engine/listener write" [device/listener: none]

;; Back to the origin - a listener left metres away from the sounds would
;; attenuate the audible playback section into near silence.
listener/position: 0x0
listener/z: 0.0

--test-- "engine has a sane sample rate" [device/sample-rate > 0]
--test-- "engine has at least one channel" [device/channels > 0]
--test-- "engine volume round trip" [
	device/volume: 0.5
	0.5 = round/to device/volume 0.01
]
--test-- "engine frames round trip" [
	device/frames: 44100
	about? device/frames 44100 1000
]
--test-- "engine time follows frames" [
	device/time: 0:0:2
	about? device/frames (2 * device/sample-rate) 1000
]
--rejects-- "ma-engine/channels write" [device/channels: 2]
--rejects-- "ma-engine/nonsense" [device/nonsense]

;;=============================================================================
group "Loading sounds"
;;=============================================================================

--test-- "load a wav file" [handle? sound: audio/load %assets/zblunk_02.wav]
;; The local path conversion is UTF-8 on POSIX and wide on Windows -
;; a non-ASCII file name is the interesting case.
--test-- "load a file with a non-ASCII name" [
	handle? utf-sound: audio/load %assets/žbluňk_02.wav
]
--test-- "sound/source keeps the full path" [
	all [
		string? sound/source
		%zblunk_02.wav = to file! last split to file! sound/source #"/"
	]
]
--test-- "loading a missing file is refused" [error? try [audio/load %assets/no-such-file.wav]]
--test-- "device lists the loaded sounds" [
	all [
		block? device/resources
		2 <= length? device/resources
	]
]

--reads-- "ma-sound" sound [
	volume      [decimal!]
	fade-volume [decimal!]
	gain-db     [decimal!]
	pan         [decimal!]
	pitch       [decimal!]
	position    [pair!]
	cursor      [integer!]
	time        [time!]
	duration    [time!]
	frames      [integer!]
	sample-rate [integer!]
	spatialize  [logic!]
	is-looping  [logic!]
	is-playing  [logic!]
	at-end      [logic!]
	start       [integer!]
	stop        [integer!]
	x           [decimal!]
	y           [decimal!]
	z           [decimal!]
	rolloff        [decimal!]
	min-distance   [decimal!]
	max-distance   [decimal!]
	min-gain       [decimal!]
	max-gain       [decimal!]
	doppler-factor [decimal!]
	attenuation    [word! none!]
	positioning    [word!]
	outputs     [integer!]
	source      [string! handle! none!]
]
--test-- "sound attenuation defaults" [
	all [
		sound/attenuation = 'inverse
		sound/positioning = 'absolute
		sound/rolloff > 0
	]
]

foreach [field value] [
	rolloff        2.0
	min-distance   0.5
	max-distance   100.0
	min-gain       0.1
	max-gain       0.9
	doppler-factor 0.0
][
	--test-- ajoin ["sound/" field " round trip"] [
		sound/:field: value
		about? sound/:field value 0.001
	]
]
foreach word [inverse linear exponential][
	--test-- ajoin ["sound attenuation " word] [
		sound/attenuation: word
		word = sound/attenuation
	]
]

--test-- "sound attenuation can be disabled" [
	sound/attenuation: none
	none? sound/attenuation
]
--test-- "sound positioning round trip" [
	sound/positioning: 'relative
	all ['relative = sound/positioning  'absolute = (sound/positioning: 'absolute)]
]
--test-- "an unknown attenuation model is refused" [
	error? try [sound/attenuation: 'quadratic]
]
--test-- "attenuation is refused a non-word" [error? try [sound/attenuation: 2]]
--test-- "a negative rolloff is refused" [error? try [sound/rolloff: -1.0]]

--test-- "sound has a non-zero duration" [sound/duration > 0:0:0]
--test-- "sound time round trip" [
	sound/time: 0:0:0.5
	all [
		about? sound/cursor (sound/sample-rate / 2) 100
		about? to decimal! sound/time 0.5 0.02
	]
]
--test-- "sound time is refused a non-time value" [error? try [sound/time: 100]]
--test-- "sound frames match the duration" [sound/frames > 0]
--test-- "fresh sound is not playing" [not sound/is-playing]
--test-- "fresh sound is not at end" [not sound/at-end]
--test-- "sound volume round trip" [sound/volume: 0.25  0.25 = round/to sound/volume 0.01]
--test-- "full volume is 0 dB" [
	sound/volume: 1.0
	about? sound/gain-db 0.0 0.001
]
--test-- "sound gain-db round trip" [
	sound/gain-db: -6.0
	all [
		about? sound/gain-db -6.0 0.001
		;; -6 dB is roughly half the amplitude
		about? sound/volume 0.5 0.01
	]
]
--test-- "sound gain-db accepts an integer" [
	sound/gain-db: 0
	about? sound/volume 1.0 0.001
]
;; silence has no finite decibel value - the engine accessor does the same
--test-- "a silent sound reports -inf dB" [
	sound/volume: 0.0
	sound/gain-db < -100.0
]
sound/volume: 1.0
--test-- "sound pan round trip"    [sound/pan: -1.0     -1.0 = round/to sound/pan 0.01]
--test-- "sound pitch round trip"  [sound/pitch: 1.5     1.5 = round/to sound/pitch 0.01]
--test-- "sound looping round trip" [
	sound/is-looping: true
	all [sound/is-looping  audio/looping? sound]
]
--test-- "looping command matches the accessor" [
	audio/looping sound false
	all [not sound/is-looping  not audio/looping? sound]
]
--test-- "sound position round trip" [
	sound/x: 1.0 sound/y: 2.0 sound/z: 3.0
	all [
		1.0 = round/to sound/x 0.01
		2.0 = round/to sound/y 0.01
		3.0 = round/to sound/z 0.01
		1.0 = round/to sound/position/x 0.01
		2.0 = round/to sound/position/y 0.01
	]
]
--test-- "getter commands match the accessors" [
	sound/volume: 0.4
	sound/pan:    0.2
	sound/pitch:  1.1
	all [
		(round/to audio/volume? sound 0.01) = (round/to sound/volume 0.01)
		(round/to audio/pan?    sound 0.01) = (round/to sound/pan    0.01)
		(round/to audio/pitch?  sound 0.01) = (round/to sound/pitch  0.01)
	]
]
--rejects-- "ma-sound/nonsense" [sound/nonsense]
--rejects-- "ma-sound/duration write" [sound/duration: 0:0:1]
--rejects-- "ma-sound/fade-volume write" [sound/fade-volume: 100]
--rejects-- "ma-sound/volume with a bad type" [sound/volume: "loud"]

;;=============================================================================
group "Waveform, delay and group nodes"
;;=============================================================================

--test-- "make-waveform-node" [
	handle? wave: audio/make-waveform-node audio/type_sine 0.5 440.0
]
--reads-- "ma-waveform" wave [
	amplitude [decimal!]
	frequency [decimal!]
	type      [word!]
	format    [word!]
]
foreach [word value] reduce [
	'sine     audio/type_sine
	'square   audio/type_square
	'triangle audio/type_triangle
	'sawtooth audio/type_sawtooth
][
	--test-- ajoin ["waveform type " word]
		compose [
			w: audio/make-waveform-node (value) 0.1 440.0
			(to lit-word! word) = w/type
		]
]
foreach [word value] reduce [
	'u8  audio/format_u8
	's16 audio/format_s16
	's24 audio/format_s24
	's32 audio/format_s32
	'f32 audio/format_f32
][
	--test-- ajoin ["waveform format " word]
		compose [
			w: audio/make-waveform-node/format 0 0.1 440.0 (value)
			(to lit-word! word) = w/format
		]
]
--test-- "waveform frequency round trip" [wave/frequency: 220.0  220.0 = wave/frequency]
--test-- "waveform amplitude round trip" [wave/amplitude: 0.3    0.3   = wave/amplitude]

--test-- "make-delay-node" [handle? delay: audio/make-delay-node 0.25 50%]
--reads-- "ma-delay" delay [
	delay [integer!]
	decay [decimal!]
	dry   [decimal!]
	wet   [decimal!]
]
--test-- "delay length matches the sample rate" [
	delay/delay = to integer! 0.25 * device/sample-rate
]
--test-- "delay decay round trip" [delay/decay: 85%  0.85 = round/to delay/decay 0.01]
--test-- "delay dry/wet round trip" [
	delay/dry: 30% delay/wet: 70%
	all [0.3 = round/to delay/dry 0.01  0.7 = round/to delay/wet 0.01]
]
--rejects-- "ma-delay/delay write" [delay/delay: 100]

--test-- "make-group-node" [handle? bus: audio/make-group-node]
--reads-- "ma-group" bus [
	volume      [decimal!]
	fade-volume [decimal!]
	gain-db     [decimal!]
	pan         [decimal!]
	pitch       [decimal!]
	position    [pair!]
	time        [time!]
	sample-rate [integer!]
	spatialize  [logic!]
	is-playing  [logic!]
	start       [integer!]
	stop        [integer!]
	x           [decimal!]
	y           [decimal!]
	z           [decimal!]
	outputs     [integer!]
	resources   [block!]
]
--test-- "a sound can be loaded into a group" [
	all [
		handle? grouped: audio/load/group %assets/drumloop.wav bus
		1 = length? bus/resources
	]
]
--test-- "group gain-db round trip" [
	bus/gain-db: -6.0
	all [about? bus/gain-db -6.0 0.001  about? bus/volume 0.5 0.01]
]
bus/gain-db: 0.0

--test-- "a group can be started and stopped" [
	all [
		handle? audio/start bus
		bus/is-playing
		handle? audio/stop bus
		not bus/is-playing
	]
]
--test-- "a group can be stopped with a fade" [
	audio/start bus
	audio/stop/fade bus 0:0:0.2
	;; A faded stop is scheduled against the ENGINE clock, which does not
	;; advance while the device is paused - and it still is here, so
	;; waiting would achieve nothing. Move the clock past the stop time
	;; instead, which also makes the test instant and deterministic.
	device/frames: device/frames + device/sample-rate
	not bus/is-playing
]
--test-- "a group refuses /loop and /seek" [
	all [
		error? try [audio/start/loop bus]
		error? try [audio/start/seek bus 100]
	]
]
--test-- "sound output can be routed to a delay node" [
	sound/output: delay
	;; `output` is write only - `outputs` is the readable bus count
	sound/outputs >= 1
]
--test-- "sound output can be detached" [
	sound/output: none
	true
]
--rejects-- "ma-sound/output read" [sound/output]
--rejects-- "routing to a wrong handle type" [sound/output: bus]

;;=============================================================================
group "Playback"
;;=============================================================================

either quick? [
	--skip-- "timed playback (--quick)"
][
	;; The device was created paused, so the whole start offset is testable.
	--test-- "a delayed start can be scheduled" [
		device/frames: 0
		sound: audio/load %assets/žbluňk_02.wav
		sound/start: 44100 ;; one second from now
		sound/start >= 44100
	]

	--test-- "the device and the sound can be started" [
		audio/start device   ;; finally start the paused playback device
		audio/start sound    ;; ...this sound starts after 44100 frames
		true
	]

	--test-- "device advances its clock" [
		frames: device/frames
		wait 0.5
		device/frames > frames
	]

	wait 1.2
	--test-- "delayed sound has started" [sound/cursor > 0]
	wait 1

	print "Now a looping sound with a 0.5s fade-in..."
	--test-- "play with /loop and /fade" [
		handle? drums: audio/play/loop/fade %assets/drumloop.wav 0:0:0.5
	]

	either handle? :drums [
		--test-- "the loop reports itself as playing" [drums/is-playing]
		--test-- "the loop is looping" [drums/is-looping]

		wait 1
		--test-- "playing sound advances its cursor" [
			cursor: drums/cursor
			wait 0.5
			drums/cursor > cursor
		]

		--test-- "seek moves the cursor" [
			audio/seek drums 0
			drums/cursor < (drums/sample-rate / 2)
		]
		--test-- "relative seek moves the cursor forward" [
			cursor: drums/cursor
			audio/seek/relative drums 22050
			drums/cursor > cursor
		]

		--test-- "pause stops the playback" [
			audio/pause drums
			wait 0.1
			not drums/is-playing
		]
		--test-- "start resumes the playback" [
			audio/start/loop drums
			wait 0.1
			all [drums/is-playing  drums/is-looping]
		]
		;; `start` passes its /loop refinement straight to
		;; ma_sound_set_looping, so a plain `start` turns looping OFF and
		;; rewinds to frame 0 - it restarts rather than resumes.
		--test-- "start without /loop clears the looping flag" [
			audio/start drums
			not drums/is-looping
		]

		print "Fading the loop out over 2 seconds..."
		--test-- "a fade lowers fade-volume, not volume" [
			;; keep it looping, or the file would end mid-fade
			audio/start/loop drums
			audio/fade drums 0:0:2 100% 5%
			wait 2.2
			all [
				drums/is-playing
				drums/fade-volume < 0.2
				drums/volume > 0.8
			]
		]

		print "Stopping the loop with a 1 second fade-out..."
		--test-- "stopped sound is no longer playing" [
			audio/stop/fade drums 0:0:1
			wait 1.5
			not drums/is-playing
		]
	][
		--skip-- "loop playback (the sound did not load)"
	]

	;; A data source node played as a sound, modulated while it plays.
	--test-- "a waveform node can be played" [
		wave: audio/make-waveform-node audio/type_sine 0.5 440.0
		handle? tone: audio/play/fade wave 0:0:1
	]
	if handle? :tone [
		a: 0 b: pi
		loop 100 [
			a: a + 0.01
			b: b + 0.006
			wave/frequency: 440.0 + (220.0 * (sin a) * cos b)
			wait 0.01
		]
		--test-- "the modulated frequency was applied" [wave/frequency <> 440.0]
		audio/stop/fade tone 0:0:1
		wait 1.2
	]
]

;;=============================================================================
group "Releasing handles"
;;=============================================================================

--test-- "a sound handle can be released" [
	tmp: audio/load %assets/zblunk_02.wav
	release tmp
	true
]
--test-- "a released sound drops its source reference" [
	tmp: audio/load %assets/zblunk_02.wav
	string? tmp/source
	release tmp
	;; MASound_free clears hob->series, so `source` is the one accessor
	;; which is safe to read afterwards - see the notes on IS_USED_HOB.
	none? tmp/source
]
--test-- "releasing the device releases its resources" [
	release device
	true
]

summary
