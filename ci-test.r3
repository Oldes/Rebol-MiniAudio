Rebol [
    title: "Rebol/MiniAudio extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]
system/options/quiet: false
system/options/log/rebol: 4

;; make sure that we load a fresh extension
try [system/modules/miniaudio: none]
;; use current directory as a modules location
system/options/modules: what-dir

;; import the extension
audio: import 'miniaudio

print as-yellow "Content of the module..."
? audio

print ["MiniAudio native version:" audio/version]

;; for the CI test, try to use first playback device available
;; paused for the purpose of the test!
try/with [
    device: audio/init-playback/pause 1
][
    print "There may be no device available in the CI runner."
    print system/state/last-error
    quit
]

;; run a real test file...
do %test.r3