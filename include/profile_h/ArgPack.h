#ifndef ARGPACK_H
#define ARGPACK_H

typedef struct {
	std::string inputFileName;
	std::string workDir;
	std::string outWorkDir;
	std::string configFileName;
	std::vector<std::string> kernelNames;

	int mode;
	enum {
		MODE_TRACE_AND_ESTIMATE = 0,
		MODE_TRACE_ONLY = 1,
		MODE_ESTIMATE_ONLY = 2
	};

	int target;
	enum {
		TARGET_XILINX_ZC702 = 0,
		TARGET_XILINX_ZCU102 = 1,
		TARGET_XILINX_VC707 = 2
	};

	bool verbose;
	bool compressed;
#ifdef PROGRESSIVE_TRACE_CURSOR
	bool progressive;
#endif
	double frequency;
	double uncertainty;
	bool memTrace;
	bool showCFG;
	bool showCFGDetailed;
	bool showPreOptDDDG;
	bool showPostOptDDDG;
	bool showScheduling;
	bool fGOpt;
	bool fNoTCS;
	bool fSBOpt;
	bool fSLROpt;
	bool fNoSLROpt;
	bool fRSROpt;
	bool fTHRFloatOpt;
	bool fTHRIntOpt;
	bool fMemDisambuigOpt;
	bool fNoFPUThresOpt;
	bool fExtraScalar;
	bool fRWRWMem;
	bool fILL;

	std::vector<std::string> targetLoops;
} ArgPack;

#endif
