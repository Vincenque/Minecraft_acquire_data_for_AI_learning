#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <stdio.h>

// Function to convert an image to grayscale
void convert_to_grayscale(unsigned char *image, int width, int height,
                          int channels) {
  if (channels < 3) {
    printf("Image does not have enough channels to convert to grayscale.\n");
    return;
  }

  for (int i = 0; i < width * height; ++i) {
    int r = image[i * channels];
    int g = image[i * channels + 1];
    int b = image[i * channels + 2];
    unsigned char gray = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
    image[i * channels] = gray;
    image[i * channels + 1] = gray;
    image[i * channels + 2] = gray;
  }
}

int main() {
  int width, height, channels;

  // Load the image
  unsigned char *image =
      stbi_load("assets/test_screen.png", &width, &height, &channels, 0);
  if (!image) {
    printf("Failed to load image.\n");
    return 1;
  }

  printf("Image loaded: %dx%d with %d channels.\n", width, height, channels);

  // Convert the image to grayscale
  convert_to_grayscale(image, width, height, channels);

  // Save the grayscale image
  if (!stbi_write_png("assets/test_screen_grayscaled.png", width, height,
                      channels, image, width * channels)) {
    printf("Failed to save grayscale image.\n");
  } else {
    printf("Grayscale image saved as assets/test_screen_grayscaled.png\n");
  }

  // Free the image memory
  stbi_image_free(image);

  printf("Work done!\n");

  return 0;
}