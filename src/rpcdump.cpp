// Copyright (c) 2009-2014 Bitcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h" // for pwalletMain
#include "bitcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"

#include <boost/lexical_cast.hpp>

#define printf OutputDebugStringF

using namespace json_spirit;
using namespace std;

class CTxDump
{
public:
    CBlockIndex *pindex;
    int64 nValue;
    bool fSpent;
    CWalletTx* ptx;
    int nOut;
    CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
    {
        pindex = NULL;
        nValue = 0;
        fSpent = false;
        this->ptx = ptx;
        this->nOut = nOut;
    }
};

Value importprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importprivkey <referencelinecoinprivkey> [label] [rescan=true]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");

    EnsureWalletIsUnlocked();

    string strSecret = params[0].get_str();
    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");

    CKey key = vchSecret.GetKey();
    CPubKey pubkey = key.GetPubKey();
    CKeyID vchAddress = pubkey.GetID();
    if (!key.IsValid()) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();
        pwalletMain->GetKeyFromPool(temppubkeyForBitCoinAddress,false);//Get new PubKey for encryption of the reference line
        CBitcoinAddress addr;
        addr.Set(vchAddress,temppubkeyForBitCoinAddress);
        pwalletMain->SetAddressBookName(addr, strLabel);

        if (!pwalletMain->AddKeyPubKey(key, pubkey))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");

        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}

Value importprivkey2(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "importprivkey2 <referencelinecoinprivkey> <referencelinecoinprivkey2> [label] [rescan=true]\n"
            "Adds both private keys (as returned by dumpprivkey and dumpprivkey2) to your wallet.");

    EnsureWalletIsUnlocked();

    string strSecret = params[0].get_str();
    string strSecret2 = params[1].get_str();
    string strLabel = "";
    if (params.size() > 2)
        strLabel = params[2].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 3)
        fRescan = params[3].get_bool();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    CBitcoinSecret vchSecret2;
    bool fGood2 = vchSecret2.SetString(strSecret2);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key 1 encoding");
    if (!fGood2) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key 2 encoding");

    CKey key = vchSecret.GetKey();
    CPubKey pubkey = key.GetPubKey();
    CKeyID vchAddress = pubkey.GetID();

    CKey key2 = vchSecret2.GetKey();
    CPubKey pubkey2 = key2.GetPubKey();

    if (!key.IsValid()) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();

        CBitcoinAddress addr;
        addr.Set(vchAddress,pubkey2);
        pwalletMain->SetAddressBookName(addr, strLabel);

        if (!pwalletMain->AddKeyPubKey(key, pubkey))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key 1 to wallet");

        if (!pwalletMain->AddKeyPubKey(key2, pubkey2))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key 2 to wallet");


        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
//    return HexStr(key.begin(),key.end());
}

Value importprivkey3(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 5)
        throw runtime_error(
            "importprivkey3 <referencelinecoinprivkey> <referencelinecoinprivkey2> <address> [label] [rescan=true]\n"
            "Adds both private keys (as returned by dumpprivkey and dumpprivkey2) to your wallet and checks if it belongs to the address specified.");

    EnsureWalletIsUnlocked();

    string strSecret = params[0].get_str();
    string strSecret2 = params[1].get_str();
    string straddr = params[2].get_str();
    string strLabel = "";
    if (params.size() > 3)
        strLabel = params[3].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 4)
        fRescan = params[4].get_bool();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    CBitcoinSecret vchSecret2;
    bool fGood2 = vchSecret2.SetString(strSecret2);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key 1 encoding");
    if (!fGood2) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key 2 encoding");

    CKey key = vchSecret.GetKey();
    CPubKey pubkey = key.GetPubKey();
    CKeyID vchAddress = pubkey.GetID();

    CKey key2 = vchSecret2.GetKey();
    CPubKey pubkey2 = key2.GetPubKey();

    CBitcoinAddress addr;
    addr.Set(vchAddress,pubkey2);

    if (key.getfirstbyte()==0 && addr.ToString().compare(straddr)!=0)
    {
       key.deletefirstbyte(0);
       for (int i=0;i<=255;i++){
          key.setlastbyte(i);
          pubkey = key.GetPubKey();
          vchAddress = pubkey.GetID();
          addr.Set(vchAddress,pubkey2);
          if (addr.ToString().compare(straddr)==0) break;
       }
    }  

    if (!key.IsValid()) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();

        addr.Set(vchAddress,pubkey2);
        pwalletMain->SetAddressBookName(addr, strLabel);

        if (!pwalletMain->AddKeyPubKey(key, pubkey))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key 1 to wallet");

        if (!pwalletMain->AddKeyPubKey(key2, pubkey2))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key 2 to wallet");


        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
//    return HexStr(key.begin(),key.end());
}

Value dumpprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpprivkey <referencelinecoinaddress>\n"
            "Reveals the private key corresponding to <referencelinecoinaddress>.");

    string strAddress = params[0].get_str();
    CBitcoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Referencelinecoin address");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    CKey vchSecret;
    if (!pwalletMain->GetKey(keyID, vchSecret))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
    return CBitcoinSecret(vchSecret).ToString();
}

Value dumpprivkey2(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpprivkey2 <referencelinecoinaddress>\n"
            "Reveals the second private key corresponding to <referencelinecoinaddress>. This key is used to decrypt the reference line.");

    string strAddress = params[0].get_str();
    CBitcoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Referencelinecoin address");
    CKeyID keyID;
    keyID=address.GetReceiverPubKey().GetID();
    CKey vchSecret;
    if (!pwalletMain->GetKey(keyID, vchSecret))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
    return CBitcoinSecret(vchSecret).ToString();
}

Value dumpallprivkeys(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "dumpallprivkeys\n"
            "Reveals all private keys.");

    Array ret;
    std::set<CKeyID> setAddress;
    pwalletMain->GetKeys(setAddress);

    std::set<CKeyID>::iterator mi = setAddress.begin();
        while (mi != setAddress.end())
        {
            CKey vchSecret;
            if (!pwalletMain->GetKey(*mi, vchSecret))
               continue;
            ret.push_back(CBitcoinSecret(vchSecret).ToString());    

            mi++;
        }

    return ret;

}

