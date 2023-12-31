Rebol [
    title: "Rebol/MiniAudio extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]

;; make sure that we load a fresh extension
try [system/modules/miniaudio: none]
;; use current directory as a modules location
system/options/modules: what-dir

;; import the extension
audio: import 'miniaudio

print as-yellow "Content of the module..."
? audio

print as-yellow "List all available playback and capture devices..."
probe load audio/get-devices

;; Is there any audio available for CI test?
