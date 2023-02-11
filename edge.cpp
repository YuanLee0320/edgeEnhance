void edge_enhancement(ac_channel<int> &input, ac_channel<int> &output, int &row, int &col) {
	static int buffer[ROWS][COLS + 4];
	static int window[5][5];
	static int accum = 0;
	static int idx = 0;
	static bool end_of_line = false;
	static bool end_of_frame = false;

	// 5x5 filter coefficients
	int filter[5][5] = { {-1, -1, -1, -1, -1},
						{-1, -1, -1, -1, -1},
						{-1, -1, 25, -1, -1},
						{-1, -1, -1, -1, -1},
						{-1, -1, -1, -1, -1} };

	while (true) {
		// Store the current input in the buffer
		buffer[row][2 + col] = input.read();

		// Shift the window to the right
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 4; j++) {
				window[i][j] = window[i][j + 1];
			}
		}

		// Store the new pixel in the rightmost column of the window
		window[2][4] = buffer[row][2 + col];

		// Check if the buffer is full
		if (idx >= 2 && idx <= ROWS + 1) {
			// Convolve the filter with the input image
			for (int m = 0; m < 5; m++) {
				for (int n = 0; n < 5; n++) {
					int ii = row + m - 2;
					int jj = col + n - 2;

					// Mirror the input at the boundary
					if (ii < 0) ii = -ii;
					if (ii >= ROWS) ii = 2 * (ROWS - 1) - ii;
					if (jj < 0) jj = -jj;
					if (jj >= COLS) jj = 2 * (COLS - 1) - jj;

					accum += buffer[ii][jj + 2] * filter[m][n];
				}
			}
			output.write(accum / 25);
			accum = 0;
		}

		// Increment the index
		idx++;

		// Check if the end of line is reached
		if (col == COLS - 1) {
			end_of_line = true;
		}

		// Check if the end of frame is reached
		if (end_of_line && row == ROWS - 1) {
			end_of_frame = true;
		}

		// Reset the buffer and window when the end of frame is reached
		if (end_of_frame) {
			for (int i = 0; i < ROWS; i++) {
				for (int j = 0; j < COLS + 4; j++) {
					buffer[i][j] = 0;
				}
			}
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					window[i][j] = 0;
				}
			}
			accum = 0;
			idx = 0;
			end_of_line = false;
			end_of_frame = false;
			row = 0;
			col = 0;
		}

		// Update the row and column indices
		if (!end_of_line) {
			col++;
		}
		else {
			col = 0;
			row++;
		}
	}
}
