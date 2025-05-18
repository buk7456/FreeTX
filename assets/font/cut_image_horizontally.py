from PIL import Image
import os

def cut_image(image_path, cut_width, output_folder):
    # Open the image
    image = Image.open(image_path)
    
    # Get image dimensions
    image_width, image_height = image.size
    
    # Create output folder if it doesn't exist
    os.makedirs(output_folder, exist_ok=True)
    
    # Calculate number of horizontal cuts
    num_cuts = image_width // cut_width
    if image_width % cut_width != 0:
        num_cuts += 1
    
    # Cut the image and save the resulting parts
    for i in range(num_cuts):
        left = i * cut_width
        right = min((i + 1) * cut_width, image_width)
        
        # Crop the image to the current section
        cropped_image = image.crop((left, 0, right, image_height))
        
        # Save the cropped image
        cropped_image.save(os.path.join(output_folder, f"img_{i}.png"))

# Prompt the user for inputs
image_path = input("Enter the path to the image: ")
cut_width = int(input("Enter the cut width: "))
output_folder = input("Enter the output folder (leave empty for default 'output_images'): ")

# Use default output folder if none is provided
if not output_folder:
    output_folder = 'output_images'

cut_image(image_path, cut_width, output_folder)
