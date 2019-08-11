#include <iostream>
#include <cmath>
#include <fstream>
#include "shared.h"
#include "wallets.h"
#include "transactions.h"
using namespace std;
const double EPS = 0.0001;

void GetCommand(char* cmd, const unsigned size)
{
	char c = ' ';
	while (c == ' ' || c == '\n')
	{
		c = getchar();
	}

	unsigned i = 0;
	for (i = 0; i < size - 1 && c != ' ' && c != '\n'; i++)
	{
		cmd[i] = c;
		c = getchar();
	}
	cmd[i] = '\0';

	while (c != ' ' && c != '\n')
	{
		c = getchar();
	}
}

bool fileExists(const char* const filename)
{
	ifstream f(filename);
	bool exists = f.good();
	f.close();
	return exists;
}

double ConvertTo(const double amount, bool ToFmiCoins)
{
	if (ToFmiCoins)
		return amount / COIN_EXCH_RATE;
	return amount * COIN_EXCH_RATE;
}

bool IsError(const long long input)
{
	if (input > FIRST_ERROR && input < LAST_ERROR)
		return true;
	return false;
	}

double Sub(const double num1, const double num2)
{
	if (fabs(num1 - num2) < EPS)
		return 0.0;
	return num1 - num2;
}

bool ClearFiles()
{
	ofstream file(WALLET_PATH);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open file: " << WALLET_PATH << endl;
		return false;
	}
	file.close();

	ofstream file1(ORDERS_PATH);
	if (!file1.is_open())
	{
		std::cerr << "Couldn't open file: " << ORDERS_PATH << endl;
		return false;
	}
	file1.close();

	ofstream file2(TRANSACTIONS_PATH);
	if (!file2.is_open())
	{
		std::cerr << "Couldn't open file: " << TRANSACTIONS_PATH << endl;
		return false;
	}
	file2.close();

	ofstream file3(THE_RICHEST_PATH);
	if (!file3.is_open())
	{
		std::cerr << "Couldn't open file: " << THE_RICHEST_PATH << endl;
		return false;
	}
	file3.close();

	return true;
}

void NumToStr(char numStr[21], unsigned long long num)
{
	long long tempNum = num;
	short numLen = 0;
	while (tempNum)
	{
		tempNum /= 10;
		++numLen;
	}
	for (int i = numLen - 1; i >= 0; i--)
	{
		numStr[i] = num % 10 + '0';
		num /= 10;
	}
	numStr[numLen] = '\0';
}

void CreateFileName(char filename[55], unsigned id, unsigned long long time)
{
	strncpy(filename, "DataFiles\\orders\\", 20);
	char idStr[11];
	NumToStr(idStr, id);
	strcat(filename, idStr);
	strcat(filename, "-");
	char now[21];
	NumToStr(now, time);

	strcat(filename, now);
	strcat(filename, ".txt");
}

long long NumOfInvestors()
{
	ifstream file;
	file.open(THE_RICHEST_PATH, ios::binary | ios::in);
	if (!file)
	{
		return 0;
	}

	file.seekg(0, ios::end);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << THE_RICHEST_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	unsigned long long fileSize = file.tellg();
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << THE_RICHEST_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.seekg(0, ios::beg);
	if (!file)
	{
		cerr << "Failed to set pointer in file: " << THE_RICHEST_PATH << endl;
		file.close();
		return CANT_SET_POINTER_IN_FILE;
	}

	file.close();

	return fileSize / sizeof(AttractInv);
}

long long Min(long long num1, long long num2)
{
	return num1 < num2 ? num1 : num2;
}

long long LoadInvestors(AttractInv *& investor)
{
	/* Loads the transaction from the file and returns their amount*/

	long long numOfInvestors = NumOfInvestors();
	if (IsError(numOfInvestors))
		return numOfInvestors;
	if (numOfInvestors == 0)
		return 0;

	investor = new(std::nothrow) AttractInv[numOfInvestors];
	if (!investor)
	{
		cerr << "Failed to allocate space for file content: " << TRANSACTIONS_PATH << endl;
		return MEMORY_ALLOC_ERROR;
	}

	ifstream file;
	file.open(THE_RICHEST_PATH, ios::binary | ios::in);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << THE_RICHEST_PATH << endl;
		delete[] investor;
		return FILE_DIDNT_OPEN;
	}

	for (long long i = 0; i < numOfInvestors; i++)
	{
		file.read((char*)&investor[i], sizeof(AttractInv));
		if (file.bad())
		{
			std::cerr << "Failed reading from file: " << THE_RICHEST_PATH << std::endl;
			file.close();
			delete[] investor;
			return READING_FAILED;
		}
	}
	file.close();

	return numOfInvestors;
}

