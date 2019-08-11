#include <iostream>
#include <fstream>
#include "wallets.h"
#include "transactions.h"
#include "shared.h"
#include "orders.h"

using namespace std;

long long NumOfOrders()
{
	ifstream file;
	file.open(ORDERS_PATH, ios::binary | ios::in);
	if (!file)
	{
		return 0;
	}

	file.seekg(0, ios::end);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << ORDERS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	unsigned long long fileSize = file.tellg();
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << ORDERS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.seekg(0, ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << ORDERS_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.close();

	return fileSize / sizeof(Order);
}

bool DeleteOrder(Order*& order, const unsigned id, long long& numOfOrders)
{
	if (id >= numOfOrders)
	{
		cerr << "Trying to delete non-existing id (orders)" << endl;
		return false;
	}
	if (numOfOrders == 1)
	{
		delete[] order;
		--numOfOrders;
		order = nullptr;
		return true;
	}
	for (long long i = id; i < numOfOrders - 1; i++)
		order[i] = order[i + 1];

	Order* newOrder = new(nothrow) Order[numOfOrders - 1];
	if (!newOrder)
	{
		cerr << "Couldn't allocate space for orders" << endl;
		return false;
	}
	for (long long i = 0; i < numOfOrders - 1; i++)
		newOrder[i] = order[i];
	delete[] order;
	order = newOrder;
	--numOfOrders;
	return true;
}

long long LoadOrders(Order *& order)
{
	/* Loads the transaction from the file and returns their amount*/

	long long numOfOrders = NumOfOrders();
	if (IsError(numOfOrders))
		return numOfOrders;
	if (numOfOrders == 0)
		return 0;

	order = new(std::nothrow) Order[numOfOrders];
	if (!order)
	{
		cerr << "Failed to allocate space for file content: " << ORDERS_PATH << endl;
		return MEMORY_ALLOC_ERROR;
	}

	ifstream file;
	file.open(ORDERS_PATH, ios::binary | ios::in);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << ORDERS_PATH << endl;
		delete[] order;
		return FILE_DIDNT_OPEN;
	}

	for (long long i = 0; i < numOfOrders; i++)
	{
		file.read((char*)&order[i], sizeof(Order));
		if (file.bad())
		{
			std::cerr << "Failed reading from file: " << ORDERS_PATH << std::endl;
			file.close();
			delete[] order;
			return READING_FAILED;
		}
	}

	file.close();
	return numOfOrders;
}

bool MakeOrder(Order& order, const Order::Type type, const unsigned walletId, const double fmiCoins)
{
	long long numOfWallets = NumOfWallets();
	if (IsError(numOfWallets))
		return false;
	if (walletId >= numOfWallets)
	{
		cerr << "Invalid ID passed" << endl;
		return false;
	}

	Wallet wallet;
	if (!GetWallet(wallet, walletId))
	{
		cerr << "Invalid Id passed" << endl;
		return false;
	}
	if (type == Order::Type::BUY && wallet.fiatMoney / COIN_EXCH_RATE < fmiCoins)
	{
		cerr << "Not enough money for this order!" << endl;
		return false;
	}
	double walletCoins = WalletCoins(wallet.id);
	if (walletCoins < 0)
		return false;
	if(type == Order::Type::SELL && walletCoins < fmiCoins)
	{	
		cerr << "Not enough fmiCoins for this order!" << endl;
		return false;
	}

	order.type = type;
	order.walletId = walletId;
	order.fmiCoins = fmiCoins;
	return true;
}

