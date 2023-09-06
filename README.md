[![Rebol-MiniAudio CI](https://github.com/Oldes/Rebol-MiniAudio/actions/workflows/main.yml/badge.svg)](https://github.com/Oldes/Rebol-MiniAudio/actions/workflows/main.yml)

# Rebol/MiniAudio

[MiniAudio](https://github.com/mackron/miniaudio) extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.12.2 and higher)

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

#### `load` `:sound`
Loads a file and returns sound's handle
* `sound` `[file!]`

#### `play` `:sound`
Loads a file (if not already loaded) and starts playing it. Returns a sound handle.
* `sound` `[file! handle!]` Source file or a ma-sound handle
* `/stream` Do not load the entire sound into memory
* `/loop` Turn looping on
* `/volume`
* `vol` `[percent! decimal!]`
* `/fade`
* `in` `[integer! time!]` PCM frames or time

#### `pause` `:sound`
Pause sound playback
* `sound` `[handle!]`

#### `start` `:sound`
Start sound playback
* `sound` `[handle!]`
* `/loop` Turn looping on
* `/seek` Specify starting position
* `frames` `[integer! time!]`
* `/fade`
* `in` `[integer! time!]` PCM frames or time

#### `stop` `:sound`
Stop sound playback
* `sound` `[handle!]`
* `/fade`
* `out` `[integer! time!]` PCM frames or time

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

#### `noise-node` `:type` `:amplitude`
Create a noise node data source
* `type` `[integer!]`
* `amplitude` `[decimal!]`
* `/seed` Optional random seed
* `val` `[integer!]`
* `/format` The sample format (default is 2 = signed 16bit float)
* `frm` `[integer!]` Value betweem 1 - 5

#### `waveform-node` `:type` `:amplitude` `:frequency`

* `type` `[integer!]`
* `amplitude` `[decimal!]`
* `frequency` `[decimal!]`
* `/format` The sample format (default is 2 = signed 16bit float)
* `frm` `[integer!]` Value betweem 1 - 5

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
