Rebol [
    title: "Rebol/MiniAudio extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]

;; make sure that we load a fresh extension
try [system/modules/miniaudio: none]

;; import the extension
miniaudio: import 'miniaudio

;; print content of the module...
? miniaudio

miniaudio/test
wait 2
quit

a: 0
b: PI
with miniaudio [
    probe src: waveform-node type_sawtooth 1.0 440.0
    print ["amplitude:" src/amplitude "frequency:" src/frequency]
    probe snd: play src
    probe src/type quit
    print ["amplitude:" src/amplitude "frequency:" src/frequency]
    loop 1000 [
        a: a + 0.01
        b: b + 0.006
       probe  src/frequency: 440.0 + (220.0 * ((sin a) * cos b))
        wait 0.01                                                                                            
    ]
    stop/fade snd 0:0:2
    wait 2
    quit


    probe n: noise-node 2 1.0
    snd: play n
    n/amplitude: 2.0
    probe n/type
    wait 2
    quit
    stop snd
    probe n: noise-node 1 1.0
    snd: play n
    probe n/type
    wait 2
    stop snd
    probe n: noise-node 2 1.0
    snd: play n
    probe n/type
    wait 2
    stop snd
    ;loop 10000 [
    ;    a: a + 0.01
    ;    b: b + 0.006
    ;    n/amplitude: (sin a) * cos b
    ;    wait 0.01                                                                                            
    ;]
]
quit

with miniaudio [
    snd1: load %/Users/oldes/Sounds/mixkit-bird-screech-2435.wav
    snd1/pitch: 0.8
    snd1/spatialization: false
    print ["spatialization:" snd1/spatialization]
    print ["volume:" snd1/volume "pan:" snd1/pan "pitch:" snd1/pitch "frames:" snd1/frames "rate:" snd1/sample-rate snd1/file ]
    play snd1
    wait 0:0:1
    print ["playing:" snd1/playing? "at end?" snd1/end? "cursor:"  snd1/cursor]
    wait 2
    print ["playing:" snd1/playing? "at end?" snd1/end? "cursor:"  snd1/cursor]
    snd1/volume: 20%
    snd1/pitch:  2.0
    snd1/pan:   -1.0
    print ["volume:" snd1/volume "pan:" snd1/pan "pitch:" snd1/pitch "frames:" snd1/frames]
    play snd1
    wait 2
    quit
]
;with miniaudio [
;    start snd: load %/Users/oldes/Sounds/mixkit-bird-screech-2435.wav
;    stop/fade snd 28000 ;0:0:1
;    wait 4
;    recycle recycle
;    shutdown
;    print "re"
;    recycle recycle
;    wait 2
;    print "end<"
;]
;quit
;with miniaudio [
;    play/stream/loop %/Users/oldes/Sounds/2ZpZelvaAtmo.mp3
;    wait 2
;    print "re"
;    recycle recycle
;    wait 2
;    quit
;]

with miniaudio [
    test

;    snd1:     load %/Users/oldes/Sounds/mixkit-bird-screech-2435.wav
;    start/seek snd1 0:0:0.5
;    ;volume snd1 1.0
;    wait 2
;    
;
;    quit
;
    sndAtmo: play/stream/loop %/Users/oldes/Sounds/2ZpZelvaAtmo.mp3
    probe looping? sndAtmo
    wait 20


    snd1:     load %/Users/oldes/Sounds/mixkit-bird-screech-2435.wav
    snd2:     load %/Users/oldes/Sounds/mixkit-forest-birds-ambience-1210.wav
    snd3:     load %/Users/oldes/Sounds/mixkit-hawk-call-squawk-1277.wav
    sndRaven: load %/Users/oldes/Sounds/mixkit-wild-raven-bird-calling-62.wav 
    start/loop snd2
    probe looping? snd2
    probe end? snd2
    wait 2
    play sndRaven
    ;stop/fade snd2 0:0:0
    stop/fade snd2 0:0:12
    wait 3

 ;   pause snd2
 ;   wait 2
 ;   play snd2
 ;   wait 1

    play snd1
    probe volume? :snd1
    volume :snd1 50%
    probe volume? :snd1
    pitch :snd1 2.0
    pan   :snd1 -1.0
    probe pan? :snd1
    wait 3
    pan  :snd1 1.0
    pan? :snd1
    play snd1
    wait 1
    play snd3


]
;probe snd2: miniaudio/play %"/Users/oldes/GIT/Builder/tree/rebol/Rebol/src/tests/units/files/drumloop.wav"
;miniaudio/play %"/Users/oldes/Sounds/mixkit-bird-screech-2435.wav"
;miniaudio/play %/Users/oldes/Sounds/mixkit-forest-birds-ambience-1210.wav
;probe snd: miniaudio/load
wait 3
;miniaudio/play snd
;wait 2
;
;miniaudio/test
;print 1
;miniaudio/play
;wait 0.5
;print 2
;miniaudio/play
;loop 100 [prin "." wait 0.1]