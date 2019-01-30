#include "Tone32.h"

void tone(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel)
{
    if (ledcRead(channel)) {
        //Serial.println("Tone channel is already in use");
        return;
    }
    ledcAttachPin(pin, channel);
    //ledcWriteTone(channel, frequency);
    ledcSetup(channel, frequency, 8);
    
    if (duration) {
        ledcWrite(pin, 128);
        delay(duration);
        noTone(pin, channel);
    }    
}

void noTone(uint8_t pin, uint8_t channel)
{
    ledcDetachPin(pin);
    ledcWrite(channel, 0);
}
