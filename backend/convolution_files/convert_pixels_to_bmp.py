from PIL import Image

# Read space-separated grayscale pixel values from file
def read_pixel_values(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()
        pixel_values = []
        height = 0
        width = 0
        for line in lines:
            pixels = line.strip().split()
            pixel_values.extend(map(int, pixels))
            height += 1
            width = len(pixels)
    return pixel_values, width, height

# Example usage:
# Read space-separated grayscale pixel values from file
pixel_values, width, height = read_pixel_values('input.txt')

# Print image dimensions
print("Image dimensions:", width, "x", height)

# Create a PIL image from pixel values
image = Image.new('L', (width, height))

# Put the BMP-compatible pixel data into the image
print(pixel_values)
print(len(pixel_values))
image.putdata(pixel_values)

# Save the image as BMP
image.save('output.bmp')

print("BMP image created successfully.")
