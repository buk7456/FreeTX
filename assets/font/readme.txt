Converting from vertical to horizontal byte ordering.
======================================================
- Use image2cpp tool, paste the byte array contained in the font.cpp file.
- Set width to 256*5 = 1280 px, height to 8 px
- Save the resulting image.
- Use the python script to slice the image horizontally, with width 5 pixels.
  (You may need to install Python Imaging Library using "pip install pillow" command if not already installed.)
- Import the images into the image2cpp tool.
- Set the ordering to horizontal, and convert to byte array.
