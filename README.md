[![rebol-miniaudio](https://github.com/user-attachments/assets/c9c6d0fe-b411-42b9-9c4d-958c4a477ba5)](https://github.com/Oldes/Rebol-MiniAudio)

[![Rebol-MiniAudio CI](https://github.com/Oldes/Rebol-MiniAudio/actions/workflows/main.yml/badge.svg)](https://github.com/Oldes/Rebol-MiniAudio/actions/workflows/main.yml)
[![Gitter](https://badges.gitter.im/rebol3/community.svg)](https://app.gitter.im/#/room/#Rebol3:gitter.im)
[![Zulip](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://rebol.zulipchat.com/)

# Rebol/MiniAudio

[MiniAudio](https://github.com/mackron/miniaudio) extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.14.1 and higher)

## Usage
```rebol
audio: import miniaudio

;; list all available devices:
print audio/get-devices

;; init a playback device (first available)...
;; keep the reference to the device handle, else it will be released by GC!
device: audio/init-playback 1

;; load a sound for later use...
sound: audio/load %assets/zblunk_02.wav

;; play a looping sound...
drums: audio/play/loop %assets/drumloop.wav

;; list resources linked with the playback device...
print ["Available sounds:" mold device/resources]

;; work in Rebol as usually, the audio has own thread
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
```

#### Using a data-source node...
```rebol
a: 0
b: PI
with audio [
    probe wave: make-waveform-node type_sine 0.5 440.0
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
```

#### Using a waveform to play a morse sound
```rebol
with audio [
    ;; initialize an audio device...
    device: init-playback 1

    ;; create a waveform
    wave: make-waveform-node type_sine 0.5 500.0

    ;; start the sound to be reused for the beep (paused)
    stop snd: play :wave

    ;; beep function accepting time how long 
    beep: function[time [decimal! time!]][
        start :snd
        wait time
        stop :snd
        wait 0.1
    ]
    dot:  does[beep 0.1]
    dash: does[beep 0.3]
]
dot dot dot dash dash dash dot dot dot
```

#### Release the playback device (and all resources)
```rebol
release device
```

## Extension commands:


#### `get-devices`
Retrive playback/capture device names

#### `init-playback` `:index`
Initialize a playback device
* `index` `[integer!]`
* `/pause` Don't start it automatically
* `/channels` The number of channels to use for playback
* `number` `[integer!]` When set to 0 the device's native channel count will be used
* `/period` Hint for making up the device's entire buffer
* `size` `[integer!]` The desired size of a period in milliseconds
* `/callback` On-data callback (two args.. buffer frames, and engine total frames)
* `context` `[object!]` The function's context
* `word` `[word!]` The function's name

#### `load` `:sound`
Loads a file and returns sound's handle
* `sound` `[file!]`
* `/group` Group of sounds which have their own effect processing and volume control
* `node` `[handle!]` ma-group handle

#### `play` `:sound`
Loads a file (if not already loaded) and starts playing it. Returns a sound handle.
* `sound` `[file! handle!]` Source file or a ma-sound handle
* `/stream` Do not load the entire sound into memory
* `/loop` Turn looping on
* `/volume`
* `vol` `[percent! decimal!]`
* `/fade`
* `in` `[integer! time!]` PCM frames or time
* `/group` Group of sounds which have their own effect processing and volume control
* `node` `[handle!]` ma-group handle

#### `pause` `:sound`
Pause sound playback
* `sound` `[handle!]`

#### `start` `:handle`
Start sound or device playback
* `handle` `[handle!]` ma-sound or ma-engine handle
* `/loop` Turn looping on (only for sounds)
* `/seek` Starting position
* `frames` `[integer! time!]` PCM frames or time
* `/fade` Fade in the sound
* `in` `[integer! time!]` PCM frames or time
* `/at` Absolute engine time when the sound should be started
* `time` `[integer! time!]` PCM frames or time

#### `stop` `:handle`
Stop sound or device playback
* `handle` `[handle!]` ma-sound or ma-engine handle
* `/fade`
* `out` `[integer! time!]` PCM frames or time (only for sounds)

#### `fade` `:sound` `:frames` `:start` `:end`
Fade sound volume
* `sound` `[handle!]`
* `frames` `[integer! time!]`
* `start` `[percent! decimal!]`
* `end` `[percent! decimal!]`

#### `seek` `:sound` `:frames`
Seek to specified position
* `sound` `[handle!]`
* `frames` `[integer! time!]`
* `/relative` Relative to the current sound position

#### `make-noise-node` `:type` `:amplitude`
Creates a noise node for generating random noise
* `type` `[integer!]` The type of noise to generate (0 - 2)
* `amplitude` `[decimal!]` The peak amplitude of the noise
* `/seed` Optional random seed
* `val` `[integer!]`
* `/format` The sample format (default is 2 = signed 16bit float)
* `frm` `[integer!]` Value betweem 1 - 5

#### `make-waveform-node` `:type` `:amplitude` `:frequency`
Creates a sound waveform node
* `type` `[integer!]` The type of waveform to generate (0 - 3)
* `amplitude` `[decimal!]` The peak amplitude of the waveform
* `frequency` `[decimal!]` The frequency of the waveform in Hertz (Hz)
* `/format` The sample format (default is 2 = signed 16bit float)
* `frm` `[integer!]` Value betweem 1 - 5

#### `make-delay-node` `:delay` `:decay`
Creates a delay (echo) sound node
* `delay` `[decimal! integer! time!]` The time before the echo is heard. Seconds, PCM frames or time.
* `decay` `[decimal! percent!]` Feedback decay (0.0 - 1.0). Affects how quickly or gradually the echoes fade away. 0 means no feedback.
* `/dry` The mix level of the dry (original) sound
* `d` `[decimal! percent!]`
* `/wet` The mix level of the wet (delayed) sound
* `w` `[decimal! percent!]`

#### `make-group-node`
Creates a sound group node

#### `volume` `:sound` `:volume`
Set the volume
* `sound` `[handle!]`
* `volume` `[percent! decimal!]`

#### `volume?` `:sound`
Get the volume
* `sound` `[handle!]`

#### `pan` `:sound` `:pan`
Set the pan
* `sound` `[handle!]`
* `pan` `[decimal!]`

#### `pan?` `:sound`
Get the pan
* `sound` `[handle!]`

#### `pitch` `:sound` `:pitch`
Set the pitch
* `sound` `[handle!]`
* `pitch` `[decimal!]`

#### `pitch?` `:sound`
Get the pitch
* `sound` `[handle!]`

#### `looping` `:sound` `:value`
Set the looping
* `sound` `[handle!]`
* `value` `[logic!]`

#### `looping?` `:sound`
Get the looping
* `sound` `[handle!]`

#### `end?` `:sound`
Return true if sound ended
* `sound` `[handle!]`


## Used handles and its getters / setters

#### __MA-SOUND__ - MiniAudio sound object

```rebol
;Refinement       Gets                Sets                          Description
/volume           decimal!            [integer! decimal! percent!]  "Sound volume"
/pan              decimal!            decimal!                      "Stereo panning (from -1.0 to 1.0)"
/pitch            decimal!            decimal!                      "Sound pitch"
/position         pair!               pair!                         "Sound position (x and y for now) relative to the listener"
/cursor           integer!            [integer! time!]              "Sound playback position in PCM frames"
/time             time!               time!                         "Sound playback position as time"
/duration         time!               none                          "Sound duration in time"
/frames           integer!            none                          "Sound length in PCM frames"
/sample-rate      integer!            none                          "Number of samples per second"
/spatialize       logic!              logic!                        "3D spatialization state"
/is-looping       logic!              logic!                        "Whether sound is looping"
/is-playing       logic!              logic!                        "Whether sound is playing"
/at-end           logic!              none                          "Whether sound is at end"
/start            integer!            [integer! time!]              "Absolute timer when the sound should be started (frames or time)"
/stop             integer!            [integer! time!]              "Absolute timer when the sound should be stopped (frames or time)"
/x                decimal!            [integer! decimal!]           "Sound X position"
/y                decimal!            [integer! decimal!]           "Sound Y position"
/z                decimal!            [integer! decimal!]           "Sound Z position"
/source           [file! handle!]     none                          "Sound source as a loaded file or data source node"
/outputs          integer!            none                          "Number of output buses"
/output           handle!             [handle! none!]               "Output bus node"
```

#### __MA-GROUP__ - MiniAudio sound group

```rebol
;Refinement       Gets                Sets                          Description
/volume           decimal!            [integer! decimal! percent!]  "Sound volume"
/pan              decimal!            decimal!                      "Stereo panning (from -1.0 to 1.0)"
/pitch            decimal!            decimal!                      "Sound group pitch"
/position         pair!               pair!                         "Sound group position (x and y for now) relative to the listener"
/time             time!               time!                         "Sound group playback position as time"
/duration         time!               none                          "Sound group duration in time"
/sample-rate      integer!            none                          "Number of samples per second"
/spatialize       logic!              logic!                        "3D spatialization state"
/is-playing       logic!              logic!                        "Whether sound is playing"
/start            integer!            [integer! time!]              "Absolute timer when the sound should be started (frames or time)"
/stop             integer!            [integer! time!]              "Absolute timer when the sound should be stopped (frames or time)"
/x                decimal!            [integer! decimal!]           "Sound group X position"
/y                decimal!            [integer! decimal!]           "Sound group Y position"
/z                decimal!            [integer! decimal!]           "Sound group Z position"
/outputs          integer!            none                          "Number of output buses"
/output           handle!             [handle! none!]               "Output bus node"
/resources        block!              none                          "Used group resources (sounds, nodes..)"
```

#### __MA-ENGINE__ - MiniAudio device engine

```rebol
;Refinement       Gets                Sets                          Description
/volume           decimal!            [integer! decimal! percent!]  "Global volume"
/cursor           integer!            [integer! time!]              "Engine playback position in PCM frames"
/time             time!               time!                         "Engine playback position as time"
/resources        block!              none                          "Used engine resources (sounds, nodes..)"
/channels         integer!            none                          "Number of output channels"
/sample-rate      integer!            none                          "Ouput device sample rate per second"
/gain-db          decimal!            [integer! decimal!]           "The amplification factor in decibels"
```

#### __MA-NOISE__ - MiniAudio noise generator

```rebol
;Refinement       Gets                Sets                          Description
/amplitude        decimal!            decimal!                      "Maximum value of the noise signal"
/format           word!               none                          "f32, s16, s24, s32, u8"
/type             word!               none                          "white, pink or brownian"
```

#### __MA-WAWEFORM__ - MiniAudio sine, square, triangle and sawtooth waveforms generator

```rebol
;Refinement       Gets                Sets                          Description
/amplitude        decimal!            decimal!                      "Signal amplitude"
/frequency        decimal!            decimal!                      "Signal frequency in hertzs"
/format           word!               none                          "f32, s16, s24, s32, u8"
/type             word!               none                          "sine, square, triangle or sawtooth"
```

#### __MA-DELAY__ - MiniAudio delay node

```rebol
;Refinement       Gets                Sets                          Description
/delay            integer!            none                          "PCM frames"
/decay            decimal!            [decimal! percent!]           "Value between 0.0 and 1.0"
/dry              decimal!            [decimal! percent!]           "The mix level of the dry (original) sound"
/wet              decimal!            [decimal! percent!]           "The mix level of the wet (delayed) sound"
```


## Other extension values:
```rebol
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
```
