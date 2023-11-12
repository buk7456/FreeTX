#ifndef _TEMPLATES_H_
#define _TEMPLATES_H_

void loadMixerTemplateBasic(uint8_t mixIdx);
void loadMixerTemplateElevon(uint8_t mixIdx);
void loadMixerTemplateVtail(uint8_t mixIdx);
void loadMixerTemplateDiffThrust(uint8_t mixIdx);

void loadSensorTemplateExtVolts2S(uint8_t telemIdx);
void loadSensorTemplateExtVolts3S(uint8_t telemIdx);
void loadSensorTemplateExtVolts4S(uint8_t telemIdx);
void loadSensorTemplateRPM2Blades(uint8_t telemIdx);
void loadSensorTemplateRPM3Blades(uint8_t telemIdx);
void loadSensorTemplateRPM4Blades(uint8_t telemIdx);
void loadSensorTemplateTemperature(uint8_t telemIdx);
void loadSensorTemplateRSSI(uint8_t telemIdx);

uint8_t getChannelIdx(char c);

#endif