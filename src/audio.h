#ifndef PROJECTNAME_AUDIO_H
#define PROJECTNAME_AUDIO_H


#include "types.h"


typedef enum {

    SAMPLE_NONE = 0,
    SAMPLE_START_BEEP = 1,
    SAMPLE_CHOOSE = 2,
    SAMPLE_SELECT = 3,
    SAMPLE_INTRO_BEEP = 4,
    SAMPLE_MOVE_BEEP = 5,
    SAMPLE_DESTROY = 6,
    SAMPLE_STAR = 7,
    SAMPLE_TOGGLE_WALLS = 8,
    SAMPLE_PAUSE = 9,

    SAMPLE_LAST = 10

} PredefinedSample;



EMPTY_STRUCT(AudioSystem);


AudioSystem* new_audio_system(u16 bufferSize);
void dispose_audio_system(AudioSystem* audio);

void audio_play_sound(AudioSystem* audio, i16 freq, i16 len);
void audio_play_sequence(AudioSystem* audio, const i16* seq, i16 len);
void audio_play_predefined_sample(AudioSystem* audio, PredefinedSample name);

void audio_update(AudioSystem* audio, i16 step);

bool audio_is_enabled(AudioSystem* audio);
void audio_toggle(AudioSystem* audio, bool state);

#endif //PROJECTNAME_AUDIO_H
