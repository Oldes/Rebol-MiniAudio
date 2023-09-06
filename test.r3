Rebol [
	title: "Rebol/MiniAudio extension test"
]

print ["Running test on Rebol build:" mold to-block system/build]

;; make sure that we load a fresh extension
try [system/modules/miniaudio: none]

audio: import miniaudio

;; list all available devices:
print audio/get-devices

;; init a playback device (first available)...
;; keep the reference to the device handle, else it will be released by GC!
probe device: audio/init-playback 1

;; load a sound for later use...
probe sound: audio/load %assets/zblunk_02.wav

;; play a looping sound...
probe drums: audio/play/loop %assets/drumloop.wav

;; list resources linked with the playback device...
print ["Available sounds:" mold device/resources]

;; work in Rebol as usually
wait 0:0:1

;; stop the music with a fade 5 seconds...
audio/stop/fade :drums 0:0:5

;; wait for the sound to fade out...
wait 5

;; play already loaded sound...
audio/play :sound
wait 1

sound/pan: -1.0
audio/play :device/resources/1
wait 1


;; Using a data-source node...
a: 0
b: PI
with audio [
	probe wave: waveform-node type_sine 0.5 440.0
	print ["amplitude:" wave/amplitude "frequency:" wave/frequency]
	probe sound: play/fade wave 0:0:3 ;; start playing with a fade..
	;; modify the wave's parameters while playing...
	loop 500 [
		a: a + 0.01
		b: b + 0.006
		wave/frequency: 440.0 + (220.0 * ((sin a) * cos b))
		wait 0.01
	]
	stop/fade sound 0:0:2

	loop 400 [
		a: a + 0.02
		b: b + 0.003
		wave/frequency: 440.0 + (220.0 * ((sin a) * cos b))
		wait 0.005
	]
]

;; release the playback device (and all resources)
release device

print "done"

