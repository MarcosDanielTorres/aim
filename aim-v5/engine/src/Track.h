#pragma once

namespace Track {
#define HEADER_MAX_SIZE 40

	using namespace aim::Components;

	struct Track {
		Line3D* left_lines;
		Line3D* right_lines;
	};

	int allocated_size = -1;
	void* load_track(const char* path) {
		void* result = nullptr;
		HANDLE file_handle = CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file_handle == INVALID_HANDLE_VALUE) {
			AIM_FATAL("Failed to load file: %s\n", path);
			return result;
		}
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size) != 0) {
			AIM_DEBUG("File size is: %d\n", file_size.QuadPart);
			void* result = VirtualAlloc(0, file_size.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result) {
				DWORD bytes_read;
				if (ReadFile(file_handle, result, file_size.QuadPart, &bytes_read, 0) && file_size.QuadPart == bytes_read) {
					AIM_DEBUG("File read successfully!\n");
					allocated_size = file_size.QuadPart;
					return result;
				}
			}
		}
		else {
			AIM_FATAL("Failed to get the file size\n");
			return result;
		}
	}


	void process_track(void* track_data) {
		AIM_DEBUG("Processing track\n");

		int i = 0;
		int first_row_cols_count = 0;
		int cols_count = 0;
		char* start_ptr = nullptr;

		char curr_char = *((char*)track_data);
		while (curr_char != '\n' && i < HEADER_MAX_SIZE) {
			curr_char = *((char*)track_data + i);
			if (curr_char == ',') first_row_cols_count++;
			if (curr_char == '\n') {
				cols_count = first_row_cols_count + 1;
				//start_ptr = (float*)((char*)track_data + i + 1);
				start_ptr = (char*)track_data + i + 1;
			}
			//track_data = (char*) track_data + 1;
			i++;
		}

		if (i == HEADER_MAX_SIZE) {
			AIM_FATAL("TOO MUCH COLUMNS BUDDY!");
		}

		int total_csv_lines = 0;
		i = 0;
		curr_char = *((char*)track_data);
		while (curr_char != '\0') {
			if (curr_char == '\n') total_csv_lines++;
			curr_char = *((char*)track_data + i);
			i++;
		}

		int* data;
		VirtualAlloc(data, allocated_size - cols_count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);


		Track track = {};
		track.left_lines = new Line3D[total_csv_lines]{};
		track.right_lines = new Line3D[total_csv_lines]{};

		int line_index = 0;

		int num_bytes = 0;
		while (i != '\0') {
			if (*(start_ptr) != ',') {
				num_bytes++;
			}
			if (*(start_ptr) == ',') {
				track.left_lines[line_index] = Line3D{ .start = glm::vec3(), .end = glm::vec3() };
				float num = *(float*)start_ptr[num_bytes];
				start_ptr = start_ptr;
				num_bytes = 0;
			}
			start_ptr = start_ptr + 1;
		}



	}

	Track process_track2(void* track_data) {
		AIM_DEBUG("Processing track\n");

		// Skip the header row
		int i = 0;
		char* start_ptr = (char*)track_data;
		while (*start_ptr != '\n') {
			start_ptr++;
		}
		start_ptr++;  // Move to the first data line

		// Count total number of lines in the CSV
		int total_csv_lines = 0;
		char* temp_ptr = start_ptr;
		while (*temp_ptr != '\0') {
			if (*temp_ptr == '\n') {
				total_csv_lines++;
			}
			temp_ptr++;
		}

		Track track = {};
		track.left_lines = new Line3D[total_csv_lines]{};
		track.right_lines = new Line3D[total_csv_lines]{};

		// Start processing each row
		int line_index = 0;
		while (*start_ptr != '\0' && line_index < total_csv_lines) {
			// Extract values from each line
			float x_m, y_m, w_tr_right_m, w_tr_left_m;

			// Parse x_m
			x_m = strtof(start_ptr, &start_ptr);  // strtof moves the pointer to the next character after the number

			// Skip comma
			if (*start_ptr == ',') start_ptr++;

			// Parse y_m
			y_m = strtof(start_ptr, &start_ptr);

			// Skip comma
			if (*start_ptr == ',') start_ptr++;

			// Parse w_tr_right_m
			w_tr_right_m = strtof(start_ptr, &start_ptr);

			// Skip comma
			if (*start_ptr == ',') start_ptr++;

			// Parse w_tr_left_m
			w_tr_left_m = strtof(start_ptr, &start_ptr);

			// Skip newline or move to next line
			if (*start_ptr == '\n') start_ptr++;

			// Store the data into left_lines and right_lines
			track.left_lines[line_index] = Line3D{ .start = glm::vec3(x_m + w_tr_left_m, y_m, 0.0f), .end = glm::vec3(w_tr_left_m, y_m, 0.0f) };
			track.right_lines[line_index] = Line3D{ .start = glm::vec3(x_m, y_m, 0.0f), .end = glm::vec3(w_tr_right_m, y_m, 0.0f) };

			line_index++;
		}

		AIM_DEBUG("Processed %d lines.\n", total_csv_lines);
		return track;
	}


	Track process_track3(void* track_data) {
		printf("Processing track\n");

		// Skip the header row
		int i = 0;
		char* start_ptr = (char*)track_data;
		while (*start_ptr != '\n') {
			start_ptr++;
		}
		start_ptr++;  // Move to the first data line

		// Count total number of lines in the CSV
		int total_csv_lines = 0;
		char* temp_ptr = start_ptr;
		while (*temp_ptr != '\0') {
			if (*temp_ptr == '\n') {
				total_csv_lines++;
			}
			temp_ptr++;
		}

		Track track = {};
		track.left_lines = new Line3D[total_csv_lines]{};
		track.right_lines = new Line3D[total_csv_lines]{};

		// Variables to store current and next point
		float x_m, y_m, w_tr_right_m, w_tr_left_m;
		float x_m_next, y_m_next;

		// Start processing each row
		int line_index = 0;
		while (*start_ptr != '\0' && line_index < total_csv_lines - 1) {
			// Extract values from each line
			x_m = strtof(start_ptr, &start_ptr);  // Parse x_m
			if (*start_ptr == ',') start_ptr++;   // Skip comma
			y_m = strtof(start_ptr, &start_ptr);  // Parse y_m
			if (*start_ptr == ',') start_ptr++;   // Skip comma
			w_tr_right_m = strtof(start_ptr, &start_ptr);  // Parse w_tr_right_m
			if (*start_ptr == ',') start_ptr++;   // Skip comma
			w_tr_left_m = strtof(start_ptr, &start_ptr);  // Parse w_tr_left_m
			if (*start_ptr == '\n') start_ptr++;  // Skip newline or move to next line

			// Read the next point to calculate the tangent (center line direction)
			x_m_next = strtof(start_ptr, &start_ptr);  // Peek ahead to the next point's x_m
			if (*start_ptr == ',') start_ptr++;
			y_m_next = strtof(start_ptr, &start_ptr);  // Peek ahead to the next point's y_m

			// Calculate the tangent vector between the current and next points
			float dx = x_m_next - x_m;
			float dy = y_m_next - y_m;
			float length = std::sqrt(dx * dx + dy * dy);

			// Normalize the perpendicular vector
			float perp_x = -dy / length;
			float perp_y = dx / length;

			// Compute left and right points using perpendicular vector
			glm::vec3 right_start = glm::vec3(x_m + w_tr_right_m * perp_x, y_m + w_tr_right_m * perp_y, 0.0f);
			glm::vec3 left_start = glm::vec3(x_m - w_tr_left_m * perp_x, y_m - w_tr_left_m * perp_y, 0.0f);

			// Store the calculated lines in the Track structure
			track.right_lines[line_index] = Line3D{ .start = right_start, .end = glm::vec3(x_m_next, y_m_next, 0.0f) };
			track.left_lines[line_index] = Line3D{ .start = left_start, .end = glm::vec3(x_m_next, y_m_next, 0.0f) };

			// Increment line index
			line_index++;
		}

		printf("Processed %d lines.\n", total_csv_lines);
		return track;
	}
}