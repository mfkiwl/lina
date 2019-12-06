#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

#include <set>
#include <iostream>

#include "profile_h/auxiliary.h"

using namespace llvm;

class HardwareProfile {
protected:
	const unsigned INFINITE_RESOURCES = 999999999;

	std::map<std::string, std::tuple<uint64_t, uint64_t, size_t>> arrayNameToConfig;
	std::map<std::string, unsigned> arrayNameToNumOfPartitions;
	std::map<std::string, unsigned> arrayNameToWritePortsPerPartition;
	std::map<std::string, float> arrayNameToEfficiency;
	std::map<std::string, unsigned> arrayPartitionToReadPorts;
	std::map<std::string, unsigned> arrayPartitionToReadPortsInUse;
	std::map<std::string, unsigned> arrayPartitionToWritePorts;
	std::map<std::string, unsigned> arrayPartitionToWritePortsInUse;
	unsigned fAddCount, fSubCount, fMulCount, fDivCount;
	unsigned unrFAddCount, unrFSubCount, unrFMulCount, unrFDivCount;
	unsigned fAddInUse, fSubInUse, fMulInUse, fDivInUse;
	unsigned fAddThreshold, fSubThreshold, fMulThreshold, fDivThreshold;
	bool isConstrained;
	bool thresholdSet;
	std::set<int> limitedBy;

public:
	enum {
		LIMITED_BY_FADD,
		LIMITED_BY_FSUB,
		LIMITED_BY_FMUL,
		LIMITED_BY_FDIV
	};

	HardwareProfile();
	virtual ~HardwareProfile() { }
	static HardwareProfile *createInstance();
	virtual void clear();

	virtual unsigned getLatency(unsigned opcode) = 0;
	virtual double getInCycleLatency(unsigned opcode) = 0;
	virtual bool isPipelined(unsigned opcode) = 0;
	virtual void calculateRequiredResources(
		std::vector<int> &microops,
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		std::unordered_map<int, std::pair<std::string, int64_t>> &baseAddress,
		std::map<uint64_t, std::set<unsigned>> &maxTimesNodesMap
	) = 0;
	virtual void setResourceLimits() = 0;
	virtual void setThresholdWithCurrentUsage() = 0;
	virtual void setMemoryCurrentUsage(
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		const ConfigurationManager::partitionCfgMapTy &partitionCfgMap,
		const ConfigurationManager::partitionCfgMapTy &completePartitionCfgMap
	) = 0;
	virtual void constrainHardware(
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		const ConfigurationManager::partitionCfgMapTy &partitionCfgMap,
		const ConfigurationManager::partitionCfgMapTy &completePartitionCfgMap
	);
	std::tuple<std::string, uint64_t> calculateResIIOp();

	virtual void fillPack(Pack &P);
	std::set<int> getConstrainedUnits() { return limitedBy; }

	virtual void arrayAddPartition(std::string arrayName) = 0;
	virtual bool fAddAddUnit(bool commit = true) = 0;
	virtual bool fSubAddUnit(bool commit = true) = 0;
	virtual bool fMulAddUnit(bool commit = true) = 0;
	virtual bool fDivAddUnit(bool commit = true) = 0;

	unsigned arrayGetNumOfPartitions(std::string arrayName);
	unsigned arrayGetPartitionReadPorts(std::string partitionName);
	unsigned arrayGetPartitionWritePorts(std::string partitionName);
	const std::map<std::string, std::tuple<uint64_t, uint64_t, size_t>> &arrayGetConfig() { return arrayNameToConfig; }
	const std::map<std::string, unsigned> &arrayGetNumOfPartitions() { return arrayNameToNumOfPartitions; }
	const std::map<std::string, float> &arrayGetEfficiency() { return arrayNameToEfficiency; }
	virtual unsigned arrayGetMaximumWritePortsPerPartition() = 0;
	unsigned fAddGetAmount() { return fAddCount; }
	unsigned fSubGetAmount() { return fSubCount; }
	unsigned fMulGetAmount() { return fMulCount; }
	unsigned fDivGetAmount() { return fDivCount; }

	bool fAddTryAllocate(bool commit = true);
	bool fSubTryAllocate(bool commit = true);
	bool fMulTryAllocate(bool commit = true);
	bool fDivTryAllocate(bool commit = true);
	bool fCmpTryAllocate(bool commit = true);
	bool loadTryAllocate(std::string arrayPartitionName, bool commit = true);
	bool storeTryAllocate(std::string arrayPartitionName, bool commit = true);
	bool intOpTryAllocate(int opcode, bool commit = true);
	bool callTryAllocate(bool commit = true);

	void pipelinedRelease();
	void fAddRelease();
	void fSubRelease();
	void fMulRelease();
	void fDivRelease();
	void fCmpRelease();
	void loadRelease(std::string arrayPartitionName);
	void storeRelease(std::string arrayPartitionName);
	void intOpRelease(int opcode);
	void callRelease();
};

