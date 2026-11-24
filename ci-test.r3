Rebol [
    title: "Rebol/MiniAudio extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]
system/options/quiet: false
system/options/log/rebol: 4

;; make sure that we load a fresh extension
try [system/modules/miniaudio: none]

if CI?: any [
    "true" = get-env "CI"
    "true" = get-env "GITHUB_ACTIONS"
    "true" = get-env "TRAVIS"
    "true" = get-env "CIRCLECI"
    "true" = get-env "GITLAB_CI"
][
    ;; configure modules location for the CI test 
    system/options/modules: dirize to-rebol-file any [
        get-env 'REBOL_MODULES_DIR
        what-dir
    ]
]


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