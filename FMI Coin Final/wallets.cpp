#include <iostream>
#include <fstream>
#include "wallets.h"
#include "transactions.h"
#include "shared.h"
#include "orders.h"
using namespace std;

const short MAX_NAME_SIZE = 256;

void GetWalletData(char name[MAX_NAME_SIZE], double &fiatMoney)
{
	cout << "Enter your starting amount of money: ";
	do
	{
		cin.clear();
		cin >> fiatMoney;
	} while (!cin);
	cout << "Enter name: ";
	cin.ignore();
	cin.getline(name, MAX_NAME_SIZE - 1);
}

long long NumOfWallets()
{
	ifstream file;
	file.open(WALLET_PATH, ios::binary | ios::in);
	if (!file)
	{
		return 0;
	}

	file.seekg(0, ios::end);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << WALLET_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	unsigned long long fileSize = file.tellg();
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << WALLET_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.seekg(0, ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << WALLET_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.close();

	return fileSize / sizeof(Wallet);
}

bool AddWallet()
{
	Wallet wallet;
	GetWalletData(wallet.owner, wallet.fiatMoney);
	if (!GetID(wallet))
		return false;

	ofstream file(WALLET_PATH, ios::app | ios::binary);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << WALLET_PATH << endl;
		return false;
	}
	file.write((const char*)&wallet, sizeof(Wallet));
	if (!file)
	{
		cerr << "Couldn't write to file: " << WALLET_PATH << endl;
		file.close();
		return false;
	}

	double fmiCoins = ConvertTo(wallet.fiatMoney, true);

	if (!AddTransaction(SYSTEM_WALLET_ID, wallet.id, fmiCoins))
	{
		file.close();
		return false;
	}
	file.close();

	return true;
}

bool GetWallet(Wallet& wallet, const unsigned walletId)
{
	ifstream file(WALLET_PATH, ios::in | ios::binary);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << WALLET_PATH << endl;
		return false;
	}
	file.seekg(walletId * sizeof(Wallet), ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << WALLET_PATH << endl;
		file.close();
		return false;
	}
	
	file.read((char*)&wallet, sizeof(Wallet));
	if (!file)
	{
		cerr << "Couldn't read from file: " << WALLET_PATH << endl;
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool RefreshWallet(Wallet& wallet)
{
	ofstream file(WALLET_PATH, ios::binary | ios::in | ios::out | ios::ate);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << WALLET_PATH << endl;
		return false;
	}
	file.seekp(wallet.id * sizeof(Wallet), ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << WALLET_PATH << endl;
		file.close();
		return false;
	}
	file.write((const char*)&wallet, sizeof(Wallet));
	if (!file)
	{
		cerr << "Failed to write to file: " << WALLET_PATH << endl;
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool GetID(Wallet & wallet)
{
	if (!fileExists(WALLET_PATH))
	{
		wallet.id = 0;
		return true;
	}
	long long numOfWallets = NumOfWallets();
	if (IsError(numOfWallets))
		return false;
	unsigned chek = -1;
	if (numOfWallets > chek)
	{
		cerr << "The limit of wallets has been reached. You can't add any more wallets." << endl;
		return false;
	}

	wallet.id = numOfWallets;
	return true;
}

bool WalletInfo(unsigned walletId)
{
	long long numOfWallets = NumOfWallets();
	if (IsError(numOfWallets))
		return false;

	if (walletId >= numOfWallets)
	{
		cerr << "Invalid ID passed" << endl;
		return true;
	}

	Wallet wallet;
	if (!GetWallet(wallet, walletId))
		return false;
	double fmiCoin = WalletCoins(walletId);
	if (fmiCoin < 0)
		return false;

	cout << "name     : " << wallet.owner << endl;
	cout << "fiatMoney: " << wallet.fiatMoney << endl;
	cout << "fmiCoin  : " << fmiCoin << endl;
	return true;
}

double GetCoinsOnHold(unsigned walletId)
{
	Order* order;
	long long numOfOrders = LoadOrders(order);
	if (IsError(numOfOrders))
		return -1.0;

	double coinsOnHold = 0.0;
	for (long long i = 0; i < numOfOrders; i++)
	{
		if (walletId == order[i].walletId && order[i].type == Order::Type::SELL)
			coinsOnHold += order[i].fmiCoins;
	}

	return coinsOnHold;
}

double WalletCoins(unsigned walletId)
{
	long long numOfWallets = NumOfWallets();
	if (IsError(numOfWallets))
		return -1.0;
	Transaction check;
	if (!TakeLastTransaction(check)) return false;

	if (walletId >= numOfWallets && check.senderId != SYSTEM_WALLET_ID)
	{
		cerr << "Invalid ID passed" << endl;
		return -1.0;
	}

	Transaction* transaction = nullptr;

	long long numOfTransactions = LoadTransactions(transaction);
	if (IsError(numOfTransactions))
		return -1.0;

	double fmiCoins = 0.0;

	for (long long i = 0; i < numOfTransactions; i++)
	{
		if (transaction[i].receiverId == walletId)
			fmiCoins += transaction[i].fmiCoins;
		if (transaction[i].senderId == walletId)
			fmiCoins -= transaction[i].fmiCoins;
	}
	delete[] transaction;

	double coinsOnHold = GetCoinsOnHold(walletId);
	if (coinsOnHold < 0)
		return -1.0;

	return Sub(fmiCoins, coinsOnHold);
}
