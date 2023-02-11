#include <ac_channel.h>
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
#pragma hls_pipeline_init_interval 1
#pragma hls_pipeline_latency min = MIN_LATENCY

	pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE];
	pixel_type accum;
	int row, col;
	int i, j, idx;
	int r, c;
	int window[WINDOW_SIZE][WINDOW_SIZE];

	row = 0;
	col = 0;
	while (true) {
		pixel_type in_pixel = input.read();

		// Update the line buffer
		for (int i = LINE_BUFFER_SIZE - 1; i > 0; i--) {
			for (int j = 0; j < WINDOW_SIZE; j++) {
				line_buffer[i][j] = line_buffer[i - 1][j];
			}
		}
		line_buffer[0][col] = in_pixel;

		// Load the current window
		idx = 0;
		for (i = row; i < row + WINDOW_SIZE; i++) {
			for (j = col; j < col + WINDOW_SIZE; j++) {
				if (i >= LINE_BUFFER_SIZE) {
					// Load the next line into the line buffer
					line_buffer[i - LINE_BUFFER_SIZE][j] = input.read();
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
			}
		}

		// Perform the convolution
		accum = 0;
		for (i = 0; i < FILTER_HEIGHT; i++) {
			for (j = 0; j < FILTER_WIDTH; j++) {
				accum += window[i][j] * filter[i][j];
			}
		}

		// Write the output pixel
		output.write(accum);

		// Update the column and row indices
		col++;
		if (col == WINDOW_SIZE) {
			col = 0;
			row++;
			if (row == WINDOW_SIZE) {
				row = 0;
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




