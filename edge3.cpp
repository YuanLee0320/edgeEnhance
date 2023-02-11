#include <ac_channel.h>
#include <ac_int.h>
#include <iostream>

#define WINDOW_SIZE 5
#define FILTER_WIDTH 5
#define FILTER_HEIGHT 5
#define LINE_BUFFER_SIZE WINDOW_SIZE-1
#define PIXEL_WIDTH 8
#define PIXEL_TYPE ac_int<PIXEL_WIDTH, false>

const int filter[FILTER_HEIGHT][FILTER_WIDTH] = {
{-1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1},
{-1, -1, 24, -1, -1},
{-1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1}
};

// Load the next line into the line buffer
void load_line(ac_channel<pixel_type> &input, pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE]) {
	for (int i = 0; i < LINE_BUFFER_SIZE; i++) {
		for (int j = 0; j < WINDOW_SIZE; j++) {
			line_buffer[i][j] = input.read();
		}
	}
}

void edge_enhancement(ac_channel<pixel_type> &input, ac_channel<pixel_type> &output) {
	pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE];
	pixel_type accum;
	pixel_type output_line_buffer[WINDOW_SIZE];
	int row, col;
	int i, j, idx;
	int r, c;
	int window[WINDOW_SIZE][WINDOW_SIZE];


	// Read in the first two lines of pixels
	load_line(input, line_buffer);
	load_line(input, line_buffer);

	while (true) {
		row = 0;
		col = 0;
		while (row < LINE_BUFFER_SIZE) {
			idx = 0;
			// Load the current window
			for (i = row; i < row + WINDOW_SIZE; i++) {
				for (j = col; j < col + WINDOW_SIZE; j++) {
					if (i >= LINE_BUFFER_SIZE) {
						// Load the next line into the line buffer
						load_line(input, line_buffer);
						i = 0;
					}
					r = i - row;
					c = j - col;
					if (i < 0) {
						window[r][c] = line_buffer[-(i + 1)][j];
					}
					else if (i >= LINE_BUFFER_SIZE) {
						window[r][c] = line_buffer[i - LINE_BUFFER_SIZE][j];
					}
					else {
						window[r][c] = line_buffer[i][j];
					}
					idx++;
				}
			}


			// Apply the filter
			accum = 0;
			for (i = 0; i < FILTER_HEIGHT; i++) {
				for (j = 0; j < FILTER_WIDTH; j++) {
					accum += window[i][j] * filter[i][j];
				}
			}

			// Saturate the result to the range [0, 255]
			if (accum > 255) {
				accum = 255;
			}
			else if (accum < 0) {
				accum = 0;
			}

			// Write the result to the output buffer
			output_line_buffer[col] = accum;

			// Move the window to the next column
			col++;
			if (col + WINDOW_SIZE > LINE_BUFFER_SIZE) {
				// Output the current line
				for (i = 0; i < WINDOW_SIZE; i++) {
					output.write(output_line_buffer[i]);
				}
				col = 0;
				row++;
			}
		}
	}
}

int main() {
	ac_channel<pixel_type> input;
	ac_channel<pixel_type> output;

	edge_enhancement(input, output);

	return 0;
}