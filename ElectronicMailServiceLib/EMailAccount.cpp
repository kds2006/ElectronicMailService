#include "EMailAccount.h"
#include "ElectronicMailService.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>

future<void> EMailAccount::send(EMailMessage const& msg) {
	lock_guard<mutex> lockGuard(JobListMutex);
	JobList.emplace_back([this, msg]() { do_send(msg); });
	return JobList.back().get_future();
}

future<void> EMailAccount::receive(EMailMessage const& msg) {
	lock_guard<mutex> lockGuard(JobListMutex);
	JobList.emplace_back([this, msg]() { do_receive(msg); });
	return JobList.back().get_future();
}

future<void> EMailAccount::forward(string const& addr, EMailMessage const& msg) {
	lock_guard<mutex> lockGuard(JobListMutex);
	JobList.emplace_back([this, addr, msg]() { do_forward(addr, msg); });
	return JobList.back().get_future();
}

void EMailAccount::block() {
	IsBlocked = true;
}

bool EMailAccount::is_blocked() const {
	return IsBlocked;
}

bool EMailAccount::has_spam(EMailMessage const& msg) const {
	static vector<string> spam_words = {
		"discount",
		"inheritance",
		"lottery"
	};
	auto body = msg.body;
	for_each(body.begin(), body.end(), [](auto& c) { c = tolower(c); });
	for (const auto& w : spam_words) {
		if (string::npos != body.find(w)) return true;
	}
	return false;
}

bool EMailAccount::is_suspicious(EMailMessage const& msg) const {
	static vector<string> susp_words = {
		"suspicious"
	};
	auto body = msg.body;
	for_each(body.begin(), body.end(), [](auto& c) { c = tolower(c); });
	for (const auto& w : susp_words) {
		if (string::npos != body.find(w)) return true;
	}
	return false;
}

void EMailAccount::log_msg(EMailMessage const& msg) const {
	std::string filename{ Name + "_" + "receivedmails.log" };
	std::fstream s{ filename, s.out | s.app };
	if (s.is_open())
	{
		s << "[" << msg.snd_address << "=>" << msg.recv_address << "]: " << msg.body << endl;
	}
}

void EMailAccount::do_send(EMailMessage const& msg) const {
	ElectronicMailService::send_message(msg);
#ifdef GTEST_API_
	++send_qty;
#endif // GTEST_API_
}

void EMailAccount::do_receive(EMailMessage const& msg) {
#ifdef GTEST_API_
	++recv_qty;
#endif // GTEST_API_

	if (has_spam(msg)) {
#ifdef GTEST_API_
		++spam_qty;
#endif // GTEST_API_
		return;
	}
	if (Name == gSecDep && msg.body.size() >= gSuspiciousFooter.size() && !msg.body.compare(0, gSuspiciousFooter.size(), gSuspiciousFooter))
	{
		// Let's log suspicious message on Security Department.
#ifdef GTEST_API_
		++sec_dep_suspicious_qty;
#endif // GTEST_API_

	}else if (is_suspicious(msg)) {
#ifdef GTEST_API_
		++suspicious_qty;
#endif // GTEST_API_
		ElectronicMailService::report_suspicious(msg);
		return;
	}
	log_msg(msg);
#ifdef GTEST_API_
	++log_qty;
#endif // GTEST_API_
}

void EMailAccount::do_forward(string const& addr, EMailMessage const& msg) const {
	ElectronicMailService::forward_message(addr, msg);
#ifdef GTEST_API_
	++forwarded_qty;
#endif // GTEST_API_
}

void EMailAccount::run() {
	StopThread = false;
	t = move(thread([this] { while (!StopThread) { run_step(); this_thread::yield(); }}));
}

void EMailAccount::stop() {
	StopThread = true;
	t.join();
}

void EMailAccount::run_step() {
	lock_guard<mutex> lockGuard(JobListMutex);
	if (!JobList.empty()) {
		//auto job = std::move(JobList.front());
		//JobList.pop_front();
		//job();
		JobList.front()();
		JobList.pop_front();
	}
}
