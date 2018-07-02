/* Copyright(C) 2018 Michael Fabian Dirks <info@xaymar.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../../../source/os/windows/named-pipe.hpp"
#include "../../../source/os/windows/semaphore.hpp"
#include <thread>
#include "../../common.hpp"

static const size_t max_messages = 1000;

void reader_thread(os::windows::semaphore* sin, os::windows::semaphore* out, os::windows::named_pipe* pipe) {
	std::unique_ptr<os::windows::async_request> read_request;
	std::unique_ptr<os::windows::async_request> write_request;
	std::vector<char> read_buffer;
	std::vector<char> write_buffer;

	size_t count = 0;
	while (count < max_messages) {
		if (!read_request || !read_request->is_valid()) {
			if (sin->wait(std::chrono::milliseconds(1)) == os::error::Success) {
				size_t avail;
				if (pipe->available(avail) == os::error::Success) {
					read_buffer.resize(avail);
				} else {
					throw std::exception("unexpected error");
				}
				if (pipe->read(read_request, read_buffer.data(), read_buffer.size()) != os::error::Success) {
					throw std::exception("unexpected error");
				}
			}
		} else {
			if (read_request->is_complete() || (read_request->wait(std::chrono::milliseconds(1)) == os::error::Success)) {
				if (!write_request || !write_request->is_valid()) {
					write_buffer.swap(read_buffer);
					if (pipe->write(write_request, write_buffer.data(), write_buffer.size()) != os::error::Success) {
						throw std::exception("unexpected error");
					}
					read_request->invalidate();
				}
			}
		}

		if (write_request && write_request->is_valid()) {
			if (write_request->is_complete() || (write_request->wait(std::chrono::milliseconds(1)) == os::error::Success)) {
				count++;
				write_request->invalidate();
				if (out->signal() != os::error::Success) {
					throw std::exception("unexpected error");
				}
			}
		}
	}
}

void sender_thread(os::windows::semaphore* sin, os::windows::semaphore* out, os::windows::named_pipe* pipe) {
	std::unique_ptr<os::windows::async_request> read_request;
	std::unique_ptr<os::windows::async_request> write_request;
	std::vector<char> read_buffer;
	std::vector<char> write_buffer;

	static const size_t buf_size = 16 * 1024 * 1024;

	write_buffer.resize(buf_size);
	for (size_t idx = 0; idx < buf_size; idx++) {
		write_buffer[idx] = (idx + idx) * idx / (idx - (idx >> 1) + 1);
		Sleep(0);
	}

	shared::logger::log("Beginning message barrage");

	shared::time::measure_timer timer;
	shared::time::measure_timer timer_msg;
	std::unique_ptr<shared::time::measure_timer::instance> tinst;
	shared::time::measure_timer timer_write;
	shared::time::measure_timer timer_read;

	size_t count = 0;
	auto total = timer.track();
	while (count < max_messages) {
		auto wtim = timer_write.track();
		if (!write_request || !write_request->is_valid()) {
			if (pipe->write(write_request, write_buffer.data(), write_buffer.size()) != os::error::Success) {
				throw std::exception("unexpected error");
			}
		} else {
			if (write_request->is_complete() || (write_request->wait(std::chrono::milliseconds(1)) == os::error::Success)) {
				if (out->signal() == os::error::Success) {
					tinst = timer_msg.track();
				} else {
				}
			}
		}
		wtim = nullptr;

		auto rtim = timer_read.track();
		if (!read_request || !read_request->is_valid()) {
			if (sin->wait(std::chrono::milliseconds(1)) == os::error::Success) {
				size_t avail;
				if (pipe->available(avail) == os::error::Success) {
					read_buffer.resize(avail);
				} else {
					throw std::exception("unexpected error");
				}
				if (pipe->read(read_request, read_buffer.data(), read_buffer.size()) != os::error::Success) {
					throw std::exception("unexpected error");
				}
				write_request->invalidate();
			}
		} else {
			if (read_request->is_complete() || (read_request->wait(std::chrono::milliseconds(1)) == os::error::Success)) {
				read_request->invalidate();
				count++;
				tinst = nullptr;
			}
		}
		rtim = nullptr;
	}
	total = nullptr;

	shared::logger::log("Read Time:  %llu ns", timer_read.total().count());
	shared::logger::log("Write Time: %llu ns", timer_write.total().count());
	shared::logger::log("Idle Time: %llu ns", timer.total().count() - timer_msg.total().count());
	shared::logger::log("Calls: %llu", timer_msg.count());
	shared::logger::log("Total Time:        %16llu ns", timer_msg.total().count());
	shared::logger::log("Average Time:      %16f ns", timer_msg.average());
	shared::logger::log(" 0.1th Percentile: %16llu ns", timer_msg.percentile(0.001).count());
	shared::logger::log(" 1.0th Percentile: %16llu ns", timer_msg.percentile(0.01).count());
	shared::logger::log("10.0th Percentile: %16llu ns", timer_msg.percentile(0.10).count());
	shared::logger::log("50.0th Percentile: %16llu ns", timer_msg.percentile(0.50).count());
	shared::logger::log("90.0th Percentile: %16llu ns", timer_msg.percentile(0.90).count());
	shared::logger::log("99.0th Percentile: %16llu ns", timer_msg.percentile(0.99).count());
	shared::logger::log("99.9th Percentile: %16llu ns", timer_msg.percentile(0.999).count());
}

int main(int argc, const char* argv[]) {
	shared::logger::is_timestamp_relative_to_start(true);
	shared::logger::to_stdout(true);
	shared::logger::to_stderr(false);
	shared::logger::to_debug(false);
	shared::logger::log("Program Start");

	try {
		os::windows::named_pipe left_pipe = os::windows::named_pipe(os::create_only, "Data", 255, os::windows::pipe_type::Message, os::windows::pipe_read_mode::Message, true);
		os::windows::semaphore left_in = os::windows::semaphore(os::create_only, "DataRight", 0, 100);
		os::windows::semaphore left_out = os::windows::semaphore(os::create_only, "DataLeft", 0, 100);
		
		os::windows::named_pipe right_pipe = os::windows::named_pipe(os::open_only, "Data");
		os::windows::semaphore right_in = os::windows::semaphore(os::open_only, "DataLeft");
		os::windows::semaphore right_out = os::windows::semaphore(os::open_only, "DataRight");
			
		std::thread left, right;

		left = std::thread(reader_thread, &left_in, &left_out, &left_pipe);
		right = std::thread(sender_thread, &right_in, &right_out, &right_pipe);

		if (left.joinable())
			left.join();

		if (right.joinable())
			right.join();
	} catch (std::exception& e) {
		shared::logger::log("%s", e.what());
		throw e;
	}

	std::cin.get();

	return 0;
}