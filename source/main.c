#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../headers/stb_image_write.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_ASCII 123
#define MATRIX_ROWS 16
#define MATRIX_COLS 12

const int row_height = 18;

// Global storage for ASCII character matrices
char ascii_matrices[MAX_ASCII][MATRIX_ROWS][MATRIX_COLS + 1] = {{{0}}};
int ascii_matrix_widths[MAX_ASCII] = {0};

// Function to load ASCII matrices from file
void load_ascii_matrices(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Error opening file");
		return;
	}

	char line[64];
	int ascii_code = -1;
	int row = 0;

	while (fgets(line, sizeof(line), file)) {
		// Remove trailing newline
		line[strcspn(line, "\r\n")] = 0;

		if (strncmp(line, "ASCII", 5) == 0) {
			sscanf(line, "ASCII %d:", &ascii_code);
			row = 0; // Reset row counter
		} else if (ascii_code >= 0 && isdigit(line[0])) {
			if (row < MATRIX_ROWS) {
				// Copy only up to MATRIX_COLS characters
				strncpy(ascii_matrices[ascii_code][row], line, MATRIX_COLS);
				ascii_matrices[ascii_code][row][MATRIX_COLS] = '\0';

				// Update matrix width if needed
				int width = strlen(line);
				if (width > ascii_matrix_widths[ascii_code]) {
					ascii_matrix_widths[ascii_code] = width;
				}
				row++;
			}
		}
	}

	fclose(file);
}

// Function to print a given character matrix
void print_character_matrix(unsigned char *character, int char_width, int cropped_height) {
	printf("First character:\n");
	for (int row = 0; row < cropped_height; row++) {
		for (int col = 0; col < char_width; col++) {
			printf("%c", character[row * char_width + col] == 255 ? '1' : '0');
		}
		printf("\n");
	}
	printf("\n");
}

// Function to compare a given character matrix with stored ASCII matrices
char match_character(unsigned char *character, int char_width, int cropped_height) {
	for (int ascii_code = 0; ascii_code < MAX_ASCII; ascii_code++) {
		int matrix_width = ascii_matrix_widths[ascii_code];
		if (matrix_width == 0)
			continue; // Skip uninitialized matrices

		// Compare using the smaller width
		int min_width = (matrix_width < char_width) ? matrix_width : char_width;
		int match = 1;

		for (int row = 0; row < cropped_height && match; row++) {
			if (row >= MATRIX_ROWS)
				break; // Avoid accessing beyond stored rows

			for (int col = 0; col < min_width; col++) {
				char expected = ascii_matrices[ascii_code][row][col];
				unsigned char actual = character[row * char_width + col];

				if ((expected == '1' && actual != 255) || (expected == '0' && actual != 0)) {
					match = 0;
					break;
				}
			}
		}

		if (match) {
			// printf("Matched ASCII character: %c (Width: %d, Input Width: %d)\n", (char)ascii_code, matrix_width, char_width);
			return (char)ascii_code;
		}
	}

	// Debugging output
	printf("No matching ASCII character found. Input Width: %d\n", char_width);
	print_character_matrix(character, char_width, cropped_height);
	return '?';
}

// Function to write matched character to output.txt
void write_character_to_file(char matched_char) {
	FILE *file = fopen("output.txt", "a");
	if (!file) {
		perror("Error opening output file");
		return;
	}
	fprintf(file, "%c", matched_char);
	fclose(file);
}

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

// Function to determine if a column is completely black
bool is_column_black(unsigned char *row, int column, int width, int cropped_height) {
	for (int y = 0; y < cropped_height; y++) {
		if (row[y * width + column] != 0) {
			return false;
		}
	}
	return true;
}

// Function to determine if a column has any white pixels
bool has_white_pixel(unsigned char *row, int column, int width, int cropped_height) {
	for (int y = 0; y < cropped_height; y++) {
		if (row[y * width + column] == 255) {
			return true;
		}
	}
	return false;
}

int character_index = 0;

