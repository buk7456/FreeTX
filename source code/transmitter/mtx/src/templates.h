#ifndef _TEMPLATES_H_
#define _TEMPLATES_H_

void loadMixerTemplateBasic(uint8_t mixIdx);
void loadMixerTemplateElevon(uint8_t mixIdx);
void loadMixerTemplateVtail(uint8_t mixIdx);
void loadMixerTemplateDiffThrust(uint8_t mixIdx);

void loadSensorTemplateExtVolts2S(uint8_t telemIdx);
void loadSensorTemplateExtVolts3S(uint8_t telemIdx);
void loadSensorTemplateExtVolts4S(uint8_t telemIdx);
void loadSensorTemplateRSSI(uint8_t telemIdx);
void loadSensorTemplateLinkQuality(uint8_t telemIdx);
void loadSensorTemplateGNSS(uint8_t telemIdx);
void loadSensorTemplateGNSSSpeed(uint8_t telemIdx);
void loadSensorTemplateGNSSDistance(uint8_t telemIdx);
void loadSensorTemplateGNSSMSLAltitude(uint8_t telemIdx);
void loadSensorTemplateGNSSAGLAltitude(uint8_t telemIdx);

uint8_t getChannelIdx(char c);

#endif
