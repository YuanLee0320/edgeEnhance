#include <ac_channel.h>
#include <ac_int.h>
#include <iostream>

#define WINDOW_SIZE 5
#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3
#define LINE_BUFFER_SIZE WINDOW_SIZE + FILTER_HEIGHT - 1
#define PIXEL_WIDTH 8
#define PIXEL_TYPE ac_int<PIXEL_WIDTH, false>

const int filter[FILTER_HEIGHT][FILTER_WIDTH] = {
	{-1, -1, -1},
	{-1, 8, -1},
	{-1, -1, -1}
};

// Load the next line into the line buffer
void load_line(ac_channel<pixel_type> &input, pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE]) {
	for (int i = 0; i < WINDOW_SIZE; i++) {
		for (int j = 0; j < LINE_BUFFER_SIZE - WINDOW_SIZE; j++) {
			line_buffer[j][i] = line_buffer[j + WINDOW_SIZE][i];
		}
		for (int j = LINE_BUFFER_SIZE - WINDOW_SIZE; j < LINE_BUFFER_SIZE; j++) {
			line_buffer[j][i] = input.read();
		}
	}
}

void edge_enhancement(ac_channel<pixel_type> &input, ac_channel<pixel_type> &output) {
	pixel_type window[WINDOW_SIZE][WINDOW_SIZE];
	pixel_type line_buffer[LINE_BUFFER_SIZE][WINDOW_SIZE];
	pixel_type accum;
	int row, col;
	int i, j, idx;

	while (true) {
		// Read in the first line of pixels
		for (i = 0; i < LINE_BUFFER_SIZE; i++) {
			for (j = 0; j < WINDOW_SIZE; j++) {
				line_buffer[i][j] = input.read();
			}
		}

		row = 0;
		col = 0;
		while (row < LINE_BUFFER_SIZE) {
			idx = 0;
			// Load the current window
			for (i = row; i < row + WINDOW_SIZE; i++) {
				for (j = 0; j < WINDOW_SIZE; j++) {
					window[idx / WINDOW_SIZE][idx % WINDOW_SIZE] = line_buffer[i][j];
					idx++;
				}
			}

			// Perform edge enhancement if the window is fully populated
			if (idx >= WINDOW_SIZE * WINDOW_SIZE) {
				accum = 0;
				for (i = 0; i < WINDOW_SIZE; i++) {
					for (j = 0; j < WINDOW_SIZE; j++) {
						accum += window[i][j] * filter[i][j];
					}
				}
				line_buffer[row + FILTER_HEIGHT / 2][col + FILTER_WIDTH / 2] = accum;
				output.write(accum);
			}
			col++;
			if (col == WINDOW_SIZE) {
				col = 0;
				row++;
				if (row == LINE_BUFFER_SIZE - WINDOW_SIZE + 1) {
					load_line(input, line_buffer);
					row = 0;
				}
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



