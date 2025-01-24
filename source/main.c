#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to convert RGB image to single-channel binary image
unsigned char *convert_to_single_channel(unsigned char *image, int width, int height, int channels) {
	if (channels < 3) {
		printf("Image does not have enough channels to process.\n");
		return NULL;
	}

	// Allocate memory for single-channel image
	unsigned char *single_channel_image = (unsigned char *)malloc(width * height);
	if (!single_channel_image) {
		printf("Failed to allocate memory for single-channel image.\n");
		return NULL;
	}

	for (int i = 0; i < width * height; ++i) {
		// Extract RGB values
		unsigned char r = image[i * channels];
		unsigned char g = image[i * channels + 1];
		unsigned char b = image[i * channels + 2];

		// Check if all channels are equal to 221, map to 255; otherwise, map to 0
		if (r == 221 && g == 221 && b == 221) {
			single_channel_image[i] = 255;
		} else {
			single_channel_image[i] = 0;
		}
	}

	return single_channel_image;
}

void divide_single_channel_image_to_columns(unsigned char *image, int width, int height) {
	// Find the last row with a white pixel (255)
	int last_white_row = -1;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (image[row * width + col] == 255) {
				last_white_row = row;
				break;
			}
		}
	}
	
	// Adjust the height based on last white row and division by 18
	if (last_white_row != -1) {
		int new_height = last_white_row + 1;
		int remainder = (new_height - 2) % 18;
		if (remainder != 0) {
			new_height += (18 - remainder);
		}
		if (new_height > height) {
			new_height = height;
		}
		height = new_height;
	}

	// Skip the first two rows
	image += 2 * width;
	height -= 2;

	// Validate dimensions
	if (width % 2 != 0) {
		printf("Error: Image width must be even to divide into two equal columns.\n");
		return;
	}

	// Calculate dimensions for each column
	int column_width = width / 2;

	// Allocate memory for the left and right columns
	unsigned char *left_column_image = (unsigned char *)malloc(column_width * height * sizeof(unsigned char));
	unsigned char *right_column_image = (unsigned char *)malloc(column_width * height * sizeof(unsigned char));

	if (!left_column_image || !right_column_image) {
		printf("Error: Memory allocation failed.\n");
		free(left_column_image);
		free(right_column_image);
		return;
	}

	// Copy pixels to the left and right column images
	for (int row = 0; row < height; row++) {
		memcpy(left_column_image + row * column_width, image + row * width, column_width);
		memcpy(right_column_image + row * column_width, image + row * width + column_width, column_width);
	}

	// Save the images to files in the assets folder
	if (stbi_write_png("assets/left_column_image.png", column_width, height, 1, left_column_image, column_width) == 0) {
		printf("Error: Could not save left column image.\n");
	} else {
		printf("Left column image saved to assets/left_column_image.png\n");
	}

	if (stbi_write_png("assets/right_column_image.png", column_width, height, 1, right_column_image, column_width) == 0) {
		printf("Error: Could not save right column image.\n");
	} else {
		printf("Right column image saved to assets/right_column_image.png\n");
	}

	// Free allocated memory
	free(left_column_image);
	free(right_column_image);

	printf("Image successfully divided and saved.\n");
}

int main() {
	int width, height, channels;

	// Load the image
	unsigned char *image = stbi_load("assets/test_screen.png", &width, &height, &channels, 0);
	if (!image) {
		printf("Failed to load the image.\n");
		return 1;
	}

	// Convert the filtered image to single-channel
	unsigned char *single_channel_image = convert_to_single_channel(image, width, height, channels);
	if (!single_channel_image) {
		stbi_image_free(image);
		return 1;
	}

	// Save the single-channel image
	if (!stbi_write_png("assets/test_screen_grayscale.png", width, height, 1, single_channel_image, width)) {
		printf("Failed to save the grayscale image.\n");
		free(single_channel_image);
		stbi_image_free(image);
		return 1;
	}

	printf("Grayscale image saved as assets/test_screen_grayscale.png.\n");

	// Divide and save the single-channel image
	divide_single_channel_image_to_columns(single_channel_image, width, height);

	// Free the image memory
	free(single_channel_image);
	stbi_image_free(image);

	printf("Work done!\n");

	return 0;
}