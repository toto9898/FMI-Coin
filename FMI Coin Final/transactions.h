#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H


struct Transaction {
	unsigned long long time;
	unsigned senderId;
	unsigned receiverId;
	double fmiCoins;
};

long long NumOfTransactions();
long long LoadTransactions(Transaction *& transaction);
bool AddTransaction(const unsigned from, const unsigned to, const double fmiCoins);
bool TakeLastTransaction(Transaction& transaction);
bool TakeTime1stTransaction(unsigned walletId, unsigned long long& time);

#endif // !TRANSACTIONS_H
