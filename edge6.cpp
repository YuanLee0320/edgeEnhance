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

// Load the next pixel into the line buffer
void load_pixel(ac_channel<pixel_type> &input, pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE], int &col) {
	for (int i = 0; i < LINE_BUFFER_SIZE - 1; i++) {
		for (int j = 0; j < WINDOW_SIZE - 1; j++) {
			line_buffer[i][j] = line_buffer[i + 1][j];
		}
	}
	line_buffer[LINE_BUFFER_SIZE - 1][col] = input.read();
	col = (col + 1) % WINDOW_SIZE;
}

void edge_enhancement(ac_channel<pixel_type> &input, ac_channel<pixel_type> &output) {
	pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE];
	pixel_type accum;
	int row, col;
	int i, j, r, c;
	int window[WINDOW_SIZE][WINDOW_SIZE];

	// Read in the first two lines of pixels
	for (int i = 0; i < WINDOW_SIZE; i++) {
		for (int j = 0; j < WINDOW_SIZE; j++) {
			line_buffer[i][j] = input.read();
		}
	}

	row = 0;
	col = 0;
	while (true) {
		// Load the current window
		for (i = 0; i < WINDOW_SIZE; i++) {
			for (j = 0; j < WINDOW_SIZE; j++) {
				r = i + row;
				c = j + col;
				if (r >= LINE_BUFFER_SIZE) {
					r = 2 * LINE_BUFFER_SIZE - r - 1;
				}
				if (c >= WINDOW_SIZE) {
					c = 2 * WINDOW_SIZE - c -
						1;
				}
				window[i][j] = line_buffer[r][c];
			}
		}

		// Perform the edge enhancement on the current window
		accum = 0;
		for (i = 0; i < WINDOW_SIZE; i++) {
			for (j = 0; j < WINDOW_SIZE; j++) {
				accum += window[i][j] * filter[i][j];
			}
		}
		output.write(accum);

		// Load the next pixel into the line buffer
		load_pixel(input, line_buffer, col);

		if (col == 0) {
			row++;
			if (row == LINE_BUFFER_SIZE) {
				row = 0;
			}
		}
	}
}

int main() {
	ac_channel<pixel_type> input;
	ac_channel<pixel_type> output;

	// Populate the input channel with data
	// ...

	edge_enhancement(input, output);

	return 0;
}