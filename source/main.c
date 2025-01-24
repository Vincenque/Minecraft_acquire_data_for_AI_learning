#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int row_height = 18;

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

void divide_single_channel_image_to_columns(unsigned char *image, int width, int height, unsigned char **left_column, unsigned char **right_column, int *final_width,
											int *final_height) {

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
	*left_column = (unsigned char *)malloc(column_width * height * sizeof(unsigned char));
	*right_column = (unsigned char *)malloc(column_width * height * sizeof(unsigned char));

	if (!*left_column || !*right_column) {
		printf("Error: Memory allocation failed.\n");
		free(*left_column);
		free(*right_column);
		return;
	}

	// Copy pixels to the left and right column images
	for (int row = 0; row < height; row++) {
		memcpy(*left_column + row * column_width, image + row * width, column_width);
		memcpy(*right_column + row * column_width, image + row * width + column_width, column_width);
	}

	*final_width = column_width;
	*final_height = height;

	printf("Image successfully divided into columns.\n");
}

// Function to divide a column into rows of given height, crop rows, and save them to files
void divide_column_into_rows(unsigned char *column, int width, int height, const char *output_prefix) {
	int num_rows = height / row_height;
	for (int i = 0; i < num_rows; i++) {
		// Extract the current row
		unsigned char *row = column + (i * row_height * width);

		// Find the first and last column containing white pixels (255)
		int first_col = -1, last_col = -1;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < row_height; y++) {
				if (row[y * width + x] == 255) {
					if (first_col == -1)
						first_col = x;
					last_col = x;
				}
			}
		}

		// If no white pixel found, skip saving this row
		if (first_col == -1 || last_col == -1)
			continue;

		// Add a black border by including one column on both sides
		first_col = (first_col > 0) ? first_col - 1 : first_col;
		last_col = (last_col < width - 1) ? last_col + 1 : last_col;

		int cropped_width = last_col - first_col + 1;
		unsigned char *cropped_row = (unsigned char *)malloc(cropped_width * row_height);

		// Copy the cropped row data
		for (int y = 0; y < row_height; y++) {
			memcpy(cropped_row + y * cropped_width, row + y * width + first_col, cropped_width);
		}

		// Generate a filename for the row
		char filename[256];
		snprintf(filename, sizeof(filename), "%s_row_%d.png", output_prefix, i);

		// Save the image as a grayscale PNG
		if (!stbi_write_png(filename, cropped_width, row_height, 1, cropped_row, cropped_width)) {
			fprintf(stderr, "Failed to write PNG: %s\n", filename);
		}

		free(cropped_row);
	}
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
	unsigned char *left_column = NULL;
	unsigned char *right_column = NULL;
	int final_width = 0;
	int final_height = 0;

	divide_single_channel_image_to_columns(single_channel_image, width, height, &left_column, &right_column, &final_width, &final_height);

	if (left_column && right_column) {
		printf("Final Width: %d, Final Height: %d\n", final_width, final_height);

		// Save the images to files in the assets folder
		if (stbi_write_png("assets/left_column_image.png", final_width, final_height, 1, left_column, final_width) == 0) {
			printf("Error: Could not save left column image.\n");
		} else {
			printf("Left column image saved to assets/left_column_image.png\n");
		}

		if (stbi_write_png("assets/right_column_image.png", final_width, final_height, 1, right_column, final_width) == 0) {
			printf("Error: Could not save right column image.\n");
		} else {
			printf("Right column image saved to assets/right_column_image.png\n");
		}

		// Divide and save rows for left and right columns
		divide_column_into_rows(left_column, final_width, final_height, "assets/left_column");
		divide_column_into_rows(right_column, final_width, final_height, "assets/right_column");

		// Free allocated memory for the columns
		free(left_column);
		free(right_column);
	} else {
		printf("Error: Could not divide the image into columns.\n");
	}

	// Free the single-channel image
	free(single_channel_image);
	stbi_image_free(image);

	return 0;
}