class XilinxHardwareProfile : public HardwareProfile {
	enum {
		LATENCY_LOAD = 2,
		LATENCY_STORE = 1,
		LATENCY_ADD = 1,
		LATENCY_SUB = 1,
		LATENCY_MUL32 = 6,
		LATENCY_DIV32 = 36,
		LATENCY_FADD32 = 5,
		LATENCY_FSUB32 = 5,
		LATENCY_FMUL32 = 4,
		LATENCY_FDIV32 = 16,
		LATENCY_FCMP = 1
	};
	enum {
		SCHEDULING_LATENCY_LOAD = 1
	};
	enum {
		BRAM_PORTS_R = 2,
		BRAM_PORTS_W = 1
	};
	enum {
		PER_PARTITION_PORTS_R = 2,
		PER_PARTITION_PORTS_W = 1,
		PER_PARTITION_MAX_PORTS_W = 2
	};
	enum {
		DSP_FADD = 2,
		DSP_FSUB = 2,
		DSP_FMUL = 3,
		DSP_FDIV = 0
	};
	enum {
		FF_FADD = 205,
		FF_FSUB = 205,
		FF_FMUL = 143,
		FF_FDIV = 761
	};
	enum {
		LUT_FADD = 390,
		LUT_FSUB = 390,
		LUT_FMUL = 321,
		LUT_FDIV = 994
	};

protected:
	std::map<std::string, unsigned> arrayNameToUsedBRAM18k;
	unsigned maxDSP, maxFF, maxLUT, maxBRAM18k;
	unsigned usedDSP, usedFF, usedLUT, usedBRAM18k;
	unsigned fAddDSP, fAddFF, fAddLUT;
	unsigned fSubDSP, fSubFF, fSubLUT;
	unsigned fMulDSP, fMulFF, fMulLUT;
	unsigned fDivDSP, fDivFF, fDivLUT;

public:
	XilinxHardwareProfile();

	void clear();

	virtual unsigned getLatency(unsigned opcode);
	virtual double getInCycleLatency(unsigned opcode);
	bool isPipelined(unsigned opcode);
	void calculateRequiredResources(
		std::vector<int> &microops,
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		std::unordered_map<int, std::pair<std::string, int64_t>> &baseAddress,
		std::map<uint64_t, std::set<unsigned>> &maxTimesNodesMap
	);
	void setThresholdWithCurrentUsage();
	void setMemoryCurrentUsage(
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		const ConfigurationManager::partitionCfgMapTy &partitionCfgMap,
		const ConfigurationManager::partitionCfgMapTy &completePartitionCfgMap
	);
	void constrainHardware(
		const ConfigurationManager::arrayInfoCfgMapTy &arrayInfoCfgMap,
		const ConfigurationManager::partitionCfgMapTy &partitionCfgMap,
		const ConfigurationManager::partitionCfgMapTy &completePartitionCfgMap
	);

	void fillPack(Pack &P);

	void arrayAddPartition(std::string arrayName);
	void arrayAddPartitions(std::string arrayName, unsigned amount);
	bool fAddAddUnit(bool commit = true);
	bool fSubAddUnit(bool commit = true);
	bool fMulAddUnit(bool commit = true);
	bool fDivAddUnit(bool commit = true);

	unsigned arrayGetMaximumWritePortsPerPartition();
	std::map<std::string, unsigned> arrayGetUsedBRAM18k() { return arrayNameToUsedBRAM18k; }

	unsigned resourcesGetDSPs() { return usedDSP; }
	unsigned resourcesGetFFs() { return usedFF; }
	unsigned resourcesGetLUTs() { return usedLUT; }
	unsigned resourcesGetBRAM18k() { return usedBRAM18k; }
};

class XilinxVC707HardwareProfile : public XilinxHardwareProfile {
	enum {
		MAX_DSP = 2800,
		MAX_FF = 607200,
		MAX_LUT = 303600,
		MAX_BRAM18K = 2060
	};

public:
	void setResourceLimits();
};

class XilinxZC702HardwareProfile : public XilinxHardwareProfile {
	enum {
		MAX_DSP = 220,
		MAX_FF = 106400,
		MAX_LUT = 53200,
		MAX_BRAM18K = 280
	};

public:
	void setResourceLimits();
};

class XilinxZCUHardwareProfile : public XilinxHardwareProfile {
	enum {
		LATENCY_LOAD,
		LATENCY_STORE,
		LATENCY_ADD,
		LATENCY_SUB,
		LATENCY_MUL32,
		LATENCY_DIV32,
		LATENCY_FADD32,
		LATENCY_FSUB32,
		LATENCY_FMUL32,
		LATENCY_FDIV32,
		LATENCY_FCMP
	};

	// XXX: You can find the definitions at lib/Build_DDDG/HardwareProfileParams.cpp
	/* This map format: {key (the operation being considered), {key (latency for completion), in-cycle latency in ns}} */
	static const std::unordered_map<unsigned, std::map<unsigned, double>> timeConstrainedLatencies;
	static const std::unordered_map<unsigned, std::map<unsigned, unsigned>> timeConstrainedDSPs;
	static const std::unordered_map<unsigned, std::map<unsigned, unsigned>> timeConstrainedFFs;
	static const std::unordered_map<unsigned, std::map<unsigned, unsigned>> timeConstrainedLUTs;

	double effectivePeriod;
	std::unordered_map<unsigned, std::pair<unsigned, double>> effectiveLatencies;

public:
	XilinxZCUHardwareProfile();
	unsigned getLatency(unsigned opcode);
	double getInCycleLatency(unsigned opcode);
};

class XilinxZCU102HardwareProfile : public XilinxZCUHardwareProfile {
	enum {
		MAX_DSP = 2520,
		MAX_FF = 548160,
		MAX_LUT = 274080,
		// XXX: This device has BRAM36k units, that can be configured to be used as 2 BRAM18k units.
		MAX_BRAM18K = 1824
	};

public:
	XilinxZCU102HardwareProfile() { }
	void setResourceLimits();
};

class XilinxZCU104HardwareProfile : public XilinxZCUHardwareProfile {
	enum {
		MAX_DSP = 1728,
		MAX_FF = 460800,
		MAX_LUT = 230400,
		// XXX: This device has BRAM36k units, that can be configured to be used as 2 BRAM18k units.
		MAX_BRAM18K = 624
	};

public:
	XilinxZCU104HardwareProfile() { }
	void setResourceLimits();
};

#endif
