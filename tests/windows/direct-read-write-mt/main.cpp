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

#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include "../../../source/os/windows/named-pipe.hpp"
#include "../../../source/os/windows/semaphore.hpp"
#include "../../common.hpp"

using namespace std::placeholders;

static std::string       pipe_name        = "Data";
static size_t            max_messages     = 10000;
static size_t            parallel_threads = 5;
std::vector<std::string> messages         = {
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

void hexlog(std::vector<char> &data) {
	static const char hexindex[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};

	std::string hex;
	for (size_t p = 0, e = data.size(); p < e; p++) {
		uint16_t c  = data[p];
		char     u0 = hexindex[c & 0xF];
		char     u1 = hexindex[(c & 0xF0) >> 8];
		hex         = hex + u1 + u0;
	}
	shared::logger::log("%lld 0x%.*s", hex.size(), hex.size(), hex.data());
}

class server_ {
	std::shared_ptr<os::async_op> rop, wop, aop;
	std::vector<char>             buf, wbuf;
	os::windows::named_pipe       pipe;

	size_t count_recv = 0;
	size_t count_send = 0;

	public:
	server_()
		: pipe(os::create_only, pipe_name, 255, os::windows::pipe_type::Byte, os::windows::pipe_read_mode::Byte, true) {
		buf.reserve(65535);
		wbuf.reserve(65535);
		shared::logger::log("Server initialized.");
	}
	~server_() {}

	void write_cb(os::error success, size_t length) {
		wop->invalidate();
		rop->invalidate();

		//shared::logger::log("write_cb %lld %lld", success, length);
		//hexlog(wbuf);
		if (success == os::error::Ok) {
			count_send++;

			if (count_send % 200 == 0 && (count_send > 0)) {
				shared::logger::log("Sent %lld messages.", count_send);
			}
		}
	}

	void parse_msg() {
		//shared::logger::log("parse_msg %.*s", buf.size(), buf.data());
		count_recv++;

		if (count_recv % 200 == 0 && (count_recv > 0)) {
			shared::logger::log("Received %lld messages.", count_recv);
		}

		wbuf.resize(buf.size() + 2);
		memcpy(wbuf.data() + 2, buf.data(), buf.size());
		reinterpret_cast<short &>(wbuf[0]) = buf.size();

		//shared::logger::log("parse_msg write %lld %.*s", wbuf.size(), buf.size(), buf.data());
		os::error ec = pipe.write(wbuf.data(), wbuf.size(), rop, std::bind(&server_::write_cb, this, _1, _2));
		if (ec != os::error::Success) {
			throw std::exception("Error");
		}
	}

	void read_2_cb(os::error success, size_t length) {
		//shared::logger::log("read_2_cb %lld %lld", success, length);
		//hexlog(buf);
		if (success == os::error::Success) {
			parse_msg();
		} else {
			throw std::exception("Error");
		}
	}

	void read_1_cb(os::error success, size_t length) {
		rop->invalidate();
		//shared::logger::log("read_1_cb %lld %lld", success, length);
		//hexlog(buf);

		size_t size = reinterpret_cast<short &>(buf[0]);
		buf.resize(size);
		os::error ec = pipe.read(buf.data(), buf.size(), rop, std::bind(&server_::read_2_cb, this, _1, _2));
		if (ec != os::error::Success) {
			throw std::exception("Error");
		}
	}

	void accept_cb(os::error success, size_t length) {
		if (success == os::error::Ok) {
			buf.resize(2);
			buf[0]       = 'G';
			buf[1]       = 'O';
			os::error ec = pipe.write(buf.data(), buf.size(), wop, nullptr);
			if (ec == os::error::Pending || ec == os::error::Success) {
			} else {
				throw std::exception("Error");
			}
		}
	}

	void loop(const char *argv[]) {
		os::error ec = os::error::Success;

		// Spawn client process
		shared::os::process client(std::string(argv[0]), std::string(argv[0]) + " client",
								   shared::os::get_working_directory());
		shared::logger::log("Client spawned, waiting...");

		while (!pipe.is_connected()) {
			ec = pipe.accept(aop, std::bind(&server_::accept_cb, this, _1, _2));
			if (ec != os::error::Pending && ec != os::error::Success) {
				if (ec == os::error::Connected) {
					break;
				}
				throw std::exception("Error");
			}
			ec = aop->wait();
			if (ec != os::error::Success) {
				throw std::exception("Error");
			}
		}
		shared::logger::log("Client connected.");

		ec = wop->wait();
		if (ec == os::error::Pending || ec == os::error::Success) {
		} else {
			throw std::exception("Error");
		}
		wop->invalidate();
		shared::logger::log("Client signalled.");

		buf.resize(1);
		while (pipe.is_connected()) {
			if (!rop || !rop->is_valid()) {
				// Try and read a single byte.
				buf.resize(2);
				ec = pipe.read(buf.data(), 2, rop, std::bind(&server_::read_1_cb, this, _1, _2));
				if (ec != os::error::Success) {
					throw std::exception("Error");
				}
			}

			std::vector<os::waitable *> waits      = {rop.get(), wop.get()};
			size_t                      wait_index = -1;
			for (size_t idx = 0; idx < waits.size(); idx++) {
				if (waits[idx]->wait(std::chrono::milliseconds(0)) == os::error::Success) {
					wait_index = idx;
					break;
				}
			}
			if (wait_index == -1) {
				os::error code = os::waitable::wait_any(waits, wait_index, std::chrono::milliseconds(1));
				if (code == os::error::TimedOut) {
					continue;
				} else if (code == os::error::Disconnected) {
					break;
				} else if (code == os::error::Error) {
					throw std::exception("Error");
				}
			}
		}
	}
};

class client_write_thread {
	os::windows::named_pipe *     pipe;
	std::thread                   worker;
	bool                          stop = false;
	std::shared_ptr<os::async_op> op;
	std::vector<char>             buf;
	size_t                        max_send   = 10;
	size_t                        count_send = 0;
	std::condition_variable       wait;
	bool                          wait_trigger = false;
	std::mutex                    wait_mutex;

	void write_cb(os::error code, size_t size) {
		op->invalidate();
		if (code == os::error::Success) {
			count_send++;

			if (count_send % 200 == 0 && (count_send > 0)) {
				shared::logger::log("Sent %lld messages.", count_send);
			}
		}
	}

	void loop() {
		os::error ec;

		std::unique_lock<std::mutex> ul(wait_mutex);
		wait.wait(ul, [this] { return wait_trigger; });

		while (!stop && pipe->is_connected()) {
			if (count_send >= max_send) {
				break;
			}

			if ((count_send < max_send) && (!op || !op->is_valid())) {
				size_t idx = count_send % messages.size();
				/*shared::logger::log("Sending message %lld with content '%.*s'.", count_send, messages[idx].size(),
									messages[idx].c_str());				*/
				buf.resize(messages[idx].size() + sizeof(short) + sizeof(long long));
				memcpy(buf.data() + sizeof(short) + sizeof(long long), messages[idx].data(), messages[idx].size());
				reinterpret_cast<short &>(buf[0]) = buf.size() - sizeof(short);
				reinterpret_cast<long long &>(buf[sizeof(short)]) =
					std::chrono::high_resolution_clock::now().time_since_epoch().count();
				ec = pipe->write(buf.data(), buf.size(), op, std::bind(&client_write_thread::write_cb, this, _1, _2));
				if (ec != os::error::Success && ec != os::error::Pending) {
					throw std::exception("Error");
				}
			}

			if (op) {
				ec = op->wait(std::chrono::milliseconds(0));
				if (ec != os::error::Success) {
					ec = op->wait(std::chrono::milliseconds(10));
				}
			}
		}
	}

	public:
	client_write_thread(os::windows::named_pipe *pipe, size_t max_send) {
		this->pipe     = pipe;
		this->stop     = false;
		this->worker   = std::thread(std::bind(&client_write_thread::loop, this));
		this->max_send = max_send;
	}
	~client_write_thread() {
		stop = true;
		if (worker.joinable())
			worker.join();
	}

	void signal() {
		std::unique_lock<std::mutex> ul(wait_mutex);
		wait_trigger = true;
		wait.notify_all();
	}
};

class client_ {
	std::shared_ptr<os::async_op>                     op;
	std::vector<char>                                 buf;
	os::windows::named_pipe                           pipe;
	std::vector<std::unique_ptr<client_write_thread>> threads;

	shared::time::measure_timer                            msg_timer;
	std::unique_ptr<shared::time::measure_timer::instance> msg_time;

	size_t count_recv = 0;

	public:
	client_() : pipe(os::open_only, pipe_name, os::windows::pipe_read_mode::Byte) {
		buf.reserve(65535);

		size_t total_messages = max_messages;
		for (size_t idx = 0; idx < parallel_threads; idx++) {
			size_t cur_messages = max_messages / parallel_threads;
			if (idx == (parallel_threads - 1)) {
				cur_messages = total_messages;
			}
			threads.push_back(std::make_unique<client_write_thread>(&pipe, cur_messages));
			total_messages -= cur_messages;
		}

		shared::logger::log("Client initialized.");
	}
	~client_() {}

	void read_2_cb(os::error ec, size_t bytes) {
		op->invalidate();
		//shared::logger::log("read_2_cb %lld %lld", ec, bytes);
		//hexlog(buf);
		if (ec == os::error::Ok) {
			long long now_time  = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			long long orig_time = reinterpret_cast<long long &>(buf[0]);
			long long total_time = now_time - orig_time;

			msg_timer.manual_track(std::chrono::nanoseconds(total_time));

			// Validate
			//bool valid = false;
			//for (std::string &msg : messages) {
			//	if (msg.size() == (bytes - sizeof(long long))) {
			//		if (msg == std::string(buf.data() + sizeof(long long), buf.data() + buf.size())) {
			//			valid = true;
			//		}
			//	}
			//}
			//if (!valid) {
			//	throw std::exception("failed");
			//}

			count_recv++;

			if (count_recv % 200 == 0 && (count_recv > 0)) {
				shared::logger::log("Received %lld messages.", count_recv);
			}
		}
	}

	void read_1_cb(os::error ec, size_t bytes) {
		op->invalidate();
		//shared::logger::log("read_1_cb %lld %lld", ec, bytes);
		//hexlog(buf);

		// All reads of this type should be considered "more data".
		// Size of current message is the first two bytes. Does not
		//  have to match up with available() or total_available().

		size_t size = reinterpret_cast<short &>(buf[0]);
		buf.resize(size);

		ec = pipe.read(buf.data(), buf.size(), op, std::bind(&client_::read_2_cb, this, _1, _2));
		if (ec != os::error::Success) {
			throw std::exception("Error");
		}
	}

	void loop() {
		os::error ec = os::error::Success;

		shared::logger::log("Waiting for signal...");
		buf.resize(2);
		ec = pipe.read(buf.data(), buf.size(), op, nullptr);
		if (ec != os::error::Success) {
			throw std::exception("Error");
		}
		ec = op->wait(std::chrono::milliseconds(100000000));
		if (ec != os::error::Success && (ec != os::error::TimedOut)) {
			throw std::exception("Error");
		}
		op->invalidate();
		buf.clear();
		buf.reserve(65535);
		shared::logger::log("Signal received.");

		for (auto &v : threads) {
			v->signal();
		}

		while (pipe.is_connected()) {
			if (count_recv >= max_messages)
				break;

			if ((count_recv < max_messages) && (!op || !op->is_valid())) {
				// Try and read a single byte.
				buf.resize(2);
				ec = pipe.read(buf.data(), 2, op, std::bind(&client_::read_1_cb, this, _1, _2));
				if (ec != os::error::Success && ec != os::error::Pending) {
					throw std::exception("Error");
				}
			}

			if (op) {
				ec = op->wait(std::chrono::milliseconds(0));
				if (ec != os::error::Success) {
					ec = op->wait(std::chrono::milliseconds(10));
				}
			}
		}

		shared::logger::log("Total Messages: %lld", msg_timer.count());
		shared::logger::log("Total Time: %16llu ns", msg_timer.total().count());
		shared::logger::log("Avrg. Time: %16.0f ns", msg_timer.average());
		shared::logger::log("Best  Time: %16llu ns", msg_timer.percentile(0, true));
		shared::logger::log("Worst Time: %16llu ns", msg_timer.percentile(1, true));
		shared::logger::log("  .1%% Time: %16llu ns", msg_timer.percentile(0.001).count());
		shared::logger::log(" 1.0%% Time: %16llu ns", msg_timer.percentile(0.01).count());
		shared::logger::log("10.0%% Time: %16llu ns", msg_timer.percentile(0.10).count());
		shared::logger::log("25.0%% Time: %16llu ns", msg_timer.percentile(0.25).count());
		shared::logger::log("50.0%% Time: %16llu ns", msg_timer.percentile(0.5).count());
		shared::logger::log("75.0%% Time: %16llu ns", msg_timer.percentile(0.75).count());
		shared::logger::log("90.0%% Time: %16llu ns", msg_timer.percentile(0.9).count());
		shared::logger::log("99.0%% Time: %16llu ns", msg_timer.percentile(0.99).count());
		shared::logger::log("99.9%% Time: %16llu ns", msg_timer.percentile(0.999).count());
	}
};

int server(int argc, const char *argv[]) {
	server_ s;
	s.loop(argv);
	std::cin.get();
	return 0;
}

int client(int argc, const char *argv[]) {
	client_ s;
	s.loop();
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