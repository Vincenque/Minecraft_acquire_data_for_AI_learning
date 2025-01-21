#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <stdio.h>

// Function to convert an image based on the condition (R, G, B) = (221, 221, 221)
void filter_image(unsigned char *image, int width, int height, int channels) {
	if (channels < 3) {
		printf("Image does not have enough channels to process.\n");
		return;
	}

	for (int i = 0; i < width * height; ++i) {
		int r = image[i * channels];
		int g = image[i * channels + 1];
		int b = image[i * channels + 2];

		// Check if pixel (r, g, b) is equal to (221, 221, 221)
		if (r == 221 && g == 221 && b == 221) {
			// Set value to 255
			image[i * channels] = 255;
			image[i * channels + 1] = 255;
			image[i * channels + 2] = 255;
		} else {
			// Set value to 0
			image[i * channels] = 0;
			image[i * channels + 1] = 0;
			image[i * channels + 2] = 0;
		}
	}
}

int main() {
	int width, height, channels;

	// Load the image
	unsigned char *image = stbi_load("assets/test_screen.png", &width, &height, &channels, 0);
	if (!image) {
		printf("Failed to load image.\n");
		return 1;
	}

	printf("Image loaded: %dx%d with %d channels.\n", width, height, channels);

	// Apply the filter to the image
	filter_image(image, width, height, channels);

	// Save the filtered image as test_screen_filtered.png
	if (!stbi_write_png("assets/test_screen_filtered.png", width, height, channels, image, width * channels)) {
		printf("Failed to save the filtered image.\n");
		stbi_image_free(image);
		return 1;
	}

	printf("Filtered image saved as test_screen_filtered.png.\n");

	// Free the image memory
	stbi_image_free(image);

	printf("Work done!\n");

	return 0;
}