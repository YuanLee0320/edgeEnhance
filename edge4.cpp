#include <ac_int.h>
#include <iostream>

#define WINDOW_SIZE 5
#define FILTER_WIDTH 5
#define FILTER_HEIGHT 5
#define LINE_BUFFER_SIZE WINDOW_SIZE-1
#define PIXEL_WIDTH 8
#define PIXEL_TYPE ac_int<PIXEL_WIDTH, false>
#define MIN_LATENCY (WINDOW_SIZE + FILTER_WIDTH - 1)

const int filter[FILTER_HEIGHT][FILTER_WIDTH] = {
{-1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1},
{-1, -1, 24, -1, -1},
{-1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1}
};

void edge_enhancement(ac_channel<pixel_type> &input, ac_channel<pixel_type> &output) {
	// Latency stage
#pragma hls_pipeline_init_interval 1
#pragma hls_pipeline_latency min = MIN_LATENCY

	pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE];
	pixel_type accum;
	int row, col;
	int i, j, idx;
	int r, c;
	int window[WINDOW_SIZE][WINDOW_SIZE];

	// Read in the first two lines of pixels
	load_line(input, line_buffer);
	load_line(input, line_buffer);

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

		// Saturate the output pixel value
		if (accum < 0) {
			accum = 0;
		}
		else if (accum > (1 << PIXEL_WIDTH) - 1) {
			accum = (1 << PIXEL_WIDTH) - 1;
		}

		// Write the output pixel value
		output.write(accum);

		// Move the window one column to the right
		col++;
		if (col >= WINDOW_SIZE) {
			// Move the window one row down and reset the column
			row++;
			col = 0;
		}
	}
}

int main() {
	ac_channel<pixel_type> input;
	ac_channel<pixel_type> output;

	// Load the input pixels into the input channel
	for (int i = 0; i < NUM_PIXELS; i++) {
		input.write(input_pixels[i]);
	}

	// Run the edge enhancement function
	edge_enhancement(input, output);

	// Read the output pixels from the output channel
	for (int i = 0; i < NUM_PIXELS; i++) {
		std::cout << output.read() << std::endl;
	}

	return 0;
}