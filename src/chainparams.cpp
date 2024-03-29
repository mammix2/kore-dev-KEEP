// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include "arith_uint256.h"
#include "kernel.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
* Build the genesis block. Note that the output of the genesis coinbase cannot
* be spent as it did not originally exist in the database.
*
* CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
*   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
*     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
*     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
*   vMerkleTree: e0028e
*/
static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript &genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBirthdayA, uint32_t nBirthdayB, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    
    txNew.nTime = nTime;
    txNew.nVersion = 1;

    if ( pszTimestamp != NULL )
      txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    else txNew.vin[0].scriptSig = CScript() << 0 << OP_0;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nBirthdayA   = nBirthdayA;
    genesis.nBirthdayB   = nBirthdayB;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    
    return genesis;
}

/*
void CChainParams::MineNewGenesisBlock()
{
    printf("Mining genesis block...\n");

    // deliberately empty for loop finds nonce value.
    for (genesis.nNonce = 0; genesis.GetHash() > bnProofOfWorkLimit; genesis.nNonce++) 
    {
        printf("Trying with this nNonce = %u \n", genesis.nNonce);
    }
    printf("genesis.nTime = %u \n", genesis.nTime);
    printf("genesis.nNonce = %u \n", genesis.nNonce);
    printf("genesis.nBirthdayA: %d\n", genesis.nBirthdayA);
    printf("genesis.nBirthdayB: %d\n", genesis.nBirthdayB);
    printf("genesis.nBits = %x \n", genesis.nBits);
    printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
    printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}
*/


/*

This is how PTS mine genesis block
// This will figure out a valid hash and Nonce if you're
            // creating a different genesis block:
            uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
            uint256 thash;
            block.nNonce = 0;
    
            while(true)
            {
                int collisions=0;
                thash = block.CalculateBestBirthdayHash(collisions);
                if (thash <= hashTarget)
                    break;
                printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(),
                    hashTarget.ToString().c_str());
                ++block.nNonce;
                if (block.nNonce == 0)
                {
                    printf("NONCE WRAPPED, incrementing time\n");
                    ++block.nTime;
                }
            }
            printf("block.nTime = %u \n", block.nTime);
            printf("block.nNonce = %u \n", block.nNonce);
            printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
            printf("block.nBits = %u \n", block.nBits);
            printf("block.nBirthdayA = %u \n", block.nBirthdayA);
            printf("block.nBirthdayB = %u \n", block.nBirthdayB);

*/

