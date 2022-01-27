#include "audio.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <i86.h>


#include "audiosrc.h"


typedef struct {

    i16* soundBuffer;
    i16* lengthBuffer;
    u16 bufferSize;
    u16 bufferPointer;

    i16 timer;
    u16 soundIndex;

} _AudioSystem;


static void add_sound(_AudioSystem* audio, i16 freq, i16 len) {

    if (audio->bufferPointer == audio->bufferSize)
        return;

    audio->soundBuffer[audio->bufferPointer] = freq;
    audio->lengthBuffer[audio->bufferPointer] = len;

    ++ audio->bufferPointer;
}


static void next_sound(_AudioSystem* audio) {

    if (audio->soundBuffer[audio->soundIndex] < 0)
        return;

    sound(audio->soundBuffer[audio->soundIndex]);
    audio->timer = audio->lengthBuffer[audio->soundIndex];
}


AudioSystem* new_audio_system(u16 bufferSize) {

    _AudioSystem* audio = (_AudioSystem*) calloc(1, sizeof(_AudioSystem));
    if (audio == NULL) {

        m_memory_error();
        return NULL;
    }

    audio->soundBuffer = (i16*) calloc(bufferSize, sizeof(i16));
    if (audio->soundBuffer == NULL) {

        m_memory_error();
        dispose_audio_system((AudioSystem*) audio);
        return NULL;
    } 

    audio->lengthBuffer = (i16*) calloc(bufferSize, sizeof(i16));
    if (audio->lengthBuffer == NULL) {

        m_memory_error();
        dispose_audio_system((AudioSystem*) audio);
        return NULL;
    } 

    audio->bufferPointer = 0;
    audio->bufferSize = bufferSize;
    audio->soundIndex = 0;

    audio->timer = 0;

    return (AudioSystem*) audio;
}


void dispose_audio_system(AudioSystem* _audio) {

    _AudioSystem* audio = (_AudioSystem*) _audio;

    if (audio == NULL) return;

    m_free(audio->soundBuffer);
    m_free(audio->lengthBuffer);
    m_free(audio);
}


void audio_play_sound(AudioSystem* _audio, i16 freq, i16 len) {

    _AudioSystem* audio = (_AudioSystem*) _audio;

    audio->bufferPointer = 0;
    audio->soundIndex = 0;

    add_sound(audio, freq, len);
}


void audio_play_sequence(AudioSystem* _audio, const i16* seq, i16 len) {

    i16 i;
    _AudioSystem* audio = (_AudioSystem*) _audio;

    if (len % 2 != 0) return;

    audio->bufferPointer = 0;
    audio->soundIndex = 0;

    for (i = 0; i < len; i += 2) {

        if (seq[i] <= 0 || seq[i + 1] <= 0)
            break;

        add_sound(audio, seq[i], seq[i + 1]);
    }

    if (audio->bufferPointer < audio->bufferSize) {

        add_sound(audio, -1, -1);
    }
}


void audio_play_predefined_sample(AudioSystem* audio, PredefinedSample name) {

    i16 p = name;
    if (p <= 0 || p >= SAMPLE_LAST)
        return;

    audio_play_sequence(audio, AUDIO_SOURCE[p-1], SRC_LEN);
}


void audio_update(AudioSystem* _audio, i16 step) {

    _AudioSystem* audio = (_AudioSystem*) _audio;

    if (audio->bufferPointer == 0) return;

    if (audio->timer <= 0) {

        next_sound(audio);
    }
    else {

        if ((audio->timer -= step) <= 0) {

            nosound();
            
            ++ audio->soundIndex;
            if (audio->soundIndex < audio->bufferPointer) {

                next_sound(audio);
            }
        }
    }
}
