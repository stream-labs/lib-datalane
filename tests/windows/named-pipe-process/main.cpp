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

#include <string>
#include <thread>
#include "../../../source/os/windows/named-pipe.hpp"
#include "../../../source/os/windows/semaphore.hpp"
#include "../../common.hpp"

#define SERVER_CLIENT_CHANNEL "DataSend"
#define CLIENT_SERVER_CHANNEL "DataRecv"
#define CLIENT_MAX_MESSAGES 10000

std::vector<std::string> messages = {
	"This is a test message",
	"It is pitch black. You are likely to be eaten by a grue.",
	"If you see this, things are working",
	"Have I told you that this works yet?",
	"Would you like to hear about our lord and savior IPC?",
	"0xCabba9e5",
	"Is your refrigerator running?",
	"0xC01DB007",
	"In front of you is a terminal. The terminal is displaying words.",
	"These are just test messages, why are you reading these?",
	"0x08008135",
	"Computer said no.",
	"Ain't no data where we are going, cause we don't need no data.",
	"The end of the world is simply determined by how good the programming on the simulation is. And the simulation "
	"was done by the lowest bidder.",
	"0xDeadBeef",
	"Virtual Reality is great, full immersion is better.",
	"Looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
	"oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
	"oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
	"ooooooooooooooooooooooooooooooooooooooooooooong "
	"Spaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaam"};

int server(int argc, const char *argv[]) {
	os::windows::named_pipe pipe = os::windows::named_pipe(
		os::create_only, "Data", 255, os::windows::pipe_type::Message, os::windows::pipe_read_mode::Message, true);
	os::windows::semaphore sin  = os::windows::semaphore(os::create_only, CLIENT_SERVER_CHANNEL);
	os::windows::semaphore sout = os::windows::semaphore(os::create_only, SERVER_CLIENT_CHANNEL);

	std::unique_ptr<os::windows::async_request> read_request, write_request, accept_request;
	std::vector<char>                           read_buffer, write_buffer;
	read_buffer.reserve(65535);
	write_buffer.reserve(65535);

	shared::logger::log("Server initialized.");

	// Spawn client process
	shared::os::process client(std::string(argv[0]), std::string(argv[0]) + " client",
							   shared::os::get_working_directory());
	shared::logger::log("Client spawned.");

	bool breakout = false;
	while (!breakout) {
		os::error ec = pipe.accept(accept_request);
		switch (ec) {
		case os::error::Connected:
			breakout = true;
			break;
		case os::error::Pending:
			break;
		case os::error::Success:
			if (accept_request->wait(std::chrono::milliseconds(5000)) == os::error::TimedOut) {
				throw std::exception("timed out waiting for client");
			}
		default:
			throw std::exception("unexpected error");
		}
	}

	write_buffer.resize(2);
	write_buffer[0] = 'G';
	write_buffer[1] = 'O';
	if (pipe.write(write_request, write_buffer.data(), write_buffer.size()) != os::error::Success) {
		throw std::exception("Failed to write to pipe.");
	}
	shared::logger::log("Writing signal..");
	if (!write_request->is_complete()) {
		if (write_request->wait() != os::error::Success) {
			throw std::exception("unexpected error");
		}
	}
	shared::logger::log("Signal written.");
	if (sout.signal() != os::error::Success) {
		throw std::exception("unexpected error");
	}
	shared::logger::log("Client signalled.");
	write_request->invalidate();

	std::vector<os::waitable *> waits;
	waits.reserve(4);
	while (pipe.is_connected()) {
		waits.clear();
		waits.push_back(&sin);

		while (waits.size() > 0) {
			size_t signalled_index = -1;
			for (size_t idx = 0; idx < waits.size(); idx++) {
				if (waits[idx]->wait(std::chrono::milliseconds(0)) == os::error::Success) {
					signalled_index = idx;
					break;
				}
			}
			if (signalled_index == -1) {
				os::error code = os::waitable::wait_any(waits, signalled_index, std::chrono::milliseconds(100));
				if (code == os::error::TimedOut) {
					break;
				} else if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("unexpected error");
				}
			}

			if (waits[signalled_index] == &sin) {
				if (!read_request || !read_request->is_valid()) {
					size_t avail;
					if (pipe.available(avail) == os::error::Success) {
						if (avail > 0) {
							read_buffer.resize(avail);
							if (pipe.read(read_request, read_buffer.data(), read_buffer.size()) != os::error::Success) {
								throw std::exception("unexpected error");
							}
							waits.push_back(read_request.get());
						} else {
							shared::logger::log("signalled with no data");
						}
					} else {
						throw std::exception("unexpected error");
					}
				} else {
					sin.signal(); // Re-increment.
				}
			} else if (waits[signalled_index] == read_request.get()) {
				if (read_request->is_valid() && read_request->is_complete() && !write_request->is_valid()) {
					write_buffer.resize(read_request->get_bytes_transferred());
					memcpy(write_buffer.data(), read_buffer.data(), write_buffer.size());

					if (pipe.write(write_request, write_buffer.data(), write_buffer.size()) != os::error::Success) {
						throw std::exception("unexpected error");
					}
					waits.push_back(write_request.get());
					read_request->invalidate();

					//shared::logger::log("Client '%.*s'", read_buffer.size(), read_buffer.data());
				}
			} else if (waits[signalled_index] == write_request.get()) {
				if (write_request->is_valid() && write_request->is_complete()) {
					if (sout.signal() == os::error::Success) {
						write_request->invalidate();
					}
				}
			}

			if (waits.size() > 1) {
				std::vector<os::waitable *> new_waits(waits.size() - 1);
				for (size_t idx = 0, eidx = waits.size(), tidx = 0; idx < eidx; idx++) {
					if (idx == signalled_index)
						continue;
					new_waits[tidx] = waits[idx];
					tidx++;
				}
				waits.swap(new_waits);
			} else {
				break;
			}
		}
	}

	return 0;
}