void CChainParams::MineNewGenesisBlock()
{
    fPrintToConsole = true;
    printf("Mining genesis block...\n");


    arith_uint256 thash;
    genesis.nNonce = 0;
    //uint256 thash;
    //uint256 hashTarget;
    //hashTarget.SetCompact(genesis.nBits);
    // this is the problem ?? getuint256()
    //uint256 hashTarget = uint256().SetCompact(genesis.nBits).getuint256();
    arith_uint256 hashTarget = UintToArith256(bnProofOfWorkLimit);

    while (true) {
        //thash = genesis.GetHash_Legacy();

        // Testing yescript. This is just for testnet
        //thash = UintToArith256(genesis.GetHash_Legacy());
        thash = UintToArith256(genesis.GetHash());
        printf("genesis.nNonce = %u \n", genesis.nNonce);
        printf("teHash      %s\n", thash.ToString().c_str());
        printf("Hash Target %s\n", hashTarget.ToString().c_str());
        printf("genesis.nBirthdayA: %d\n", genesis.nBirthdayA);
        printf("genesis.nBirthdayB: %d\n", genesis.nBirthdayB);
        if (thash <= hashTarget)
            break;
        ++genesis.nNonce;
        if (genesis.nNonce == 0) {
            printf("NONCE WRAPPED, incrementing time\n");
            ++genesis.nTime;
        }
    }

    printf("Here is the genesis block \n");
    printf("genesis.nTime  = %u \n", genesis.nTime);
    printf("genesis.nNonce = %u \n", genesis.nNonce);
    printf("genesis.nBirthdayA: %d\n", genesis.nBirthdayA);
    printf("genesis.nBirthdayB: %d\n", genesis.nBirthdayB);
    printf("genesis.nBits          = %x \n", genesis.nBits);
    printf("genesis.GetHash        = %s\n", genesis.GetHash().ToString().c_str());
    printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
            boost::assign::map_list_of
            (0, uint256S("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99"))
            (5, uint256S("0x00eaaa465402e6bcf745c00c38c0033a26e4dea19448d9109e4555943d677a31"))
            (1000, uint256S("0x2073f0a245cedde8344c2d0b48243a58908ffa50b02e2378189f2bb80037abd9")) // ,last PoW block, begin of PoS
            (40000, uint256S("0x572b31cc34f842aecbbc89083f7e40fff6a07e73e6002be75cb95468f4e3b4ca"))
            (80000, uint256S("0x070aa76a8a879f3946322086a542dd9e4afca81efafd7642192ed9fe56ba74f1"))
            (120000, uint256S("0x70edc85193638b8adadb71ea766786d207f78a173dd13f965952eb76932f5729"))
            (209536, uint256S("0x8a718dbb44b57a5693ac70c951f2f81a01b39933e3e19e841637f757598f571a"));
        static const Checkpoints::CCheckpointData data = {
            &mapCheckpoints,
            1529501760, // * UNIX timestamp of last checkpoint block
            422518,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            1440        // * estimated number of transactions per day after checkpoint
        };

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (0, uint256("0x000ce3b76d9435adbc2713c62239cea20fe6bf0f69ed4d4f5c95ef07018a0450"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1533841307,
    0,
    250};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};


class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xe4;
        pchMessageStart[1] = 0x7b;
        pchMessageStart[2] = 0xb3;
        pchMessageStart[3] = 0x4a;
        vAlertPubKey = ParseHex("042b0fb78026380244cc458a914dae461899b121f53bc42105d134158b9773e3fdadca67ca3015dc9c4ef9b9df91f2ef05b890a15cd2d2b85930d37376b2196002");
        nDefaultPort = 10743;
        //consensus.powLimit = uint256S("1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        bnProofOfWorkLimit = ~uint256(0) >> 3;
        //consensus.posLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	    bnProofOfStakeLimit = ~uint256(0) >> 16;
        nSubsidyHalvingInterval = 4000;
        nMaxReorganizationDepth = 25;        
        nEnforceBlockUpgradeMajority = 750; // consensus.nMajorityEnforceBlockUpgrade = 750;        
        nRejectBlockOutdatedMajority = 950; // consensus.nMajorityRejectBlockOutdated = 950;        
        nToCheckBlockUpgradeMajority = 1000;// consensus.nMajorityWindow = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60;
        nTargetSpacing = 1 * 60;
        nStakeTargetSpacing = 60; // stake every 1 hour
        nPastBlocksMin = 24; 
        nPastBlocksMax = 24;
        nStakeMinAge = 60*60; // it will stake after 1 hour
        nModifier = MODIFIER_INTERVAL_TESTNET;
        nClientMintibleCoinsInterval =  5 * 60;
        nClientMintibleCoinsInterval =  1 * 60;
        nMaturity = 25;
        nMasternodeCountDrift = 20;          // ONLY KORE
        nMaxMoneyOut = 12000000 * COIN;

        /** Height or Time Based Activations **/
        nLastPOWBlock = 1000;
        nModifierUpdateBlock = 615800;
#ifdef ZEROCOIN        
        nZerocoinStartHeight = 863787;
        nZerocoinStartTime = 1508214600; // October 17, 2017 4:30:00 AM
        nBlockZerocoinV2 = 1153160; //!> The block that zerocoin v2 becomes active - roughly Tuesday, May 8, 2018 4:00:00 AM GMT
        nInvalidAmountFiltered = 268200*COIN; //Amount of invalid coins filtered through exchanges, that should be considered valid        
#endif        
        nBlockEnforceSerialRange = 895400; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 908000; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 891737; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 891730; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 902850; //Start enforcing the invalid UTXO's

        nEnforceNewSporkKey = 1525158000; //!> Sporks signed after (GMT): Tuesday, May 1, 2018 7:00:00 AM GMT must use the new spork key
        nRejectOldSporkKey = 1527811200; //!> Fully reject old spork key after (GMT): Friday, June 1, 2018 12:00:00 AM
        CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        //        CreateGenesisBlock(nTime,      nNonce, nBirthdayA,  nBirthdayB,  nBits,  nVersion,  genesisReward)
        // genesis for Kore - using momentum
	    // genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1508884606, 22      , 12624920,   58284520,   0x201fffff, 1,  pow (7,2) * COIN);
        genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1508884606, 3      , 12624920,   58284520,   0x201fffff, 1,  pow (7,2) * COIN);
        hashGenesisBlock = genesis.GetHash();
        LogPrintf("%s", hashGenesisBlock.ToString());
        genesis.print();
        //assert(hashGenesisBlock == uint256("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99"));
        assert(hashGenesisBlock == uint256("0x153544fcc00d0b231cade18b7cf5fe0c7b7fc2d0429a79bc6f972778683e9e18"));
        if (false)
          MineNewGenesisBlock();
        assert(genesis.hashMerkleRoot == uint256("0x53e2105c87e985ab3a3a3b3c6921f660f18535f935e447760758d4ed7c4c748c"));
        // Primary DNS Seeder 
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-1", "dnsseed.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-2", "dnsseed2.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-3", "dnsseed3.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-4", "dnsseed4.kore.life"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,45);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,85);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        // Lico - Verify if it is necessary
        // 	BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        //base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strDevFundPubKey = "04D410C4A7FEC6DBF6FEDC9721104ADA1571D5E3E4791085EFC083A9F3F4C007D240A6A647DDA0CA1466641B0739A86A67B97AC48484FC7CA88257804B7CE52ED2";
        strSporkKey = "0427E31B51989DB4DFEAB8C3901FB1862A621E6B0D4CF556E5C9AAD7283A46C915EC4508FB4F248534C3A03FC0475ED3785086B9C217E0F42ED4C8BF80ED2296C8";
        strObfuscationPoolDummyAddress = "KWFvN4Gb55dzG95cq3k5jXFmNVkJLftyjZ";
        nStartMasternodePayments = 1508884606; //Genesis time
        nBudgetVoteUpdate = 60*60;     // can only change vote after 1 hour

