#include "pch.h"
#include "ElectronicMailService.h"

map<string, unique_ptr<EMailAccount>> gAccounts;

class EMailAccountTest : public testing::Test {
protected:
    void SetUp() override {
        for (auto& record : gAddressBook) {
            gAccounts.emplace(record.second, make_unique<EMailAccount>(gAddressBook, record.first, record.second));
        }
        for_each(gAccounts.begin(), gAccounts.end(), [](auto& account) { if (account.second) account.second->run(); });

        secdep_acc = get_ptr(gSecDep);
        bob_acc = get_ptr(gBob);
        alice_acc = get_ptr(gAlice);
    }

    void TearDown() override {
        for_each(gAccounts.begin(), gAccounts.end(), [](auto& account) { if (account.second) account.second->stop(); });
        gAccounts.clear();
    }

    EMailAccount* get_ptr(string const& name) {
        if (auto pos = gAddressBook.find(name); pos != gAddressBook.end()) {
            if (auto account = gAccounts.find(pos->second); account != gAccounts.end()) {
                return account->second.get();
            }
        }
        return nullptr;
    }

    EMailAccount* secdep_acc = nullptr;
    EMailAccount* bob_acc = nullptr;
    EMailAccount* alice_acc = nullptr;
};

TEST_F(EMailAccountTest, RegularMessage) {
    ASSERT_TRUE(bob_acc && alice_acc);
    auto bob_send_qty = bob_acc->send_qty;
    auto alice_recv_qty = alice_acc->recv_qty;
    auto alice_log_qty = alice_acc->log_qty;
    bob_acc->send({ bob_acc->Addr, alice_acc->Addr, "Hello!" }).wait();
    EXPECT_EQ(bob_send_qty + 1, bob_acc->send_qty);

    while (alice_acc->get_job_size()) { this_thread::yield(); }
    EXPECT_EQ(alice_recv_qty+1, alice_acc->recv_qty);
    EXPECT_EQ(alice_log_qty+1, alice_acc->log_qty);
}

TEST_F(EMailAccountTest, SpamMessage) {
    ASSERT_TRUE(bob_acc && alice_acc);
    auto bob_send_qty = bob_acc->send_qty;
    auto alice_recv_qty = alice_acc->recv_qty;
    auto alice_spam_qty = alice_acc->spam_qty;
    auto alice_log_qty = alice_acc->log_qty;
    bob_acc->send({ bob_acc->Addr, alice_acc->Addr, "discount!" }).wait();
    EXPECT_EQ(bob_send_qty+1, bob_acc->send_qty);

    while (alice_acc->get_job_size()) { this_thread::yield(); }
    EXPECT_EQ(alice_recv_qty+1, alice_acc->recv_qty);
    EXPECT_EQ(alice_spam_qty+1, alice_acc->spam_qty);
    EXPECT_EQ(alice_log_qty, alice_acc->log_qty);
}

TEST_F(EMailAccountTest, SuspiciousMessage) {
    ASSERT_TRUE(bob_acc && alice_acc && secdep_acc);
    auto bob_send_qty = bob_acc->send_qty;
    auto alice_recv_qty = alice_acc->recv_qty;
    auto alice_spam_qty = alice_acc->spam_qty;
    auto alice_log_qty = alice_acc->log_qty;
    auto alice_suspicious_qty = alice_acc->suspicious_qty;

    auto secdep_recv_qty = secdep_acc->recv_qty;
    auto secdep_suspicious_qty = secdep_acc->sec_dep_suspicious_qty;
    auto secdep_log_qty = secdep_acc->log_qty;

    bob_acc->send({ bob_acc->Addr, alice_acc->Addr, "suspicious!" }).wait();
    EXPECT_EQ(bob_send_qty + 1, bob_acc->send_qty);

    while (alice_acc->get_job_size()) { this_thread::yield(); }
    EXPECT_EQ(alice_recv_qty + 1, alice_acc->recv_qty);
    EXPECT_EQ(alice_suspicious_qty+1, alice_acc->suspicious_qty);
    EXPECT_EQ(alice_log_qty, alice_acc->log_qty);

    while (secdep_acc->get_job_size()) { this_thread::yield(); }
    EXPECT_EQ(secdep_recv_qty+1, secdep_acc->recv_qty);
    EXPECT_EQ(secdep_suspicious_qty+1, secdep_acc->sec_dep_suspicious_qty);
    EXPECT_EQ(secdep_log_qty+1, secdep_acc->log_qty);

    EXPECT_TRUE(bob_acc->is_blocked());
}

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}