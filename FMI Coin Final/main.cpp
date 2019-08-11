/*
	Име: Тодор Тодоров
	Ф-н: 45428

	Има няколко допълнителни команди за удобство
	Трябва да се създаде папка "DataFiles" и в нея папка "orders",
	за да работи запазването на файловете
	
	attract-investors не работи до край
*/

#include <iostream>
#include <ctime>
#include "wallets.h"
#include "shared.h"
#include "transactions.h"
#include "orders.h"
using namespace std;
const char*  const commands[] = { "add-wallet", "make-order", "quit", "wallet-info", "attract-investors", "clear-files", "print-orders"};
const char* const BS[] = { "SELL", "BUY" };
short GetCommand();

int main()
{
		
	while(true)	
	{
		int commandID = -1;
		unsigned walletId;
		
		do
		{
			commandID = GetCommand();
		} while (commandID < 0);

		switch (commandID)
		{
		case 0:
			if (!AddWallet())
				return 1;
			break;
		case 1:
			char secCommand[10];
			cin >> secCommand;
			Order::Type type;

			if (!stricmp(secCommand, BS[0]))
				type = Order::Type::SELL;
			else if (!stricmp(secCommand, BS[1]))
				type = Order::Type::BUY;
			else
			{
				cerr << "Wrong command" << endl;
				break;
			}
			double fmiCoins;
			cin >> fmiCoins;
			cin >> walletId;

			Order order;
			if (!MakeOrder(order, type, walletId, fmiCoins))
				break;
			if (!Fulfill(order, time(0)))
				return -1;

			break;
		case 2:
			return 0;

		case 3:
			unsigned int ID;
			cin >> ID;
			if (!WalletInfo(ID))
				return -1;
			break;
		case 5:
			if (!ClearFiles())
				return -1;
			break;
		case 6:
			if (!PrintOrders())
				cerr << "Can't load orders" << endl;
			break;
		}

		std::cout << endl;
	}




	return 0;
}

short GetCommand()
{
	char command[32];
	//cin >> command;

	GetCommand(command, 32);

	for (short i = 0; i < 7; i++)
		if (!stricmp(commands[i], command))
			return i;

	cerr << "There is no such command" << endl;
	return -1;
}
