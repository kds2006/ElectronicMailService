// ElectronicMailService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ElectronicMailService.h"

using namespace std;

map<string, unique_ptr<EMailAccount>> gAccounts;

int main()
{
    for (auto& record: gAddressBook) {
        gAccounts.emplace(record.second, make_unique<EMailAccount>(gAddressBook, record.first, record.second));
    }

    for_each(gAccounts.begin(), gAccounts.end(), [](auto& account) { if (account.second) account.second->run(); });
    auto ft = gAccounts.begin()->second->send({ gAccounts.begin()->first, (++gAccounts.begin())->first,"Hello!"});

    ft.wait();

    for_each(gAccounts.begin(), gAccounts.end(), [](auto& account) { if (account.second) account.second->stop(); });
}