#ifdef ZEROCOIN
        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        nZerocoinHeaderVersion = 4; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zkore to be stakable
#endif

        nBudgetFeeConfirmations = 6; // Number of confirmations for the finalization fee

        nMasternodeMinConfirmations = 15;
        nMasternodeMinMNPSeconds = 10 * 60;  
        nMasternodeMinMNBSeconds =  5 * 60;   
        nMasternodePingSeconds = 5 * 60;
        nMasternodeExpirationSeconds = 120 * 60;
        nMasternodeRemovalSeconds = 130 * 60;
        nMasternodeCheckSeconds = 5;
        nMasternodeCoinScore = 499;
        // Amount of blocks in a months period of time
        nMasternodeBudgetPaymentCycle = 43200; // (using 1 minutes per) = (60*24*30)
        // Submit final budget during the last 2 days (2880 blocks) before payment for Mainnet
        nMasternodeFinalizationWindow = ((MasternodeBudgetPaymentCycle() / 30) * 2);        
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x45;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xba;        
        vAlertPubKey = ParseHex("0469a7d953bb8c51875585c3fc20111962b741ec31fa1bbe9e85f0a26a0a425d42e51cd3535b062bf727b41d6733f3ab867774c57fdfadd601436202a412227a4e");
        nDefaultPort = 11743;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; // KORE: 1 minute
        nTargetSpacing = 1 * 30;  // KORE: 30 seconds
        nStakeTargetSpacing = 10; // stake every 10 minutes
        nPastBlocksMin = 64;
        nPastBlocksMax = 64;
        nStakeMinAge = 30*60; // it will stake after 30 minutes
        nModifier = MODIFIER_INTERVAL_TESTNET;
        nClientMintibleCoinsInterval =  10; // every 10 seconds
        nClientMintibleCoinsInterval =  2; // additional 2 seconds
        fSkipProofOfWorkCheck = false;
        bnProofOfWorkLimit = ~uint256(0) >> 3;
        //consensus.posLimit = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	    bnProofOfStakeLimit = ~uint256(0) >> 4;

        
        nLastPOWBlock = 1000;
        nMaturity = 1; // will mature in the next block.
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
#ifdef ZEROCOIN        
        nZerocoinStartHeight = 201576;
        nZerocoinStartTime = 1501776000;
        nBlockZerocoinV2 = 444020; //!> The block that zerocoin v2 becomes active        
        nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid        
