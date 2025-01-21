#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to convert RGB image to single-channel grayscale
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
		int r = image[i * channels];
		int g = image[i * channels + 1];
		int b = image[i * channels + 2];

		// Compute the grayscale value: grayscale = 0.3*R + 0.59*G + 0.11*B
		single_channel_image[i] = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
	}

	return single_channel_image;
}

// Function to filter an image based on the condition (R, G, B) = (221, 221, 221)
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
			image[i * channels] = 255; // Set to white
			image[i * channels + 1] = 255;
			image[i * channels + 2] = 255;
		} else {
			image[i * channels] = 0; // Set to black
			image[i * channels + 1] = 0;
			image[i * channels + 2] = 0;
		}
	}
}

void divide_and_save_image(unsigned char *image, int width, int height, int channels) {
	if (channels != 1) {
		printf("Error: Image must have 1 channel (grayscale values).\n");
		return;
	}

	// Skip the first two rows
	int row_size = width;
	unsigned char *image_data = image + (2 * row_size);
	int adjusted_height = height - 2; // New height after skipping two rows

	// Divide the image into two columns
	int half_width = width / 2;

	// Temporary buffer for each small part
	unsigned char *buffer = (unsigned char *)malloc(half_width * 18);
	if (!buffer) {
		printf("Failed to allocate memory for buffer.\n");
		return;
	}

	// Iterate over the two columns
	for (int col = 0; col < 2; col++) {
		unsigned char *column_data = image_data + (col * half_width);

		// Divide the column into rows of height 18
		for (int row = 0; row < adjusted_height / 18; row++) {
			int row_start = row * 18 * row_size;

			// Copy the 18-row segment to the buffer
			for (int i = 0; i < 18; i++) {
				memcpy(buffer + (i * half_width), column_data + row_start + (i * row_size), half_width);
			}

			// Save the part to a new PNG file in the assets directory
			char output_filename[256];
			snprintf(output_filename, sizeof(output_filename), "assets/output_col%d_row%d.png", col, row);
			if (!stbi_write_png(output_filename, half_width, 18, 1, buffer, half_width)) {
				printf("Failed to save image part: %s\n", output_filename);
			} else {
				printf("Saved: %s\n", output_filename);
			}
		}
	}

	// Free resources
	free(buffer);
	printf("Image processing completed.\n");
}

int main() {
	int width, height, channels;

	// Load the image
	unsigned char *image = stbi_load("assets/test_screen.png", &width, &height, &channels, 0);
	if (!image) {
		printf("Failed to load the image.\n");
		return 1;
	}

	// Apply the filter to the image
	filter_image(image, width, height, channels);

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
	divide_and_save_image(single_channel_image, width, height, 1);

	// Free the image memory
	free(single_channel_image);
	stbi_image_free(image);

	printf("Work done!\n");

	return 0;
}