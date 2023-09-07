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

;; reset the playback start time
device/frames: 0

sound/start: 44100
print ["Sound will start in" sound/start "frames (1s)..."]
audio/start sound ;; this sound will start after 44100 frames!
wait 1
print "Now there should be the sound!"
wait 1

;; play a looping sound...
print "Now start a loop with fast fade-in (0.5 seconds)..."
probe drums: audio/play/loop/fade %assets/drumloop.wav 0:0:0.5

;; list resources linked with the playback device...
print ["Available sounds:" mold device/resources]

;; work in Rebol as usually
wait 0:0:1

;; for synchronization purposes the global playback time may be accessed and modified
print ["Device global time in PCM frames:" device/frames "as time:" device/time]
print  "Modifying global playback time to 44100 PCM frames (1s)"
device/frames: 44100
print ["Device global time in PCM frames:" device/frames "as time:" device/time]
print  "Modifying global playback time to 0:0:2"
device/time: 0:0:2
print ["Device global time in PCM frames:" device/frames "as time:" device/time]

;; stop the music with a fade 5 seconds...
print "Now stop the loop in 5 seconds fade-out..."
audio/stop/fade :drums 0:0:5

;; wait for the sound to fade out...
wait 5

;; play already loaded sound...
sound/pan:  1.0
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

