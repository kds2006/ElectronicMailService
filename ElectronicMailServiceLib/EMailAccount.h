#pragma once
#include "EMailMessage.h"
#include <map>
#include <string>
#include <future>
#include <deque>
#include <thread>

struct EMailAccount
{
	EMailAccount(map<string, string> const& abook, string const& name, string const& addr) : ABook{ abook }, Name{ name }, Addr{addr} {}
	future<void> send(EMailMessage const& msg);
	future<void> receive(EMailMessage const& msg);
	future<void> forward(string const& addr, EMailMessage const& msg);
	void block();
	bool is_blocked() const;
	future<pair<bool, int>> enqueue_task(int i);
	void run();
	void run_step();
	void stop();
private:
	void do_send(EMailMessage const& msg) const;
	void do_receive(EMailMessage const& msg);
	void do_forward(string const& addr, EMailMessage const& msg) const;
	bool has_spam(EMailMessage const& msg) const;
	bool is_suspicious(EMailMessage const& msg) const;
	void log_msg(EMailMessage const& msg) const;
private:
	map<string, string> const& ABook;
	string Name;
	string Addr;
	bool IsBlocked = false;
	deque<packaged_task<void()>> JobList;
	mutex JobListMutex;
	bool StopThread = false;
	thread t;

#ifdef GTEST_API_
public:
	size_t get_job_size() { return JobList.size(); }
	mutable unsigned send_qty = 0;
	mutable unsigned recv_qty = 0;
	mutable unsigned forwarded_qty = 0;
	mutable unsigned spam_qty = 0;
	mutable unsigned sec_dep_suspicious_qty = 0;
	mutable unsigned suspicious_qty = 0;
	mutable unsigned log_qty = 0;

	friend class EMailAccountTest_RegularMessage_Test;
	friend class EMailAccountTest_SpamMessage_Test;
	friend class EMailAccountTest_SuspiciousMessage_Test;
#endif // GTEST_API_
};

