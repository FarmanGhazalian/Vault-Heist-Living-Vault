#pragma once

class AnimationController
{
public:
    static inline float s_speed = 1.0f;

    static void setSpeed(float speed)
    {
        if (speed <= 0.1f) speed = 0.5f;
        s_speed = speed;
    }

    static float getSpeed()
    {
        return s_speed;
    }

    static float apply(float dt)
    {
        return dt * s_speed;
    }
};
