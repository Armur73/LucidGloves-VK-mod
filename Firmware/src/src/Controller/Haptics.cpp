#include "Haptics.h"

#if USING_FORCE_FEEDBACK
int setupPos[5] = {0, 0, 0, 0, 0};
void Haptics::setupServoHaptics() {
    pinkyServo.attach(PIN_PINKY_MOTOR);
    ringServo.attach(PIN_RING_MOTOR);
    middleServo.attach(PIN_MIDDLE_MOTOR);
    indexServo.attach(PIN_INDEX_MOTOR);
    thumbServo.attach(PIN_THUMB_MOTOR);
    writeServoHaptics(setupPos);
}

void Haptics::scaleLimits(int* hapticLimits, float* scaledLimits) {
    for (int i = 0; i < 5; i++) {
#if FLIP_FORCE_FEEDBACK
        scaledLimits[i] = hapticLimits[i] / 1000.0f * 180.0f;
#else
        scaledLimits[i] = 180.0f - hapticLimits[i] / 1000.0f * 180.0f;
#endif
    }
}

void Haptics::dynScaleLimits(int* hapticLimits, float* scaledLimits) {
    for (int i = 0; i < sizeof(hapticLimits); i++) {
        scaledLimits[i] = hapticLimits[i] / 1000.0f * 180.0f;
    }
}

void Haptics::writeServoHaptics(int* hapticLimits) {
    float scaledLimits[5];
    scaleLimits(hapticLimits, scaledLimits);
    nehejbej = true;
    if (hapticLimits[0] >= 0) thumbServo.write(scaledLimits[0]);
    delayMicroseconds(100000);
    if (hapticLimits[1] >= 0) indexServo.write(scaledLimits[1]);
    delayMicroseconds(100000);
    if (hapticLimits[2] >= 0) middleServo.write(scaledLimits[2]);
    delayMicroseconds(100000);
    if (hapticLimits[3] >= 0) ringServo.write(scaledLimits[3]);
    delayMicroseconds(100000);
    if (hapticLimits[4] >= 0) pinkyServo.write(scaledLimits[4]);
    delayMicroseconds(100000);
    nehejbej = false;
}



#endif

bool Haptics::nehejbejSe() {
    return nehejbej;
}