// Function to save a character as a PNG file
void save_character_as_png(unsigned char *character, int char_width, int char_height) {
	char filename[256];
	snprintf(filename, sizeof(filename), "assets/char_%d.png", character_index);
	if (!stbi_write_png(filename, char_width, char_height, 1, character, char_width)) {
		printf("Failed to save image: %s\n", filename);
	} else {
		// printf("Saved character as %s\n", filename);
		character_index++;
	}
}

// Main function to process the row
void extract_characters(unsigned char *cropped_row, int cropped_width, int cropped_height) {
	int start_col = -1;
	int space_count = 0;

	for (int col = 0; col < cropped_width; col++) {
		if (is_column_black(cropped_row, col, cropped_width, cropped_height)) {
			space_count++;
			if (space_count == 8) {
				start_col = col - 8;
			}
		} else {
			space_count = 0;
		}

		if (has_white_pixel(cropped_row, col, cropped_width, cropped_height)) {
			if (start_col == -1) {
				start_col = col;
			}
		} else {
			if (start_col != -1) {
				int end_col = col - 1;
				int char_width = end_col - start_col + 3; // Include 1-pixel black borders

				// Allocate memory for the character
				unsigned char *character = (unsigned char *)malloc(char_width * cropped_height);
				if (!character) {
					printf("Memory allocation failed for character extraction\n");
					exit(EXIT_FAILURE);
				}

				// Copy the character data, including 1-pixel black borders
				for (int y = 0; y < cropped_height; y++) {
					for (int x = -1; x <= char_width - 2; x++) {
						int src_col = start_col + x;
						int dst_col = x + 1;

						if (src_col < 0 || src_col >= cropped_width) {
							character[y * char_width + dst_col] = 0; // Black border
						} else {
							character[y * char_width + dst_col] = cropped_row[y * cropped_width + src_col];
						}
					}
				}

				// Handle spaces (8 black columns in a row)
				if (space_count == 8) {
					space_count = 0;
					for (int i = 0; i < cropped_height * char_width; i++) {

						character[i] = 0;
					}
				}

				char matched_char = match_character(character, char_width, MATRIX_ROWS);
				write_character_to_file(matched_char);

				// Save the character as a PNG file
				save_character_as_png(character, char_width, cropped_height);

				// Free the allocated memory
				free(character);

				start_col = -1;
			}
		}
	}
	write_character_to_file('\n');
}

// Function to divide a column into rows of given height, crop rows, and save them to files
void divide_column_into_rows(unsigned char *column, int width, int height, const char *output_prefix) {
	int num_rows = height / row_height;
	for (int i = 0; i < num_rows; i++) {
		// Extract the current row, skipping the first two rows
		unsigned char *row = column + (i * row_height * width) + (2 * width);
		int effective_height = row_height - 2;
		if (effective_height <= 0)
			continue; // Ensure valid height

		// Find the first and last column containing white pixels (255)
		int first_col = -1, last_col = -1;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < effective_height; y++) {
				if (row[y * width + x] == 255) {
					if (first_col == -1)
						first_col = x;
					last_col = x;
				}
			}
		}

		// If no white pixel found, skip saving this row
		if (first_col == -1 || last_col == -1) {
			write_character_to_file('\n');
			continue;
		}

		// Add a black border by including one column on both sides
		first_col = (first_col > 0) ? first_col - 1 : first_col;
		last_col = (last_col < width - 1) ? last_col + 1 : last_col;

		int cropped_width = last_col - first_col + 1;
		int cropped_height = effective_height;

		unsigned char *cropped_row = (unsigned char *)malloc(cropped_width * cropped_height);

		// Copy the cropped row data
		for (int y = 0; y < cropped_height; y++) {
			memcpy(cropped_row + y * cropped_width, row + y * width + first_col, cropped_width);
		}

		extract_characters(cropped_row, cropped_width, cropped_height);

		// Generate a filename for the row
		char filename[256];
		snprintf(filename, sizeof(filename), "%s_row_%d.png", output_prefix, i);

		// Save the image as a grayscale PNG
		if (!stbi_write_png(filename, cropped_width, cropped_height, 1, cropped_row, cropped_width)) {
			printf("Failed to write PNG: %s\n", filename);
		}

		free(cropped_row);
	}
}

int main() {
	int width, height, channels;

	load_ascii_matrices("ascii_base.txt");

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