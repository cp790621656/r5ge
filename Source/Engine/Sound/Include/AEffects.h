#pragma once

//============================================================================================================
//          R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Sound effect that can be applied to a sound
//============================================================================================================

typedef struct ReverbProp
{   
    int          Room;
    int          RoomHF;
    float        RoomRolloffFactor;
    float        DecayTime;
    float        DecayHFRatio;
    int          Reflections;
    float        ReflectionsDelay;
    int          Reverb;
    float        ReverbDelay;
    float        Diffusion;
    float        Density;
    float        HFReference;
} ReverbProp;

#define FX_AUDITORIUM { -1000, -476, 0, 4.32f, 0.59f, -789, 0.020f, -789, 0.020f, 100.0f, 100.0f, 5000.0f }