#endif        
        nBlockEnforceSerialRange = 1; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 9908000; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 9891737; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 9891730; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 9902850; //Start enforcing the invalid UTXO's
        nEnforceNewSporkKey = 1521604800; //!> Sporks signed after Wednesday, March 21, 2018 4:00:00 AM GMT must use the new spork key
        nRejectOldSporkKey = 1522454400; //!> Reject old spork key after Saturday, March 31, 2018 12:00:00 AM GMT

        // sending rewards to this public key            
        CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        const char* pszTimestamp = "https://bitcoinmagazine.com/articles/altcoins-steal-spotlight-bitcoin-reaches-new-highs/";
        // genesis when using momentum
        //genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1533841307, 22 , 21828300, 63688767, 0x201fffff, 1, 49 * COIN);
        // genesis for momentum
        //genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1533841307, 22 , 21828300, 63688767, 0x201fffff, 1, 49 * COIN);
        // genesis for yescrytR32
        genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1533841307, 7 , 21828300, 63688767, 0x201fffff, 1, 49 * COIN);
        printf("hashMerkleRoot for TestNet: %s \n",genesis.hashMerkleRoot.ToString().c_str());
        assert(genesis.hashMerkleRoot == uint256("0x73bf9a836ff7c2fc79445a622ce5154bfde2811c57c397d6a3909bc97390174a"));
        // Activate only when creating a new genesis block
        if (false)
            MineNewGenesisBlock();
        hashGenesisBlock = genesis.GetHash();
        printf("hashGenesisBlock for TestNet: %s \n",hashGenesisBlock.ToString().c_str());
        // for momentum
        //assert(hashGenesisBlock == uint256("0x0ba6c4977e9da831f14ff9180b37e30a96e78cb3581029907eb24c55cf079fc5"));
        assert(hashGenesisBlock == uint256("0x0d7edba948672b6444b96155b79c22fc4da6dd1014a5f3a148594c60a12def23"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "kore-testnet.seed.fuzzbawls.pw"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,105);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,190);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,233);
        // Kore BIP32 pubkeys
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Kore BIP32 prvkeys
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        // Kore BIP44
        // base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        strDevFundPubKey = "04fb16faf70501f5292a630bced3ec5ff4df277d637e855d129896066854e1d2c9d7cab8dbd5b98107594e74a005e127c66c13a918be477fd3827b872b33d25e03";
        strSporkKey      = "04ca99e36f198eedd11b386cf2127a036ec1f0028c2b2a5ec0ff71aa2045c1c4494d45013467a5653eb64442a4d8f93ca62e00f5d9004a3a6469e72b8516ed4a99";
        strObfuscationPoolDummyAddress = "jPt4RY7Nfs5XCWqCBmmDWAUza475KR42iU";
        nStartMasternodePayments = 1533841307; //genesis block time
        nBudgetFeeConfirmations = 2; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet

        nMasternodeMinConfirmations = 6;    
        nMasternodeMinMNPSeconds = 2 * 60;
        nMasternodeMinMNBSeconds =  1 * 60;
        nMasternodePingSeconds = 1 * 60;
        nMasternodeExpirationSeconds = 24 * 60;
        nMasternodeRemovalSeconds = 26 * 60;
        nMasternodeCheckSeconds = 1;
        nMasternodeCoinScore = 499;
        // a superblock will have 140 cycle
        nMasternodeBudgetPaymentCycle = 30; // every 60 blocks, it will check if it is necessary to pay
        nMasternodeFinalizationWindow = 15; // 13 + 1 finalization confirmations + 1 minutes buffer for propagation

        nBudgetVoteUpdate = 1*60;     // can only change vote after 1 minute
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xcf;
        pchMessageStart[1] = 0x05;
        pchMessageStart[2] = 0x6a;
        pchMessageStart[3] = 0xe1;
        nSubsidyHalvingInterval = 150;       // consensus.nSubsidyHalvingInterval
        nEnforceBlockUpgradeMajority = 750;  // consensus.nMajorityEnforceBlockUpgrade
        nRejectBlockOutdatedMajority = 950;  // consensus.nMajorityRejectBlockOutdated
        nToCheckBlockUpgradeMajority = 1000; // consensus.nMajorityWindow
        nMinerThreads = 1;
        nTargetTimespan = 60 * 60; // consensus.nTargetTimespan one hour
        nTargetSpacing = 1 * 60;   // consensus.nTargetSpacing 1 minutes        
        bnProofOfWorkLimit = ~uint256(0) >> 1; // this make easier to find a block !        
        genesis.nTime = 1453993470;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 12345;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 18444;
        // TODO Lico removed assertion
        //assert(hashGenesisBlock == uint256("0x4f023a2120d9127b21bbad01724fdb79b519f593f2a85b60d3d79160ec5f29df"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

    	base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,105);
	    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,190);
	    base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,233);
    	// Kore BIP32 pubkeys
    	base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
	    // Kore BIP32 prvkeys
    	base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
    	// Kore BIP44
    	// base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }

};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 51478;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) { nSubsidyHalvingInterval = anSubsidyHalvingInterval; }
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