bool RefreshInvestor(AttractInv& investor, short investorIndex)
{
	if (investorIndex == -1) 
	{
		ifstream fileRead(THE_RICHEST_PATH, ios::binary | ios::in);
		if (!fileRead.is_open())
		{
			cerr << "Couldn't open file: " << THE_RICHEST_PATH << endl;
			return false;
		}

		AttractInv tempInv;
		size_t i = 0;
		for (i = 0; !fileRead.eof(); i++)
		{
			fileRead.read((char*)&tempInv, sizeof(AttractInv));
			if (!fileRead)
			{
				cerr << "Failed to read from file: " << THE_RICHEST_PATH << endl;
				fileRead.close();
				return false;
			}
			if (tempInv.time1stTr == investor.time1stTr)
			{
				investorIndex = i;
				break;
			}
		}
		if (investorIndex == -1)
		{
			fileRead.clear();
			cerr << "Can't find the investor to refresh it." << endl;
			fileRead.close();
			return false;
		}
		if (fileRead.eof()) fileRead.clear();
		fileRead.close();
	}

	ofstream fileWrite(THE_RICHEST_PATH, ios::binary | ios::in | ios::out | ios::ate);
	if (!fileWrite.is_open())
	{
		cerr << "Couldn't open file: " << THE_RICHEST_PATH << endl;
		return false;
	}
	fileWrite.seekp(investorIndex * sizeof(AttractInv), ios::beg);
	if (!fileWrite)
	{
		cerr << "Failed to set pointer in file: " << THE_RICHEST_PATH << endl;
		fileWrite.close();
		return false;
	}
	fileWrite.write((const char*)&investor, sizeof(AttractInv));
	if (!fileWrite)
	{
		cerr << "Failed to write to file: " << THE_RICHEST_PATH << endl;
		fileWrite.close();
		return false;
	}

	fileWrite.close();
	return true;
}

short PoorestInvestorID(AttractInv*& investor, long long numOfInvestors)
{
	short index = -1;
	double min = investor[0].fmiCoins;
	for (short i = 0; i < numOfInvestors; i++)
	{
		if (investor[i].fmiCoins < min)
		{
			min = investor[i].fmiCoins;
			index = i;
		}
	}
	return index;
}

bool AddInvestor(AttractInv& investor)
{
	AttractInv* investors = nullptr;

	long long numOfInvestors = LoadInvestors(investors);
	if (IsError(numOfInvestors <= 0)) return false;

	for (size_t i = 0; i < numOfInvestors; i++)
	{
		if (investor.time1stTr == investors[i].time1stTr)
		{
			if (!RefreshInvestor(investor))return false;
			delete[] investors;
			return true;
		}
	}

	if (numOfInvestors < 10)
	{
		ofstream file(THE_RICHEST_PATH, ios::app | ios::binary);
		if (!file.is_open())
		{
			cerr << "Couldn't open file: " << THE_RICHEST_PATH << endl;
			delete[] investors;
			return false;
		}

		file.write((const char*)&investor, sizeof(AttractInv));
		if (!file)
		{
			cerr << "Failed to write to file: " << THE_RICHEST_PATH << endl;
			file.close();
			delete[] investors;
			return false;
		}

		file.close();
		delete[] investors;
		return true;
	}

	short poorestInvestorID = PoorestInvestorID(investors, numOfInvestors);
	if (poorestInvestorID < 0)
	{
		delete[] investors;
		return false;
	}
	if (!RefreshInvestor(investor, poorestInvestorID))
	{
		delete[] investors;
		return false;
	}
	delete[] investors;
	return true;
}

bool TheInvestors()
{
	long long numOfInvestors = NumOfInvestors();
	if (IsError(numOfInvestors) || numOfInvestors == 0) return false;

	if (numOfInvestors < 10)
	{
//		if (!AddInvestor(investor));
	}


	/*Transaction* transaction;
	long long numOfTransactions = LoadTransactions(transaction);
	if (IsError(numOfTransactions))
		return false;
	if (numOfTransactions == 0)
		return false;

	Transaction tempTr;
	long long swapIndex = 0;

	for (long long i = 0; i < 10; i++)
	{
		tempTr = transaction[i];
		
		for (long long j = i + 1; j < numOfTransactions;  j++)
			if (transaction[j].fmiCoins > transaction[i].fmiCoins)
				swapIndex = j;

		transaction[i] = transaction[swapIndex];
		transaction[swapIndex] = tempTr;
	}

	long long printTo = Min(10, numOfTransactions);
	for (size_t i = 0; i < printTo; i++)
	{
		double coins = WalletCoins(transaction[i].)
		cout << "#" << i + 1 << ":  "
	}*/


	return true;
}

