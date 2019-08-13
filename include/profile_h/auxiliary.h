#ifndef AUXILIARY_H
#define AUXILIARY_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"

#define ENABLE_TIMER

//#define DBG_PRINT_ALL
//#define USE_FUTURE
// XXX: According to https://github.com/llvm-mirror/llvm/blob/6b547686c5410b7528212e898fe30fc7ee7a70a3/lib/Analysis/LoopPass.cpp,
// the loop queue that runOnLoop is called is populated in reverse program order. Assuming that runOnLoop() will execute following
// a (reverse?) program order guarantees that lpNameLevelPair2headBBnameMap is populated in program order as well, which guarantees
// that loopName2levelUnrollVecMap is populated in program order as well, which is iterated to calculate the dynamic dapataths.
// Thus, progressive trace cursor won't skip any valuable data from the trace analysis if the target loops are passed in a
// monotonically crescent vector, which is enforced in validations performed at lin-profile.cpp 
#define PROGRESSIVE_TRACE_CURSOR

#include <fstream>
#include <list>
#include <map>
#include <queue>
#include <unordered_map>

#include "profile_h/ArgPack.h"

#define BUFF_STR_SZ 1024

#define FILE_TRACE_SUFFIX "_trace.bc"
#define FILE_DYNAMIC_TRACE "dynamic_trace.gz"
#define FILE_MEM_TRACE "mem_trace.txt"
#define FILE_SUMMARY_SUFFIX "_summary.log"

// XXX: For now, I'm using the old separators as defined in the original lin-analyzer to simplify correctness
// comparison and also portability
#define LEGACY_SEPARATOR
//#define GLOBAL_SEPARATOR "~"
#define CHECK_VISITED_NODES

extern ArgPack args;
#ifdef PROGRESSIVE_TRACE_CURSOR
extern long int progressiveTraceCursor;
extern uint64_t progressiveTraceInstCount;
#endif

extern const std::string functionNameMapperMDKindName;
extern const std::string loopNumberMDKindName;
extern const std::string assignBasicBlockIDMDKindName;
extern const std::string assignLoadStoreIDMDKindName;
extern const std::string extractLoopInfoMDKindName;

bool isFunctionOfInterest(std::string key, bool isMangled = true);
bool verifyModuleAndPrintErrors(llvm::Module &M);
std::string constructLoopName(std::string funcName, unsigned loopNo, unsigned depth = ((unsigned) -1));
std::string appendDepthToLoopName(std::string loopName, unsigned depth);
std::tuple<std::string, unsigned> parseLoopName(std::string loopName);
std::tuple<std::string, unsigned, unsigned> parseWholeLoopName(std::string wholeLoopName);

unsigned nextPowerOf2(unsigned x);
uint64_t nextPowerOf2(uint64_t x);

typedef std::map<std::string, std::string> getElementPtrName2arrayNameMapTy;
extern getElementPtrName2arrayNameMapTy getElementPtrName2arrayNameMap;

#define VERBOSE_PRINT(X) \
	do {\
		if(args.verbose) {\
			X;\
		} \
	} while(false)

typedef std::map<std::string, uint64_t> wholeloopName2loopBoundMapTy;
extern wholeloopName2loopBoundMapTy wholeloopName2loopBoundMap;

