import os
import struct
from PIL import Image

image_path  = "image.bmp"

# Define the output filename and path
output_file = "splash"
output_path = "./"

# Open the output file in binary mode
with open(os.path.join(output_path, output_file), "wb") as f:

    # Open the image file and convert it to black and white
    with Image.open(image_path) as img:
        img = img.convert("1")
        pixels = img.getdata()
        
        width, height = img.size
        
        # Convert the pixels to bytes using vertical ordering for every 8 rows
        for y_block in range(height // 8):
            for x in range(width):
                byte = 0
                bit_count = 0
                for y_offset in range(8):
                    y = y_block * 8 + y_offset
                    pixel = pixels[y * width + x]
                    # Set the corresponding bit in the byte for each pixel
                    if pixel:
                        # For a white pixel, set the bit to 1
                        byte |= (1 << (bit_count % 8))
                    bit_count += 1
                    if bit_count % 8 == 0:
                        # Write the byte to the output file and reset the bit count and byte
                        f.write(bytes([byte]))
                        byte = 0
                # If there are any remaining bits, write the final byte to the output file
                if bit_count % 8 != 0:
                    f.write(bytes([byte]))
        
        # If there are any remaining rows, write them to the output file
        if height % 8 != 0:
            y_block = height // 8
            for x in range(width):
                byte = 0
                bit_count = 0
                for y_offset in range(height % 8):
                    y = y_block * 8 + y_offset
                    pixel = pixels[y * width + x]
                    # Set the corresponding bit in the byte for each pixel
                    if pixel:
                        # For a white pixel, set the bit to 1
                        byte |= (1 << (bit_count % 8))
                    bit_count += 1
                    if bit_count % 8 == 0:
                        # Write the byte to the output file and reset the bit count and byte
                        f.write(bytes([byte]))
                        byte = 0
                # If there are any remaining bits, write the final byte to the output file
                if bit_count % 8 != 0:
                    f.write(bytes([byte]))

print("Done")