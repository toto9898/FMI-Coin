#ifndef WALLETS_H
#define WALLETS_H


struct Wallet {
	char owner[256];
	unsigned id;
	double fiatMoney;
};

long long NumOfWallets();
bool AddWallet();
bool GetWallet(Wallet& wallet, const unsigned walletId);
bool RefreshWallet(Wallet& wallet); // обновява протмонето във файла
bool GetID(Wallet& wallet); // дава id на новороденото протмоне
bool WalletInfo(unsigned walletId);
double GetCoinsOnHold(unsigned walletId); // връща монетите, които са на борсата
double WalletCoins(unsigned walletId);// връща монетите с които даденото протмоне разполага

#endif // !WALLETS_H
