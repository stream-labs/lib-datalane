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

// Thank you windows.h for defining common words as macros.
#define NO_MIN_MAX

#include <limits>
#include <queue>
#include <string>
#include <thread>
#include "../../../source/os/windows/named-pipe.hpp"
#include "../../../source/os/windows/semaphore.hpp"
#include "../../common.hpp"

using namespace std::placeholders;

// Thank you windows.h for defining common words as macros.
#undef max
#undef min

#define SERVER_CLIENT_CHANNEL "DataSend1"
#define CLIENT_SERVER_CHANNEL "DataRecv1"
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

// New functionality by commit 2a6040df7922a077dc9aa16216e4085796637a42
struct server_data {
	os::windows::named_pipe pipe;
	os::windows::semaphore  sin;
	os::windows::semaphore  sout;

	std::shared_ptr<os::async_op>               read_op, write_op;
	std::unique_ptr<os::windows::async_request> accept_request;
	std::vector<char>                           read_buf, write_buf;
	std::queue<std::vector<char>>               write_queue;

	size_t count_recv = 0;
	size_t count_send = 0;

	size_t pending_msg = 0;

	server_data()
		: pipe(os::create_only, "Data", 255, os::windows::pipe_type::Message, os::windows::pipe_read_mode::Message,
			   true),
		  sin(os::create_only, CLIENT_SERVER_CHANNEL), sout(os::create_only, SERVER_CLIENT_CHANNEL) {}

	void accept_client() {
		switch (pipe.accept(accept_request)) {
		default:
			throw std::exception("unexpected error");
		case os::error::Connected:
			break;
		case os::error::Success:
			if (os::waitable::wait(accept_request.get(), std::chrono::milliseconds(5000)) == os::error::TimedOut) {
				throw std::exception("timed out waiting for client");
			}
		}
	}

	void signal_client_start_cb(os::error err, size_t bytes) {
		os::error ec = sout.signal();
		if (ec != os::error::Success) {
			throw std::exception("unexpected error");
		}
	}

	void signal_client_start() {
		shared::logger::log("Signalling Client to start...");

		write_buf.resize(2);
		write_buf[0] = 'G';
		write_buf[1] = 'O';

		if (pipe.write(write_buf.data(), write_buf.size(), write_op,
					   std::bind(&server_data::signal_client_start_cb, this, _1, _2))
			!= os::error::Success) {
			throw std::exception("Failed to write to pipe.");
		}

		if (!write_op->is_complete()) {
			if (write_op->wait() != os::error::Success) {
				throw std::exception("unexpected error");
			}
		}
		write_op->invalidate();

		shared::logger::log("Done.");
	}

	void read_cb(os::error err, size_t bytes) {
		read_op->invalidate();

		if (err == os::error::Success) {
			count_recv++;
			//shared::logger::log("Read message %lld of %lld, content '%.*s'.", count_recv, CLIENT_MAX_MESSAGES, bytes - 1, read_buf.data() + 1);
			send_message(read_buf);
		}

		if (pending_msg > 0) {
			read_message();
		}
	}

	void read_message() {
		if (read_op && read_op->is_valid()) {
			return;
		}

		size_t    avail = 0;
		os::error ec;

		while (avail == 0) {
			ec = pipe.available(avail);
			if (ec != os::error::Success) {
				break;
			}
			if (avail == 0) {
				Sleep(0); // Give up time slice.
			}
		}

		read_buf.resize(avail);
		ec = pipe.read(read_buf.data(), avail, read_op, std::bind(&server_data::read_cb, this, _1, _2));
		if (ec != os::error::Success) {
			throw std::exception("unexpected error");
		}

		if (pending_msg > 0) {
			pending_msg--;
		}
	}

	void send_cb(os::error err, size_t bytes) {
		write_op->invalidate();

		if (err == os::error::Success) {
			sout.signal();
			count_send++;
			//shared::logger::log("Sent message %lld of %lld.", count_send, CLIENT_MAX_MESSAGES);

			if (write_queue.size() > 0) {
				write_buf.swap(write_queue.front());
				write_queue.pop();

				err = pipe.write(write_buf.data(), write_buf.size(), write_op,
								 std::bind(&server_data::send_cb, this, _1, _2));
				if (err != os::error::Success) {
					throw std::exception("unexpected error");
				}
			}
		}
	}

	void queue_message(std::vector<char> &buf) {
		write_queue.push(std::vector<char>(buf));
	}