int client(int argc, const char *argv[]) {
	os::windows::named_pipe                     pipe = os::windows::named_pipe(os::open_only, "Data");
	os::windows::semaphore                      sin  = os::windows::semaphore(os::open_only, SERVER_CLIENT_CHANNEL);
	os::windows::semaphore                      sout = os::windows::semaphore(os::open_only, CLIENT_SERVER_CHANNEL);
	std::unique_ptr<os::windows::async_request> read_request, write_request;
	std::vector<char>                           read_buffer, write_buffer;
	size_t                                      count = 0, recv_count = 0;
	std::vector<os::waitable *>                 waits;

	read_buffer.reserve(65535);
	write_buffer.reserve(65535);

	shared::logger::log("Client initialized.");

	// Timers
	shared::time::measure_timer go_timer;
	shared::time::measure_timer loop_timer;
	shared::time::measure_timer msg_time;

	// Wait for GO
	auto got = go_timer.track();
	while (pipe.is_connected()) {
		if (!read_request || !read_request->is_valid()) {
			if (sin.wait(std::chrono::milliseconds(100)) == os::error::Success) {
				size_t avail = 0;
				if (pipe.available(avail) != os::error::Success) {
					throw std::exception("unexpected error");
				}
				if (avail > 0) {
					read_buffer.resize(avail);
					if (pipe.read(read_request, read_buffer.data(), read_buffer.size()) != os::error::Success) {
						throw std::exception("unexpected error");
					}
				}
			}
		} else {
			if (read_request->is_complete()
				|| (read_request->wait(std::chrono::milliseconds(100)) == os::error::Success)) {
				if (read_buffer[0] == 'G' && read_buffer[1] == 'O') {
					read_request->invalidate();
					break;
				} else {
					throw std::exception("wrong initial message");
				}
			}
		}
	}
	shared::logger::log("Signal received.");
	got = nullptr;

	auto loopt = loop_timer.track();
	auto msgt  = msg_time.track();
	msgt->cancel();
	while (pipe.is_connected()) {
		if (count < CLIENT_MAX_MESSAGES) {
			if (!write_request || !write_request->is_valid()) {
				// Select a message for each send.
				size_t idx = count % messages.size();
				write_buffer.resize(messages[idx].size());
				memcpy(write_buffer.data(), messages[idx].data(), write_buffer.size());
				os::error code = pipe.write(write_request, write_buffer.data(), write_buffer.size());
				if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("unexpected error");
				}
				count++;
				//shared::logger::log("Sent '%.*s'", write_buffer.size(), write_buffer.data());
				waits.push_back(write_request.get());
				msgt = msg_time.track();
			}
		}

		while (waits.size() > 0) {
			size_t signalled_index = -1;
			for (size_t idx = 0; idx < waits.size(); idx++) {
				if (waits[idx]->wait(std::chrono::milliseconds(0)) == os::error::Success) {
					signalled_index = idx;
					break;
				}
			}
			if (signalled_index == -1) {
				os::error code = os::waitable::wait_any(waits, signalled_index, std::chrono::milliseconds(100));
				if (code == os::error::TimedOut) {
					break;
				} else if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("unexpected error");
				}
			}

			if (waits[signalled_index] == &sin) {
				if (!read_request || !read_request->is_valid()) {
					size_t avail = 0;
					if (pipe.available(avail) == os::error::Success) {
						if (avail > 0) {
							read_buffer.resize(avail);
							if (pipe.read(read_request, read_buffer.data(), read_buffer.size()) != os::error::Success) {
								throw std::exception("unexpected error");
							}
							waits.push_back(read_request.get());
						} else {
							pipe.total_available(avail);
							shared::logger::log("signalled with no data, %lld", avail);
						}
					} else {
						throw std::exception("unexpected error");
					}
				}
			} else if (waits[signalled_index] == read_request.get()) {
				if (read_request->is_valid() && read_request->is_complete()) {
					//shared::logger::log("Received '%.*s'", read_buffer.size(), read_buffer.data());
					read_request->invalidate();
					msgt = nullptr;
					recv_count++;
				}
			} else if (waits[signalled_index] == write_request.get()) {
				if (write_request->is_valid() && write_request->is_complete()) {
					if (sout.signal() == os::error::Success) {
						waits.push_back(&sin);
						write_request->invalidate();
					}
				}
			}

			if (waits.size() > 1) {
				std::vector<os::waitable *> new_waits(waits.size() - 1);
				for (size_t idx = 0, eidx = waits.size(), tidx = 0; idx < eidx; idx++) {
					if (idx == signalled_index)
						continue;
					new_waits[tidx] = waits[idx];
					tidx++;
				}
				waits.swap(new_waits);
			} else {
				break;
			}

			if (recv_count >= CLIENT_MAX_MESSAGES)
				break;
		}

		if (recv_count >= CLIENT_MAX_MESSAGES)
			break;
	}
	loopt = nullptr;

	shared::logger::log("Signal Time: %lld", go_timer.total().count());
	shared::logger::log("Loop Time: %lld", loop_timer.total().count());
	shared::logger::log("Total Messages: %lld", msg_time.count());
	shared::logger::log("Total Time: %16llu ns", msg_time.total().count());
	shared::logger::log("Avrg. Time: %16.0f ns", msg_time.average());
	shared::logger::log("Best  Time: %16llu ns", msg_time.percentile(0, true));
	shared::logger::log("Worst Time: %16llu ns", msg_time.percentile(1, true));
	shared::logger::log("  .1%% Time: %16llu ns", msg_time.percentile(0.001).count());
	shared::logger::log(" 1.0%% Time: %16llu ns", msg_time.percentile(0.01).count());
	shared::logger::log("10.0%% Time: %16llu ns", msg_time.percentile(0.10).count());
	shared::logger::log("25.0%% Time: %16llu ns", msg_time.percentile(0.25).count());
	shared::logger::log("50.0%% Time: %16llu ns", msg_time.percentile(0.5).count());
	shared::logger::log("75.0%% Time: %16llu ns", msg_time.percentile(0.75).count());
	shared::logger::log("90.0%% Time: %16llu ns", msg_time.percentile(0.9).count());
	shared::logger::log("99.0%% Time: %16llu ns", msg_time.percentile(0.99).count());
	shared::logger::log("99.9%% Time: %16llu ns", msg_time.percentile(0.999).count());

	std::cin.get();
	return 0;
}

int main(int argc, const char *argv[]) {
	//try {
	shared::logger::is_timestamp_relative_to_start(true);
	shared::logger::to_stdout(true);
	shared::logger::to_stderr(false);
	shared::logger::to_debug(false);
	shared::logger::log("Program Start");

	int code = 0;
	if (argc == 0 || argc == 1) { // Server
		code = server(argc, argv);
	} else {
		code = client(argc, argv);
	}

	std::cin.get();

	return code;
	//} catch (std::exception e) {
	//	shared::logger::log("%s", e.what());
	//	throw e;
	//}
}