bool Fulfill(Order& order, const unsigned long long now, bool addTransactionsInfoToTxt, double coinsCirculated)
{
	// Създаваме името на текстовия файл, в който може да с наложи да пишем
	char filename[55];
	CreateFileName(filename, order.walletId, now);
	Transaction transaction; // тази транзакция ще потрябва този текстов файл

	Order* orders = nullptr;
	long long numOfOrders = LoadOrders(orders);
	if (IsError(numOfOrders))
		return false;

	if (numOfOrders == 0 || orders[0].type == order.type)
	{
		delete[] orders;
		if (!AddOrder(order))
			return false;
		if(addTransactionsInfoToTxt)
			if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;

		return true;
	}

	Wallet wallet;
	if (!GetWallet(wallet, order.walletId))
	{
		delete[] orders;
		return false;
	}

	Wallet wallet2;
	if (!GetWallet(wallet2, orders[0].walletId))
	{
		delete[] orders;
		return false;
	}

	if (orders[0].type != order.type)
	{
		if (order.type == Order::Type::SELL)
		{
			// Ако след изпълнение, сме удовлетворили някоя заявка (т.е. ще се налага да премахваме заявка)
			if (orders[0].fmiCoins < order.fmiCoins || Sub(orders[0].fmiCoins, order.fmiCoins) == 0.0)
			{
				// Дали нашата заявка също ще бъде удовлетворена
				bool fulfilled = false;
				if (Sub(orders[0].fmiCoins, order.fmiCoins) == 0.0)
					fulfilled = true;

				wallet.fiatMoney += ConvertTo(orders[0].fmiCoins, false);
				if (!RefreshWallet(wallet))
				{
					delete[] orders;
					return false;
				}
				coinsCirculated += ConvertTo(orders[0].fmiCoins, false);

				if (!AddTransaction(order.walletId, orders[0].walletId, orders[0].fmiCoins))
				{
					delete[] orders;
					return false;
				}

				// Добавяме информацията към текстовия файл
				if (!TakeLastTransaction(transaction))
					return false;

				if (!AddOrderToTxt(filename, transaction, order.type)) 
					return false;
				// край на добавянето

				order.fmiCoins -= orders[0].fmiCoins;
				if (!DeleteOrder(orders, 0, numOfOrders))
					return false;
				if (!SaveOrders(orders, numOfOrders))
					return false;

				if (fulfilled == true)
				{
					delete[] orders;
					orders = nullptr;
					if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;
					return true;
				}
				else
				{
					delete[] orders;
					return Fulfill(order, now, true, coinsCirculated);
				}
			}
			// Ако след изпълнение, сме удовлетворили заявката си 
			//(т.е. просто трябва да променин стойностите на заявка от файла)
			else if (orders[0].fmiCoins > order.fmiCoins)
			{
				wallet.fiatMoney += ConvertTo(order.fmiCoins, false);
				if (!RefreshWallet(wallet))
				{
					delete[] orders;
					return false;
				}
				coinsCirculated += ConvertTo(order.fmiCoins, false);

				orders[0].fmiCoins -= order.fmiCoins;

				if (!AddTransaction(order.walletId, orders[0].walletId, order.fmiCoins))
				{
					delete[] orders;
					return false;
				}

				// Добавяме информацията към текстовия файл
				if (!TakeLastTransaction(transaction))
					return false;

				if (!AddOrderToTxt(filename, transaction, order.type))
					return false;
				// край на добавянето


				if (!SaveOrders(orders, numOfOrders))
					return false;

				if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;

				return true;
			}
		}
		else if (order.type == Order::Type::BUY)
		{
			if (orders[0].fmiCoins < order.fmiCoins || Sub(orders[0].fmiCoins, order.fmiCoins) == 0.0)
			{
				bool fulfilled = false;
				if (Sub(orders[0].fmiCoins, order.fmiCoins) == 0.0)
					fulfilled = true;

				wallet2.fiatMoney += ConvertTo(orders[0].fmiCoins, false);
				if (!RefreshWallet(wallet2))
				{
					delete[] orders;
					return false;
				}
				coinsCirculated += ConvertTo(orders[0].fmiCoins, false);

				if(!AddTransaction(orders[0].walletId, order.walletId, orders[0].fmiCoins))
				{
					delete[] orders;
					return false;
				}

				// Добавяме информацията към текстовия файл
				if (!TakeLastTransaction(transaction))
					return false;

				if (!AddOrderToTxt(filename, transaction, order.type))
					return false;
				// край на добавянето

				order.fmiCoins -= orders[0].fmiCoins;

				if (!DeleteOrder(orders, 0, numOfOrders))
					return false;
				if (!SaveOrders(orders, numOfOrders))
					return false;

				if (fulfilled == true)
				{
					delete[] orders;
					if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;
					return true;
				}
				else
				{
					delete[] orders;
					return Fulfill(order, now, true, coinsCirculated);
				}
			}
			else if (orders[0].fmiCoins > order.fmiCoins)
			{

				wallet2.fiatMoney += ConvertTo(order.fmiCoins, false);
				if (!RefreshWallet(wallet2))
				{
					delete[] orders;
					return false;
				}
				coinsCirculated += ConvertTo(order.fmiCoins, false);

				orders[0].fmiCoins -= order.fmiCoins;
				if (!SaveOrders(orders, numOfOrders))
					return false;

				if (!AddTransaction(orders[0].walletId, order.walletId, order.fmiCoins))
				{
					delete[] orders;
					return false;
				}

				// Добавяме информацията към текстовия файл
				if (!TakeLastTransaction(transaction))
					return false;

				if (!AddOrderToTxt(filename, transaction, order.type))
					return false;
				// край на добавянето

				delete[] orders;
				if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;
				return true;
			}
		}
	}
	if (!AddTransactionsInfoToTxt(filename, coinsCirculated)) return false;
	return true;
}

