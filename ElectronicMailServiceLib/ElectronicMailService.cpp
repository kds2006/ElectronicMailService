// ElectronicMailService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ElectronicMailService.h"

using namespace std;

const string gSecDep = "Security Department";
const string gBob = "Bob";
const string gAlice = "Alice";
const string gSuspiciousFooter = "[SUSPICIOUS]";

map<string, string> gAddressBook = {
    {gSecDep,"sec.dep@paramount.com"},
    {gBob,"bob@paramount.com"},
    {gAlice,"alice@paramount.com"}
};

void ElectronicMailService::send_message(EMailMessage const& msg) {
    auto account = gAccounts.find(msg.recv_address);
    if (account != gAccounts.end() && account->second) {
        account->second->receive(msg);
    }
}

void ElectronicMailService::forward_message(string const& addr, EMailMessage const& msg) {
    auto account = gAccounts.find(addr);
    if (account != gAccounts.end() && account->second) {
        account->second->receive(msg);
    }
}

void ElectronicMailService::report_suspicious(EMailMessage const& msg) {
    auto account = gAccounts.find(msg.snd_address);
    if (account != gAccounts.end() && account->second) {
        account->second->block();
    }
    EMailMessage susp_msg;
    susp_msg.snd_address = msg.snd_address;
    susp_msg.recv_address = msg.recv_address;
    susp_msg.body = gSuspiciousFooter + msg.body;

    if (auto pos = gAddressBook.find(gSecDep); pos != gAddressBook.end()) {
        forward_message(pos->second, susp_msg);
    }
}