	void send_message(std::vector<char> &buf) {
		os::error err;

		if (write_op && write_op->is_valid()) {
			queue_message(buf);
		}

		write_buf.swap(buf);
		err = pipe.write(write_buf.data(), write_buf.size(), write_op, std::bind(&server_data::send_cb, this, _1, _2));
		if (err != os::error::Success) {
			throw std::exception("unexpected error");
		}
	}

	void loop() {
		os::error ec;

		accept_client();
		while (!pipe.is_connected()) {
			ec = accept_request->wait(std::chrono::milliseconds(1000));
			if (ec == os::error::Success) {
				break;
			} else if (ec == os::error::TimedOut) {
				continue;
			} else {
				shared::logger::log("Failed to wait for client.");
				throw std::exception();
			}
		}

		signal_client_start();

		while (pipe.is_connected()) {
			os::waitable *waits[]    = {&sin, read_op.get(), write_op.get()};
			size_t        wait_index = std::numeric_limits<size_t>::max();
			for (size_t idx = 0; idx < 3; idx++) {
				if (waits[idx] != nullptr && waits[idx]->wait(std::chrono::milliseconds(0)) == os::error::Success) {
					wait_index = idx;
					break;
				}
			}
			if (wait_index == std::numeric_limits<size_t>::max()) {
				os::error code = os::waitable::wait_any(waits, 3, wait_index, std::chrono::milliseconds(1000));
				if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("unexpected error");
				}
			}
			if (wait_index == std::numeric_limits<size_t>::max()) {
				continue;
			}

			if (waits[wait_index] == &sin) {
				if (read_op && read_op->is_valid()) {
					pending_msg++;
				} else {
					read_message();
				}
			}
		}
	}
};

int server(int argc, const char *argv[]) {
	server_data sd;
	shared::logger::log("Server initialized.");
	shared::os::process client(std::string(argv[0]), std::string(argv[0]) + " client",
							   shared::os::get_working_directory());
	shared::logger::log("Client spawned.");
	sd.loop();
	shared::logger::log("Done.");
	return 0;
}

struct client_data {
	os::windows::named_pipe pipe;
	os::windows::semaphore  sin;
	os::windows::semaphore  sout;

	std::shared_ptr<os::async_op> read_op, write_op;
	std::vector<char>             read_buf, write_buf;
	std::queue<std::vector<char>> write_queue;

	bool   is_initialized = false;
	size_t count_send     = 0;
	size_t count_recv     = 0;

	size_t pending_msg = 0;

	shared::time::measure_timer go_timer;
	shared::time::measure_timer loop_timer;
	shared::time::measure_timer msg_time;

	std::unique_ptr<shared::time::measure_timer::instance>             go_time;
	std::queue<std::unique_ptr<shared::time::measure_timer::instance>> msg_times;

	client_data()
		: pipe(os::open_only, "Data", os::windows::pipe_read_mode::Message), sin(os::open_only, SERVER_CLIENT_CHANNEL),
		  sout(os::open_only, CLIENT_SERVER_CHANNEL) {}

	void write_cb(os::error err, size_t bytes) {
		write_op->invalidate();

		if (err == os::error::Success) {
			os::error ec = sout.signal();
			if (ec != os::error::Success) {
				throw std::exception("Could not signal remote.");
			}
			count_send++;
			//shared::logger::log("Sent message %lld of %lld.", count_send, CLIENT_MAX_MESSAGES);
			/*if ((count_send + 1) % 100 == 0)
				shared::logger::log("Sent %lld messages.", count_send);*/
		} else {
			throw std::exception("sending failed");
		}
	}

	void send_message() {
		if (write_op && write_op->is_valid()) {
			return;
		}
		if (count_send >= CLIENT_MAX_MESSAGES) {
			return;
		}

		uint8_t idx      = count_send % messages.size();
		size_t  msg_size = 1 + messages[idx].size();

		write_buf.resize(msg_size);
		write_buf[0] = idx;
		memcpy(reinterpret_cast<void *>(&write_buf[1]), messages[idx].c_str(), messages[idx].size());

		msg_times.push(msg_time.track());

		os::error ec =
			pipe.write(write_buf.data(), msg_size, write_op, std::bind(&client_data::write_cb, this, _1, _2));
		if (ec != os::error::Success) {
			throw std::exception("failed");
		}
	}

