#ifndef _ABOUT_H_
#define _ABOUT_H_

#include "../../config.h"

/*
 Notes:-
 - The text should be brief and on point; there is limited flash memory.
 - There is no need to meticulously style or format the text; it automatically resizes to fit the screen.
 - Line feed and form feed can as well be used. Carriage return is ignored.
 - Do not remove or delete the copyright notices, as long as the code uses the relevant components.
*/

static const char versionText[] PROGMEM = 
  "FW version: "
  _SKETCHVERSION
  "\n(c) 2020-2024 Buk7456"
  "\nhttps://github.com/\nbuk7456/FreeTX";

static const char thirdPartyNoticesText[] PROGMEM =
  "The software uses, with permissions, code from the following open source libraries: "
  "\f"
  "Adafruit GFX library"
  "\nCopyright (c) 2013 Adafruit Industries"
  "\f"
  "SparkFun External EEPROM Arduino library"
  "\nCopyright (c) 2016 SparkFun Electronics"
  "\f"
  "Arduino-Lora library"
  "\nCopyright (c) 2016 Sandeep Mistry"
  "\f"
  "Special thanks to all the contributors to the Arduino core "
  "and standard libraries that this software uses.";

static const char disclaimerText[] PROGMEM = 
  "The software is provided \"as is\" without warranty of any "
  "kind, express or implied. In no event shall the authors or copyright holders be liable "
  "for any direct, indirect, incidental, special, exemplary or consequential "
  "damages (including but not limited to personal and/or property damage) or "
  "other liability arising from the use of the software. "
  "\f"
  "Improperly operating RC models can cause serious injury or death.";

#endif