bool AddOrder(Order& order)
{
	if (order.type == Order::Type::BUY)
	{
		Wallet wallet;
		if(!GetWallet(wallet, order.walletId)) return false;
		
		wallet.fiatMoney = Sub(wallet.fiatMoney, ConvertTo(order.fmiCoins, false));
		if (!RefreshWallet(wallet)) return false;
	}


	ofstream file(ORDERS_PATH, ios::app | ios::binary);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << ORDERS_PATH << endl;
		return false;
	}
	file.write((const char*)&order, sizeof(Order));
	if (!file)
	{
		cerr << "Couldn't write to file: " << ORDERS_PATH << endl;
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool SaveOrders(Order*& order, long long numOfOrders)
{
	ofstream file(ORDERS_PATH, ios::binary | ios::trunc);
	if (!file.is_open())
	{
		cerr << "Couldn't save file: " << ORDERS_PATH << endl;
		return false;
	}

	for (long long i = 0; i < numOfOrders; i++)
	{
		file.write((const char*)&order[i], sizeof(Order));
		if (!file)
		{
			cerr << "Couldn't write to file: " << ORDERS_PATH << endl;
			file.close();
			return false;
		}
	}
	file.close();
	return true;
}

bool PrintOrders()
{
	Order* order = nullptr;
	long long numOfOrders = LoadOrders(order);
	if (IsError(numOfOrders))
		return false;
	if (numOfOrders == 0)
		cerr << "there is no any" << endl;

	for (long long i = 0; i < numOfOrders; i++)
	{
		char bs[5];
		if (order[i].type == order[i].BUY)
			strncpy(bs, "BUY", 4);
		else
			strncpy(bs, "SELL", 5);

		cout << '#' << order[i].walletId << ' ';
		cout << bs << ' ';
		cout << order[i].fmiCoins << '@' << endl;
	}
	return true;
}

bool AddOrderToTxt(const char filename[55], Transaction& transaction, Order::Type type)
{
	ofstream file(filename, ios::out | ios::app);
	if (!file.is_open())
	{
		cerr << "Couldn't save file: " << filename << endl;
		return false;
	}

	if (type == Order::Type::BUY)
	{
		file << transaction.receiverId << " bought " << transaction.fmiCoins << " coins from " << transaction.senderId << endl;
		if (!file)
		{
			cerr << "Couldn't write to file: " << filename << endl;
			file.close();
			return false;
		}
	}
	else if (type == Order::Type::SELL)
	{
		file << transaction.senderId << " sold " << transaction.fmiCoins << " coins to " << transaction.receiverId << endl;
		if (!file)
		{
			cerr << "Couldn't write to file: " << filename << endl;
			file.close();
			return false;
		}
	}
	file.close();
	return true;
}

long long NumOfOrdersMade(unsigned walletId)
{
	Transaction* transaction = nullptr;
	long long numOfTransactions = LoadTransactions(transaction);
	if (IsError(numOfTransactions)) return -1;

	long long ordersMade = 0;

	for (size_t i = 0; i < numOfTransactions; i++)
		if (walletId == transaction[i].senderId || walletId == transaction[i].receiverId)
			++ordersMade;

	return ordersMade - 1; // заради системната транзакция
}

bool AddTransactionsInfoToTxt(const char* filename, double coinsCirculated)
{
	ifstream file(filename);
	if (!file.is_open())
	{
		cerr << "Couldn't open file: " << ORDERS_PATH << endl;
		return false;
	}

	int numOfTr = 0;
	char check;
	while (!file.eof())
	{
		file.get(check);
		if (!file && !file.eof())
		{
			file.close();
			return false;
		}
		if (check == '\n') ++numOfTr;
	}
	file.clear();
	file.close();
	--numOfTr;

	ofstream fileWrite(filename, ios::app);
	if (!fileWrite.is_open())
	{
		cerr << "Couldn't open file: " << ORDERS_PATH << endl;
		return false;
	}
	fileWrite << "transactions made: " << numOfTr - 1 << endl;
	fileWrite << "value of transactions " << coinsCirculated << endl;
	if (!file)
	{
		fileWrite.close();
		return false;
	}
	fileWrite.close();
	return true;

}