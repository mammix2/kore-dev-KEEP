// Copyright (c) 2015-2016 The Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pos.h"
#include "script/interpreter.h"
#include "arith_uint256.h"

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.

uint256 ComputeStakeModifier_Legacy(const CBlockIndex* pindexPrev, const uint256& kernel)
{
    if (!pindexPrev)
        return uint256(); // genesis block's modifier is 0

    CHashWriter ss(SER_GETHASH, 0);
    ss << kernel << pindexPrev->nStakeModifierOld;
    return ss.GetHash();
}

// BlackCoin kernel protocol v3
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.nTime + txPrev.vout.hash + txPrev.vout.n + nTime) < bnTarget * nWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coins one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: scrambles computation to make it very difficult to precompute
//                   future proof-of-stake
//   txPrev.nTime: slightly scrambles computation
//   txPrev.vout.hash: hash of txPrev, to reduce the chance of nodes
//                     generating coinstake at the same time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   nTime: current timestamp
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.

bool CheckStakeKernelHash(const CBlockIndex* pindexPrev, unsigned int nBits, 
                          const CCoins* txPrev, const COutPoint& prevout, unsigned int nTimeTx,
                          uint256& hashProofOfStake)
{
    // Weight
    int64_t nValueIn = txPrev->vout[prevout.n].nValue;
    if (nValueIn == 0)
        return false;

    // Base target
    arith_uint256 bnTarget = arith_uint256().SetCompact(nBits);

    if(fDebug) {
      LogPrintf("CheckStakeKernelHash: pindexPrev->nStakeModifier:%u txPrev->nTime:%u \n", pindexPrev->nStakeModifier, txPrev->nTime);
      LogPrintf("CheckStakeKernelHash: prevout.hash: %s prevout.n:%u nTimeTx:%u \n", prevout.hash.GetHex(), prevout.n, nTimeTx);
    }

    // Calculate hash
    CHashWriter ss(SER_GETHASH, 0);
    ss << pindexPrev->nStakeModifierOld << txPrev->nTime << prevout.hash << prevout.n << nTimeTx;
    hashProofOfStake = ss.GetHash();

    if(fDebug) LogPrintf("CheckStakeKernelHash: hashProofOfStake:%s nValueIn:%d target: %x \n", hashProofOfStake.GetHex(), nValueIn, bnTarget.GetCompact());

    // Now check if proof-of-stake hash meets target protocol
    if (UintToArith256(hashProofOfStake) / nValueIn > bnTarget){
		if(fDebug)LogPrintf("CheckStakeKernelHash() : hash does not meet protocol target   previous = %x  target = %x\n", nBits, bnTarget.GetCompact());
        return false;
	}

    return true;
}

bool CheckKernel(CBlockIndex* pindexPrev, unsigned int nBits, int64_t nTime, const COutPoint& prevout, int64_t* pBlockTime)
{
    uint256 hashProofOfStake, targetProofOfStake;

    CTransaction prevtx;
    uint256 hashBlock;
    if (!GetTransaction(prevout.hash, prevtx, hashBlock, true))
        return false;

    CBlockIndex* pIndex = NULL;
    BlockMap::iterator iter = mapBlockIndex.find(hashBlock);
    if (iter != mapBlockIndex.end()) 
        pIndex = iter->second;

    // Read block header
    CBlock block;
    if (!ReadBlockFromDisk(block, pIndex))
        return false;

    // Maturity requirement
    if (pindexPrev->nHeight - pIndex->nHeight < STAKE_MIN_CONFIRMATIONS)
        return false;

    if (pBlockTime)
        *pBlockTime = block.GetBlockTime();

    // Min age requirement
    if (prevtx.nTime + Params().StakeMinAge() > nTime) // Min age requirement
        return false;

    return CheckStakeKernelHash(pindexPrev, nBits, new CCoins(prevtx, pindexPrev->nHeight), prevout, nTime, hashProofOfStake);
}


/*
  This method is what kore is using currently which is now being changed 
  to the method in the kernel.cpp
*/
// Check kernel hash target and coinstake signature
bool CheckProofOfStake_Legacy(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits,
                           uint256& hashProofOfStake, std::unique_ptr<CStakeInput>& stake)
{
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s \n", tx.GetHash().ToString());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx.vin[0];

    // First try finding the previous transaction in database
    CTransaction prevtx;
    uint256 hashBlock;

    if (!GetTransaction(txin.prevout.hash, prevtx, hashBlock, true))
       return error("CheckProofOfStake() : INFO: read txPrev failed");

    // Verify signature
    if (!VerifySignature(prevtx, tx, 0, SCRIPT_VERIFY_NONE, 0)) {
       return error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str());
    }

    //Construct the stakeinput object
    CkoreStake* koreInput = new CkoreStake();
    koreInput->SetInput(prevtx, txin.prevout.n);
    stake = std::unique_ptr<CStakeInput>(koreInput);

    CBlockIndex* pIndex = NULL;
    BlockMap::iterator iter = mapBlockIndex.find(hashBlock);
    if (iter != mapBlockIndex.end()) 
        pIndex = iter->second;

    // Read block header
    CBlock block;
    if (!ReadBlockFromDisk(block, pIndex))
       return error("CheckProofOfStake(): *** ReadBlockFromDisk failed at %d, hash=%s \n", pindexPrev->nHeight, pindexPrev->GetBlockHash().ToString());

    // Min age requirement
    if (pindexPrev->nHeight - pIndex->nHeight < STAKE_MIN_CONFIRMATIONS)
       return error("%s: tried to stake at depth %d \n", __func__, pindexPrev->nHeight - pIndex->nHeight);

    if (prevtx.nTime + STAKE_MIN_AGE > tx.nTime) // Min age requirement
        return error("CheckProofOfStake() : min age violation - nTimeBlockFrom=%d nStakeMinAge=%d nTimeTx=%d \n", prevtx.nTime, STAKE_MIN_AGE, tx.nTime);

    if (!CheckStakeKernelHash(pindexPrev, nBits, new CCoins(prevtx, pindexPrev->nHeight), 
                              txin.prevout, tx.nTime, hashProofOfStake))
       return error("%s: CheckStakeKernelHash failed \n", __func__);

    return true;
}

bool VerifySignature(const CTransaction& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType)
{
    assert(nIn < txTo.vin.size());
    const CTxIn& txin = txTo.vin[nIn];
    if (txin.prevout.n >= txFrom.vout.size())
        return false;
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    if (txin.prevout.hash != txFrom.GetHash())
        return false;

    return VerifyScript(txin.scriptSig, txout.scriptPubKey, flags, TransactionSignatureChecker(&txTo, nIn),  NULL);
}
