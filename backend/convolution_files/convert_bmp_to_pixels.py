from PIL import Image

# Open the BMP image
image = Image.open('test.bmp')

# Convert the image to grayscale (if necessary)
image = image.convert('L')

# Get pixel data
pixel_values = list(image.getdata())

# Determine width and height of the image
width, height = image.size

# Convert pixel values to a list of lists (rows)
rows = [pixel_values[i * width:(i + 1) * width] for i in range(height)]

# Print image dimensions
print("Image dimensions:", len(rows), "x", len(rows[0]))

# Print grayscale pixel values matrix
print("Grayscale pixel values matrix:")
for row in rows:
    print(row, sep=' ')

# Write to text file
with open('input.txt', 'w') as f:
    for row in rows:
        f.write(' '.join(map(str, row)) + '\n')