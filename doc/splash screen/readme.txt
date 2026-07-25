
- The firmware now supports monochrome bitmap images for the splash screen.
- To make a custom splash screen, create a monochrome bitmap image with 128x64 px resolution and 
  save it as "SPLASH.BMP" inside the "IMAGES" folder on the SD card.


Legacy instructions below.
--------------------------

Prerequisites
- Python installed on your computer
- Python Imaging Library (install via command "pip install pillow" if not already installed)

Steps
1. Make or prepare the image using any image editing software, with dimensions 128 px width and 64 px height.
2. Export it as "image.bmp" file.
3. Put the file into the same folder as the python script, then run the python script.
4. Create a folder "IMAGES" on the SD card, then copy the generated "splash" to it.

Now whenever you power on your rc transmitter, it shows your custom splash screen. 
