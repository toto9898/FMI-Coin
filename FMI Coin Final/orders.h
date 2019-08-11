#ifndef ORDERS_H
#define ORDERS_H

struct Order {
	enum Type { SELL, BUY } type;
	unsigned walletId;
	double fmiCoins;
};

bool DeleteOrder(Order*& order, const unsigned id, long long& numOfOrders);
long long LoadOrders(Order *& order);
bool MakeOrder(Order& order, const Order::Type type, const unsigned walletId, const double fmiCoins);
bool AddOrder(Order& order);
bool PrintOrders();
bool SaveOrders(Order*& order, long long numOfOrders);
bool Fulfill(Order& order, const unsigned long long now, bool AddTransactionsInfoToTxt = false, double coinsCirculated = 0);
bool AddOrderToTxt(const char filename[55], Transaction& transaction, Order::Type type);
long long NumOfOrdersMade(unsigned walletId); // връща броя направени поръчки от дадено протмоне
bool AddTransactionsInfoToTxt(const char* filename, double coinsCirculated);

#endif // !ORDERS_H
