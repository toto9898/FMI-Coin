#include <iostream>
#include <ctime>
#include <fstream>
#include "shared.h"
#include "transactions.h"
#include "wallets.h"
#include "orders.h"
using namespace std;


long long NumOfTransactions()
{
	ifstream file;
	file.open(TRANSACTIONS_PATH, ios::binary | ios::in);
	if (!file)
	{
		return 0;
	}

	file.seekg(0, ios::end);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << TRANSACTIONS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	unsigned long long fileSize = file.tellg();
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << TRANSACTIONS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.seekg(0, ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << TRANSACTIONS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.close();

	return fileSize / sizeof(Transaction);
}

long long LoadTransactions(Transaction *& transaction)
{
	/* Loads the transaction from the file and returns their amount*/

	long long numOfTransactions = NumOfTransactions();
	if (IsError(numOfTransactions))
		return numOfTransactions;
	if (numOfTransactions == 0)
		return 0;

	transaction = new(std::nothrow) Transaction[numOfTransactions];
	if (!transaction)
	{
		cerr << "Failed to allocate space for file content: " << TRANSACTIONS_PATH << endl;
		return MEMORY_ALLOC_ERROR;
	}

	ifstream file;
	file.open(TRANSACTIONS_PATH, ios::binary | ios::in);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << TRANSACTIONS_PATH << endl;
		delete[] transaction;
		return FILE_DIDNT_OPEN;
	}

	for (long long i = 0; i < numOfTransactions; i++)
	{
		file.read((char*)&transaction[i], sizeof(Transaction));
		if (file.bad())
		{
			std::cerr << "Failed reading from file: " << TRANSACTIONS_PATH << std::endl;
			file.close();
			delete[] transaction;
			return READING_FAILED;
		}
	}

	file.close();

	return numOfTransactions;
}

bool AddTransaction(const unsigned from, const unsigned to, const double fmiCoins)
{
	ofstream file(TRANSACTIONS_PATH, ios::app | ios::binary);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << TRANSACTIONS_PATH << endl;
		return false;
	}
	
	Transaction toBeAdded;
	toBeAdded.fmiCoins = fmiCoins;
	toBeAdded.senderId = from;
	toBeAdded.receiverId = to;
	toBeAdded.time = time(0);
	
	file.write((const char*)&toBeAdded, sizeof(toBeAdded));
	if (!file)
	{
		cerr << "Failed to write to file: " << TRANSACTIONS_PATH << endl;
		file.close(); 
		return false;
	}

	file.close();

	// ---------   Проверява дали след тази транзакция
	AttractInv possibleInvestor;
	possibleInvestor.fmiCoins = WalletCoins(to);
	if (possibleInvestor.fmiCoins < 0) return false;

	possibleInvestor.numOfOrders = NumOfOrdersMade(to);
	if (possibleInvestor.numOfOrders < 0) return false;

	if (!TakeTime1stTransaction(to, possibleInvestor.time1stTr)) return false;
	
	possibleInvestor.timeLastTr = toBeAdded.time;

	if (!AddInvestor(possibleInvestor)) return false;

	return true;
}

bool TakeLastTransaction(Transaction& transaction)
{
	long long numOfTransactions = NumOfTransactions();
	if (IsError(numOfTransactions))
		return false;

	ifstream file;
	file.open(TRANSACTIONS_PATH, ios::binary | ios::in);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << TRANSACTIONS_PATH << endl;
		return false;
	}

	file.seekg((numOfTransactions - 1) * sizeof(Transaction), ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << TRANSACTIONS_PATH << endl;
		file.close();
		return false;
	}

	file.read((char*)&transaction, sizeof(Transaction));
	if (!file)
	{
		cerr << "Couldn't read from file: " << TRANSACTIONS_PATH << endl;
		file.close();
		return false;
	}
	file.close();

	return true;
}

bool TakeTime1stTransaction(unsigned walletId, unsigned long long& time)
{
	long long numOfTransactions = NumOfTransactions();
	if (IsError(numOfTransactions))
		return false;

	ifstream file;
	file.open(TRANSACTIONS_PATH, ios::binary | ios::in);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << TRANSACTIONS_PATH << endl;
		return false;
	}
	
	Transaction transaction;
	for (size_t i = 0; i < numOfTransactions; i++)
	{
		file.read((char*)&transaction, sizeof(Transaction));
		if (!file)
		{
			cerr << "Couldn't read from file: " << TRANSACTIONS_PATH << endl;
			file.close();
			return false;
		}
		if (transaction.senderId == SYSTEM_WALLET_ID)
		{
			file.close();
			time = transaction.time;
			return true;
		}

	}
	file.close();

	return false;
}
