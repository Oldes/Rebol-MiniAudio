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
;; (for test purpose, not starting the playback automatically)
probe device: audio/init-playback/pause 1

;; load a sound for later use...
probe sound: audio/load %assets/zblunk_02.wav

;; changing the initial playback start time offset in PCM frames
device/frames: 44100
;; delay the start of the sound in frames
sound/start: 44100
print ["Sound will start in" sound/start - device/frames "frames (1s)..."]
audio/start device ;; finally start the paused playback device
audio/start sound  ;; this sound will start after 44100 frames!
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

;; fade the music with a fade 5 seconds...
print "Now fade out the loop in 5 seconds..."
audio/fade :drums 0:0:5 100% 5%

;; wait for the sound to fade out...
wait 5

;; play already loaded sound...
sound/pan:  1.0
audio/play :sound
wait 1

sound/pan: -1.0
audio/play :device/resources/1
wait 1

;- Using delay node...
delay: audio/delay-node 0.25 50% ;; 0.25s delay with 50% decay
print ["Prepared delay:" delay/delay "frames and" delay/decay "decay."]
;; set the music output bus to the delay node...
drums/output: :delay
;; fade in the music
audio/fade :drums 0:0:2.5 5% 100%
wait 5
;; stop the music with a fade 5 seconds...
print "Now stop the loop in 5 seconds fade-out..."
;; modify the decay...
delay/decay: 85%
print ["Decay value changed to:" delay/decay]
audio/stop/fade :drums 0:0:5
wait 4

;- Using a data-source node...
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

