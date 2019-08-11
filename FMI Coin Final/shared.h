#ifndef SHARED_H
#define SHARED_H

enum Errors
{
	FIRST_ERROR = -10,
	READING_FAILED,
	MEMORY_ALLOC_ERROR,
	FILE_DIDNT_OPEN,
	FILE_NOT_FOUND,
	CANT_SET_POINTER_IN_FILE,
	LAST_ERROR
};

struct AttractInv
{
	double fmiCoins;
	long long numOfOrders;
	unsigned long long time1stTr;
	unsigned long long timeLastTr;
};

const char* const WALLET_PATH = "DataFiles\\wallets.dat";
const char* const TRANSACTIONS_PATH = "DataFiles\\transactions.dat";
const char* const ORDERS_PATH = "DataFiles\\orders.dat";
const char* const THE_RICHEST_PATH = "DataFiles\\the richest.dat";

const unsigned long long SYSTEM_WALLET_ID = 4294967295;
const unsigned short COIN_EXCH_RATE = 375;

void GetCommand(char* cmd, const unsigned size);
bool fileExists(const char* const filename);
double ConvertTo(const double amount, bool ToFmiCoins = true);
bool IsError(const long long input);
double Sub(const double num1, const double num2);
bool ClearFiles();
void NumToStr(char numStr[21], unsigned long long num);
void CreateFileName(char filename[55], unsigned id, unsigned long long time);
long long Min(long long num1, long long num2);
long long NumOfInvestors();
long long LoadInvestors(AttractInv *& investor);
bool RefreshInvestor(AttractInv& investor, short investorIndex = -1);  // презаписва investor според подаденото id, или обновява някой според времето на първата му транзакция
short PoorestInvestorID(AttractInv*& investor, long long numOfInvestors);
bool AddInvestor(AttractInv& investor);  // добавя investor, ако изпълнява изискванията

#endif // !SHARED_H
