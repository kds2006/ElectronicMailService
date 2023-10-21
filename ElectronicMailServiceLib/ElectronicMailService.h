#pragma once
#include "EMailMessage.h"
#include "EmailAccount.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>

struct ElectronicMailService {
	static void send_message(EMailMessage const& msg);
	static void forward_message(string const& addr, EMailMessage const& msg);
	static void report_suspicious(EMailMessage const& msg);
};

extern const string gSecDep;
extern const string gBob;
extern const string gAlice;
extern const string gSuspiciousFooter;
extern map<string, unique_ptr<EMailAccount>> gAccounts;
extern map<string, string> gAddressBook;