namespace llvm {

/// FunctionName --> number of loops inside it
typedef std::map<std::string, uint64_t> funcName2loopNumMapTy;
extern funcName2loopNumMapTy funcName2loopNumMap;

typedef std::map<BasicBlock*, std::string> BB2loopNameMapTy;
extern BB2loopNameMapTy BB2loopNameMap;

typedef std::pair<std::string, std::string> bbFuncNamePairTy;
typedef std::pair<std::string, unsigned> lpNameLevelPairTy;
typedef std::map<std::string, unsigned> LpName2numLevelMapTy;
typedef std::map<bbFuncNamePairTy, lpNameLevelPairTy> bbFuncNamePair2lpNameLevelPairMapTy;

extern bbFuncNamePair2lpNameLevelPairMapTy bbFuncNamePair2lpNameLevelPairMap;
extern bbFuncNamePair2lpNameLevelPairMapTy headerBBFuncnamePair2lpNameLevelPairMap;
extern bbFuncNamePair2lpNameLevelPairMapTy exitBBFuncnamePair2lpNameLevelPairMap;
extern LpName2numLevelMapTy LpName2numLevelMap;

extern std::map<std::string, std::string> functionName2MangledNameMap;
extern std::map<std::string, std::string> mangledName2FunctionNameMap;

class ConfigurationManager {
public:
	typedef struct {
		std::string funcName;
		unsigned loopNo;
		unsigned loopLevel;
	} pipeliningCfgTy;
	typedef struct {
		std::string funcName;
		unsigned loopNo;
		unsigned loopLevel;
		int lineNo;
		uint64_t unrollFactor;
	} unrollingCfgTy;
	typedef struct {
		enum {
			PARTITION_TYPE_BLOCK,
			PARTITION_TYPE_CYCLIC,
			PARTITION_TYPE_COMPLETE
		};
		unsigned type;
		std::string baseAddr;
		uint64_t size;
		size_t wordSize;
		uint64_t pFactor;
	} partitionCfgTy;
	typedef std::unordered_map<std::string, partitionCfgTy> partitionCfgMapTy;
	typedef struct {
		uint64_t totalSize;
		size_t wordSize;
	} arrayInfoCfgTy;
	typedef std::map<std::string, arrayInfoCfgTy> arrayInfoCfgMapTy;

private:
	std::string kernelName;

	std::vector<pipeliningCfgTy> pipeliningCfg;
	std::vector<unrollingCfgTy> unrollingCfg;
	std::vector<partitionCfgTy> partitionCfg;
	std::vector<partitionCfgTy> completePartitionCfg;

	partitionCfgMapTy partitionCfgMap;
	partitionCfgMapTy completePartitionCfgMap;
	arrayInfoCfgMapTy arrayInfoCfgMap;

	void appendToPipeliningCfg(std::string funcName, unsigned loopNo, unsigned loopLevel);
	void appendToUnrollingCfg(std::string funcName, unsigned loopNo, unsigned loopLevel, int lineNo, uint64_t unrollFactor);
	void appendToPartitionCfg(unsigned type, std::string baseAddr, uint64_t size, size_t wordSize, uint64_t pFactor);
	void appendToCompletePartitionCfg(std::string baseAddr, uint64_t size);
	void appendToArrayInfoCfg(std::string arrayName, uint64_t totalSize, size_t wordSize);

public:
	ConfigurationManager(std::string kernelName);

	void clear();
	void parseAndPopulate(std::vector<std::string> &pipelineLoopLevelVec);
	void parseToFiles();

	std::string getCfgKernel() { return kernelName; }
	const std::vector<pipeliningCfgTy> &getPipeliningCfg() const { return pipeliningCfg; }
	const std::vector<unrollingCfgTy> &getUnrollingCfg() const { return unrollingCfg; }
	const std::vector<partitionCfgTy> &getPartitionCfg() const { return partitionCfg; }
	const std::vector<partitionCfgTy> &getCompletePartitionCfg() const { return completePartitionCfg; }

	const partitionCfgMapTy &getPartitionCfgMap() const { return partitionCfgMap; }
	const partitionCfgMapTy &getCompletePartitionCfgMap() const { return completePartitionCfgMap; }
	const arrayInfoCfgMapTy &getArrayInfoCfgMap() const { return arrayInfoCfgMap; }
};

class LimitedQueue {
	size_t size;
	std::queue<unsigned> history;

public:
	LimitedQueue(size_t size);

	void push(unsigned elem);
	unsigned front();
	unsigned back();
	size_t getSize();
};

}

#endif // End of AUXILIARY_H