	void read_cb(os::error err, size_t bytes) {
		read_op->invalidate();
		msg_times.pop();

		uint8_t idx = (uint8_t &)read_buf[0];
		if (idx >= messages.size()) {
			shared::logger::log("Index %d is out of bounds (maximum %d)", idx, messages.size() - 1);
			throw std::exception("Message corrupted.");
		}

		size_t      msg_len = bytes - 1;
		const char *msg     = (const char *)&read_buf[1];
		if ((msg_len == messages[idx].size()) && (memcmp(msg, messages[idx].c_str(), msg_len) == 0)) {
			count_recv++;
			//shared::logger::log("Read message %lld of %lld.", count_recv, CLIENT_MAX_MESSAGES);
			/*if ((count_recv + 1) % 100 == 0)
					shared::logger::log("Received %lld messages.", count_recv);*/
		} else {
			shared::logger::log("Message does not match original message.\nRecv: %.*s\nOrig: %.*s).", msg_len, msg,
								messages[idx].size(), messages[idx].c_str());
			throw std::exception("Message corrupted.");
		}

		if (pending_msg > 0) {
			read_message();
		}

		send_message();
	}

	void read_message() {
		if (read_op && read_op->is_valid()) {
			pending_msg++;
			return;
		}
		if (count_recv >= CLIENT_MAX_MESSAGES) {
			return;
		}

		size_t    avail = 0;
		os::error ec;

		while (avail == 0) {
			ec = pipe.available(avail);
			if (ec != os::error::Success) {
				break;
			}
		}

		read_buf.resize(avail);
		ec = pipe.read(read_buf.data(), avail, read_op,
					   std::bind(&client_data::read_cb, this, std::placeholders::_1, std::placeholders::_2));
		if (ec != os::error::Success) {
			throw std::exception("unexpected error");
		}

		if (pending_msg > 0) {
			pending_msg--;
		}
	}

	void wait_for_signal() {
		os::error ec;

		go_time = go_timer.track();

		shared::logger::log("Waiting for sin.");
		ec = sin.wait();
		if (ec != os::error::Success) {
			throw std::exception("unexpected error");
		}

		shared::logger::log("Waiting for available.");
		size_t avail = 0;
		while (avail == 0) {
			ec = pipe.available(avail);
			if (ec != os::error::Success) {
				break;
			}
		}
		if (avail == 0) {
			throw std::exception("unexpected error");
		}

		shared::logger::log("Waiting for read.");
		read_buf.resize(avail);
		ec = pipe.read(read_buf.data(), avail, read_op, nullptr);
		if (ec != os::error::Success) {
			throw std::exception("unexpected error");
		}

		shared::logger::log("Waiting for read_op.");
		ec = read_op->wait();
		if (ec != os::error::Success && ec != os::error::TimedOut) { // how does an infinite wait TIME OUT?!
			throw std::exception("unexpected error");
		}

		if (read_buf[0] == 'G' && read_buf[1] == 'O') {
			go_time.reset();
			is_initialized = true;
			shared::logger::log("Signal received.");
		} else {
			shared::logger::log("Wrong initial message");
			throw std::exception("failed to initialize");
		}
		read_op->invalidate();
	}

	void loop() {
		size_t avail = 0;

		// Wait for signal
		wait_for_signal();

		// Send first message.
		send_message();

		size_t last_recv = 0, last_send = 0;
		auto loop_time = loop_timer.track();
		while (pipe.is_connected()) {
			// Commit 2a6040df7922a077dc9aa16216e4085796637a42:
			// - Windows: You need a thread that can enter an alertable state or is permanently in that state.
			if (count_recv == CLIENT_MAX_MESSAGES)
				break;

			if ((count_recv + count_send) - (last_recv + last_send) > 100) {
				shared::logger::log("Sent %lld and received %lld.", count_send, count_recv);
				last_recv = count_recv;
				last_send = count_send;
			}
			
			os::waitable *waits[] = {
				&sin,
				read_op.get(),
				write_op.get(),
			};
			size_t wait_index = std::numeric_limits<size_t>::max();
			for (size_t idx = 0; idx < 3; idx++) {
				if (waits[idx] != nullptr && waits[idx]->wait(std::chrono::milliseconds(0)) == os::error::Success) {
					wait_index = idx;
					break;
				}
			}
			if (wait_index == std::numeric_limits<size_t>::max()) {
				os::error code = os::waitable::wait_any(waits, 3, wait_index, std::chrono::milliseconds(100));
				if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("unexpected error");
				}
			}
			if (wait_index == std::numeric_limits<size_t>::max()) {
				continue;
			}
			if (waits[wait_index] == &sin) {
				read_message();
			} else if (write_op && (waits[wait_index] == write_op.get())) {
				//send_message();
			}

			Sleep(0);
		}
		loop_time.reset();

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
	}
};

int client(int argc, const char *argv[]) {
	client_data cd;
	shared::logger::log("Client initialized.");
	cd.loop();
	std::cin.get();
	return 0;
}

int main(int argc, const char *argv[]) {
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
}