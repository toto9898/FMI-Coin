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
bool RefreshWallet(Wallet& wallet); // �������� ���������� ��� �����
bool GetID(Wallet& wallet); // ���� id �� ������������ ��������
bool WalletInfo(unsigned walletId);
double GetCoinsOnHold(unsigned walletId); // ����� ��������, ����� �� �� �������
double WalletCoins(unsigned walletId);// ����� �������� � ����� �������� �������� ���������

#endif // !WALLETS_H
