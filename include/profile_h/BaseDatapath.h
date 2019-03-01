#ifndef __BASE_DATAPATH__
#define __BASE_DATAPATH__

/* Base class of all datapath types. Child classes must implement the following
 * abstract methods:
 *
 * globalOptimizationPass()
 * stepExecutingQueue()
 * getTotalMemArea()
 * getAverageMemPower()
 * writeConfiguration()
 *
 */

// If using GCC, these pragmas will stop GCC from outputting the thousands of warnings generated by boost library (WHICH IS EXTREMELY ANNOYING)
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wparentheses"
#endif
#include <boost/config.hpp>
#include <boost/tokenizer.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/topological_sort.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stdint.h>

// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
// TODO: ENABLE THOSE HEADERS ON DEMAND
#include "profile_h/auxiliary.h"
//#include "profile_h/file_func.h"
//#include "profile_h/fpga_resources.h"
//#include "profile_h/generic_func.h"
//#include "profile_h/opcodes.h"
#include "profile_h/DDDGBuilder.h"
#include "profile_h/HardwareProfile.h"
//#include "profile_h/Registers.h"

using namespace llvm;

// TODO: ENABLE THOSE MACROS ON DEMAND
// TODO: ENABLE THOSE MACROS ON DEMAND
// TODO: ENABLE THOSE MACROS ON DEMAND
// TODO: ENABLE THOSE MACROS ON DEMAND
// TODO: (but first, check if there is no other better way to create them (i.e. enums)
#if 0
#define CONTROL_EDGE 200
#define PIPE_EDGE 201

#define LEFT_NODE 1
#define RIGHT_NODE 2

#define BYTE_SIZE_SHIFT 3

#define GPU_VEC_ADD_MUL_LIMITATION 4

#define WRITE_EXECUTION_RECORD

// CHECK_INST_IN_EACH_LEVEL is used to check detail instructions in 
// each level when we analyze parallelism profile in parallelismProfileDDDG() 
// of BaseDatapath.cpp
#define CHECK_INST_IN_EACH_LEVEL
#endif

typedef boost::property<boost::vertex_index_t, unsigned> VertexProperty;
typedef boost::property<boost::edge_weight_t, uint8_t> EdgeProperty;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, VertexProperty, EdgeProperty> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;
typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;
typedef boost::graph_traits<Graph>::in_edge_iterator InEdgeIterator;
typedef boost::graph_traits<Graph>::out_edge_iterator OutEdgeIterator;
typedef boost::property_map<Graph, boost::edge_weight_t>::type EdgeWeightMap;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertexNameMap;

typedef std::unordered_map<std::string, unsigned> staticInstID2OpcodeMapTy;
extern staticInstID2OpcodeMapTy staticInstID2OpcodeMap;

typedef struct {
	unsigned from;
	unsigned to;
	uint8_t paramID;
} edgeTy;

#if 0
// Used heavily in reporting cycle-level statistics.
typedef std::unordered_map<std::string, std::vector<int>> ActivityMap;
typedef std::unordered_map<std::string, int> MaxActivityMap;

typedef std::unordered_map<std::string, unsigned> instID2TimestampMapTy;

// Instruction ID (string type) --> Vertices List Map
typedef std::unordered_map<std::string, std::list<Vertex> > staticInstID2VertexListMapTy;
// Vertex --> Timestamp Map
typedef std::unordered_map<Vertex, unsigned> Vertex2TimestampMapTy;
// We need to use rbegin() method, therefore, can not use unordered_multimap
typedef std::multimap<unsigned, Vertex> timestamp2VertexMMapTy;
typedef std::unordered_map<std::string, timestamp2VertexMMapTy> staticInstID2TimestampInfoMapTy;
typedef std::unordered_map<Vertex, unsigned> Vertex2LevelMapTy;
typedef std::unordered_map<unsigned, std::pair<long long int, unsigned> > vertexID2AddressAndSizeMapTy;

typedef unsigned node_idTy;
typedef long long int addressTy;
typedef uint64_t memSizeTy;

typedef std::unordered_set<unsigned> engineSlotTy;

typedef std::vector<unsigned> schedTimeTy;
typedef std::vector<unsigned> cPathNodeTy;
typedef std::list<unsigned> selectedNodeTy;
typedef std::list<std::pair<unsigned, unsigned> > startingQueueListTy;
typedef std::list< std::pair<unsigned, unsigned> > readyQueueListTy;
typedef std::list<unsigned> selectedQueueListTy;
/// Node ID --> Node time map (Only consider nodes with latency larger than 1)
typedef std::map<unsigned, unsigned> executingQueueMapTy;

typedef std::map<std::string, uint64_t> arrayName2maxMemOpNumTy;
typedef std::map<std::string, float> arrayName2MemBankStatisTy;

// Key is (staticID_str, vertex, timestamp)
struct KeyTy{
	std::string static_instID_str;
	Vertex vertex;
	unsigned timestamp;
};

class keyComp {
public:
	bool operator() (const KeyTy& arg1, const KeyTy& arg2) const {
		if (arg1.static_instID_str < arg2.static_instID_str) {
			return true;
		}
		else if (arg1.static_instID_str == arg2.static_instID_str) {
			if (arg1.vertex < arg2.vertex) {
				return true;
			}
			else if (arg1.vertex == arg2.vertex) {
				if (arg1.timestamp < arg2.timestamp) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			} 
		}
		else {
			return false;
		}
	}
};

struct AddressTuple{
	addressTy raddr1;
	addressTy raddr2;
	addressTy waddr;

	bool operator< (const AddressTuple &other) const {
		if (raddr1 < other.raddr1) {
			return true;
		}
		else if (raddr1 == other.raddr1) {
			if (raddr2 < other.raddr2) {
				return true;
			}
			else if (raddr2 == other.raddr2) {
				if (waddr < other.waddr) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	bool operator== (const AddressTuple &other) const {
		return raddr1 == other.raddr1 && raddr2 == other.raddr2 && waddr == waddr;
	}

	AddressTuple operator- (const AddressTuple &other) {
		AddressTuple result;
		result.raddr1 = raddr1 - other.raddr1;
		result.raddr2 = raddr2 - other.raddr2;
		result.waddr = waddr - other.waddr;
		return result;
	}

	/*
	AddressTuple& operator= (const AddressTuple &other) {
		raddr1 = other.raddr1;
		raddr2 = other.raddr2;
		waddr = other.waddr;
		return *this;
	}*/
};

// Vector of pairs (Vertex, AddressTuple) for sorting.
typedef std::pair<Vertex, AddressTuple> pairVertexAddressTupleTy;
typedef std::vector< pairVertexAddressTupleTy > VertexAddressTuplePairVecTy;

// (staticID_str, vertex, timestamp) -> AddressTuple
//typedef std::unordered_multimap<Key, AddressTuple> instancesToAddressTupleMapTy;

// (staticID_str, vertex, timestamp) -> partition ID
//typedef std::unordered_multimap<Key, unsigned> instancesToPartitionIDMapTy;
typedef std::map<KeyTy, Vertex, keyComp> instancesToPartitionIDMapTy;

/*
bool Vertex2LevelMapCmpFunc(const pair<Vertex, unsigned> &p1, const pair<Vertex, unsigned> &p2);
*/

typedef std::map<unsigned, std::vector<unsigned> > cStep2nodeIDMapTy;

typedef std::map<std::string, std::pair<uint64_t, unsigned> > arrayInfoMapTy;

enum Loop_Status {INIT, EXECUTING, FINISHED};

template<class T>
bool pairCompare(const T &pair1, const T &pair2) {
	return pair1.second < pair2.second;
}

template<class T>
typename T::const_iterator map_max_element(const T &A) {
	typedef typename T::value_type pair_type;
	return std::max_element(A.begin(), A.end(), pairCompare<pair_type>);
}

//class Scratchpad;

struct partitionEntry
{
  std::string type;
  unsigned array_size; //num of bytes
  unsigned wordsize; //in bytes
  unsigned part_factor;
	partitionEntry() : type(""), array_size(0), wordsize(0), part_factor(0) {}
	partitionEntry(std::string tp, unsigned size, unsigned wdsize, unsigned partFT) : type(tp), array_size(size), wordsize(wdsize), part_factor(partFT) {}
};

struct regEntry
{
  int size;
  int reads;
  int writes;
};
struct callDep
{
  std::string caller;
  std::string callee;
  int callInstID;
};
struct newEdge
{
  unsigned from;
  unsigned to;
  int parid;
};
struct RQEntry
{
  unsigned node_id;
  mutable float latency_so_far;
  mutable bool valid;
};

struct RQEntryComp
{
  bool operator() (const RQEntry& left, const RQEntry &right) const
  { return left.node_id < right.node_id; }
};

bool compare_ready_queue_basedon_alapTime(const std::pair<unsigned, unsigned>& first, const std::pair<unsigned, unsigned>& second);

struct OccupiedMemBWPerPartition {
	uint16_t readPort_num;
	uint16_t writePort_num;
};

struct LoopLatencyTy {
	uint64_t no_pipeline;
	uint64_t with_pipeline;
	bool enable_pipeline;
};

struct max_asapTy {
	uint64_t max_cycles;
	uint64_t max_level;
};

struct resourceTy {
	unsigned dsp_used;
	unsigned bram18k_used;
	unsigned ff_used;
	unsigned lut_used;

	unsigned fadd_used;
	unsigned fsub_used;
	unsigned fmul_used;
	unsigned fdiv_used;
};

struct sharedMemTy {
	int shared_loads;
	int repeated_stores;
};

struct loopInfoTy {
	std::string loop_name;
	unsigned lp_level;
	unsigned lp_level_unroll_factor;
	bool enable_pipelining;

	uint64_t num_cycles;
	/// ASAP result
	unsigned IL_asap;
	/// RC_List result
	uint64_t rc_list_il;

	unsigned max_II;
	unsigned ResII_mem;
	std::string limited_mem_name;
	unsigned ResII_op;
	std::string limited_op_name;
	unsigned RecII;
};

struct subTraceInstTy {
	uint64_t num_ld_inst;
	uint64_t num_st_inst;
	uint64_t num_fadd_inst;
	uint64_t num_fsub_inst;
	uint64_t num_fmul_inst;
	uint64_t num_fdiv_inst;
	uint64_t num_fcmp_inst;
	uint64_t num_integer_inst;
	uint64_t num_bitwise_inst;
	uint64_t num_control_inst;
	uint64_t num_br_inst;
};

class Constraint_FPGA_simulation {
public:
	Constraint_FPGA_simulation() : hwfloatAdd_num(INFINITE_HARDWARE), hwfloatSub_num(INFINITE_HARDWARE), hwfloatMul_num(INFINITE_HARDWARE), hwfloatDiv_num(INFINITE_HARDWARE), bram18k_num(INFINITE_HARDWARE), readPt_num_perBram(INFINITE_HARDWARE), writePt_num_perBram(INFINITE_HARDWARE) {}
	Constraint_FPGA_simulation(unsigned fadd_num, unsigned fsub_num, unsigned fmul_num, unsigned fdiv_num, unsigned bram_num, unsigned readP_num, unsigned writeP_num)
		: hwfloatAdd_num(fadd_num), hwfloatSub_num(fsub_num), hwfloatMul_num(fmul_num), hwfloatDiv_num(fdiv_num), bram18k_num(bram_num), readPt_num_perBram(readP_num), writePt_num_perBram(writeP_num) {}
	~Constraint_FPGA_simulation() {}
	void setConstraints(unsigned faddNum, unsigned fsubNum, unsigned fmulNum, unsigned fdivNum, unsigned bramNum, unsigned readPNum, unsigned writePNum) {
		hwfloatAdd_num = faddNum;
		hwfloatSub_num = fsubNum;
		hwfloatMul_num = fmulNum;
		hwfloatDiv_num = fdivNum;
		bram18k_num = bramNum;
		readPt_num_perBram = readPNum;
		writePt_num_perBram = writePNum;
	}

	unsigned get_fadd_num() const { return hwfloatAdd_num; }
	unsigned get_fsub_num() const { return hwfloatSub_num; }
	unsigned get_fmul_num() const { return hwfloatMul_num; }
	unsigned get_fdiv_num() const { return hwfloatDiv_num; }
	unsigned get_fbram_num() const { return bram18k_num; }
	unsigned get_freadP_num() const { return readPt_num_perBram; }
	unsigned get_fwriteP_num() const { return writePt_num_perBram; }

private:
	unsigned hwfloatAdd_num;
	unsigned hwfloatSub_num;
	unsigned hwfloatMul_num;
	unsigned hwfloatDiv_num;
	unsigned bram18k_num;
	unsigned readPt_num_perBram;
	unsigned writePt_num_perBram;
};
#endif

class BaseDatapath {
public:
	class RCScheduler {
		typedef std::list<std::pair<unsigned, uint64_t>> nodeTickTy;
		typedef std::list<unsigned> selectedListTy;
		typedef std::map<unsigned, unsigned> executingMapTy;

		const std::vector<int> &microops;
		const Graph &graph;
		unsigned numOfTotalNodes;
		const std::unordered_map<unsigned, Vertex> &nameToVertex;
		const VertexNameMap &vertexToName;
		HardwareProfile &profile;
		const std::unordered_map<int, std::pair<std::string, int64_t>> &baseAddress;
		const std::vector<uint64_t> &asap;
		const std::vector<uint64_t> &alap;
		std::vector<uint64_t> &rc;

		std::vector<unsigned> numParents;
		std::vector<bool> finalIsolated;
		unsigned totalConnectedNodes;
		unsigned scheduledNodeCount;
		uint64_t cycleTick;

		nodeTickTy startingNodes;

		nodeTickTy fAddReady;
		nodeTickTy fSubReady;
		nodeTickTy fMulReady;
		nodeTickTy fDivReady;
		nodeTickTy fCmpReady;
		nodeTickTy loadReady;
		nodeTickTy storeReady;
		nodeTickTy intOpReady;
		nodeTickTy callReady;
		nodeTickTy othersReady;

		selectedListTy fAddSelected;
		selectedListTy fSubSelected;
		selectedListTy fMulSelected;
		selectedListTy fDivSelected;
		selectedListTy fCmpSelected;
		selectedListTy loadSelected;
		selectedListTy storeSelected;
		selectedListTy intOpSelected;
		selectedListTy callSelected;

		executingMapTy fAddExecuting;
		executingMapTy fSubExecuting;
		executingMapTy fMulExecuting;
		executingMapTy fDivExecuting;
		executingMapTy intOpExecuting;

		bool dummyAllocate() { return true; }
		static bool compareReadyByALAP(const std::pair<unsigned, uint64_t> &first, const std::pair<unsigned, uint64_t> &second) { return first.second < second.second; }

		void assignReady();
		void select();

		void pushReady(unsigned nodeID, uint64_t tick);
		void trySelect(nodeTickTy &ready, selectedListTy &selected, bool (HardwareProfile::*tryAllocate)() = nullptr);
		void trySelect(nodeTickTy &ready, selectedListTy &selected, bool (HardwareProfile::*tryAllocateMem)(std::string));
	public:
		RCScheduler(
			const std::vector<int> &microops,
			const Graph &graph, unsigned numOfTotalNodes,
			const std::unordered_map<unsigned, Vertex> &nameToVertex, const VertexNameMap &vertexToName,
			HardwareProfile &profile, const std::unordered_map<int, std::pair<std::string, int64_t>> &baseAddress,
			const std::vector<uint64_t> &asap, const std::vector<uint64_t> &alap, std::vector<uint64_t> &rc
		);
		void schedule();
	};

	class ColorWriter {
		Graph &graph;
		VertexNameMap &vertexNameMap;
		const std::vector<std::string> &bbNames;
		const std::vector<std::string> &funcNames;
		std::vector<int> &opcodes;
		llvm::bbFuncNamePair2lpNameLevelPairMapTy &bbFuncNamePair2lpNameLevelPairMap;

	public:
		ColorWriter(
			Graph &graph,
			VertexNameMap &vertexNameMap,
			const std::vector<std::string> &bbNames,
			const std::vector<std::string> &funcNames,
			std::vector<int> &opcodes,
			llvm::bbFuncNamePair2lpNameLevelPairMapTy &bbFuncNamePair2lpNameLevelPairMap
		);

		template<class VE> void operator()(std::ostream &out, const VE &v) const;
	};

	class EdgeColorWriter {
		Graph &graph;
		EdgeWeightMap &edgeWeightMap;

	public:
		EdgeColorWriter(Graph &graph, EdgeWeightMap &edgeWeightMap);

		template<class VE> void operator()(std::ostream &out, const VE &e) const;
	};

private:
	std::string kernelName;
	ConfigurationManager &CM;
	std::ostream *summaryFile;
	std::string loopName;
	unsigned loopLevel;
	uint64_t loopUnrollFactor;

	DDDGBuilder *builder;
	ParsedTraceContainer PC;
#ifdef USE_FUTURE
	FutureCache *future;
#endif
	HardwareProfile *profile;

	void findMinimumRankPair(std::pair<unsigned, unsigned> &pair, std::map<unsigned, unsigned> rankMap);

public:
#ifdef USE_FUTURE
	BaseDatapath(
		std::string kernelName, ConfigurationManager &CM, std::ofstream *summaryFile,
		std::string loopName, unsigned loopLevel, uint64_t loopUnrollFactor,
		FutureCache *future,
		bool enablePipelining, uint64_t asapII
	);
#else
	BaseDatapath(
		std::string kernelName, ConfigurationManager &CM, std::ofstream *summaryFile,
		std::string loopName, unsigned loopLevel, uint64_t loopUnrollFactor,
		bool enablePipelining, uint64_t asapII
	);
#endif

	~BaseDatapath();

	std::string getTargetLoopName() const;
	unsigned getTargetLoopLevel() const;
	uint64_t getTargetLoopUnrollFactor() const;
	unsigned getNumNodes() const;
	unsigned getNumEdges() const;

	void insertMicroop(int microop);
	void insertDDDGEdge(unsigned from, unsigned to, uint8_t paramID);
	bool edgeExists(unsigned from, unsigned to);
	void updateRemoveDDDGEdges(std::set<Edge> &edgesToRemove);
	void updateAddDDDGEdges(std::vector<edgeTy> &edgesToAdd);
	void updateRemoveDDDGNodes(std::vector<unsigned> &nodesToRemove);

	std::string constructUniqueID(std::string funcID, std::string instID, std::string bbID);

protected:
	// Special edge types
	enum {
		EDGE_CONTROL = 200,
		EDGE_PIPE = 201
	};

	bool enablePipelining;
	uint64_t asapII;
	uint64_t numCycles;

	// A map from node ID to its microop
	std::vector<int> microops;
	// The DDDG
	Graph graph;
	// Number of nodes in the graph
	unsigned numOfTotalNodes;
	// A map from node ID to boost internal ID
	std::unordered_map<unsigned, Vertex> nameToVertex;
	// A map from boost internal ID to node ID
	VertexNameMap vertexToName;
	// A map from edge internal ID to its weight (parameter ID before estimation, node latency after estimation)
	EdgeWeightMap edgeToWeight;
	// Set containing all called functions
	std::unordered_set<std::string> functionNames;
	// TODO: check type
	unsigned numOfPortsPerPartition;
	// A map from node ID to a getelementptr pair (variable name and its address)
	std::unordered_map<int, std::pair<std::string, int64_t>> baseAddress;
	// A set containing the name of all arrays that are not marked for partitioning
	std::set<std::string> noPartitionArrayName;
	// TODO: check its purpose
  	std::unordered_set<std::string> dynamicMemoryOps;
	// Vector with scheduled times for each node
	std::vector<uint64_t> asapScheduledTime;
	std::vector<uint64_t> alapScheduledTime;
	std::vector<uint64_t> rcScheduledTime;
	// Vector with nodes on the critical path
	std::vector<unsigned> cPathNodes;

	void initBaseAddress();

	uint64_t fpgaEstimationOneMoreSubtraceForRecIICalculation();
	uint64_t fpgaEstimation();

	void removeInductionDependencies();
	void removePhiNodes();
	void enableStoreBufferOptimisation();
	void initScratchpadPartitions();
	void optimiseDDDG();
	void performMemoryDisambiguation();
	void removeSharedLoads();
	void removeRepeatedStores();
	void reduceTreeHeight(bool (&isAssociativeFunc)(unsigned));

	std::tuple<uint64_t, uint64_t> asapScheduling();
	void alapScheduling(std::tuple<uint64_t, uint64_t> asapResult);
	void identifyCriticalPaths();
	uint64_t rcScheduling();

	void dumpGraph(bool isOptimised = false);

#if 0
  //Change graph.
  void addDddgEdge(unsigned int from, unsigned int to, uint8_t parid);
	void insertMicroop(int node_microop);
  void setGlobalGraph();

	// For FPGA simulation
	void setGraphForStepping(NameVecTy& curBBname);

	// For parallel simulation
	void setGraphForStepping();

  int clearGraph();
  void dumpGraph();

  //Accessing graph stats.
  int getNumOfNodes() {return boost::num_vertices(graph_);}
  int getNumOfEdges() {return boost::num_edges(graph_);}
  int getMicroop(unsigned int node_id) {return microop.at(node_id);}
  int getNumOfConnectedNodes(unsigned int node_id) {return boost::degree(nameToVertex[node_id], graph_);}
  int getUnrolledLoopBoundary(unsigned int region_id) {return loopBound.at(region_id);}
  std::string getBenchName() {return benchName;}
  std::string getBaseAddressLabel(unsigned int node_id) {return baseAddress[node_id].first;}
	
	std::string getTargetLoopName() const;
	unsigned getTargetLoopLevel() const;
	unsigned getTargetLoopLevelUnrollFactor() const;

  bool doesEdgeExist(unsigned int from, unsigned int to) {return edge(nameToVertex[from], nameToVertex[to], graph_).second;}
  int shortestDistanceBetweenNodes(unsigned int from, unsigned int to);

  // Graph optimizations.
  void removeInductionDependence();
  void removePhiNodes();
  void memoryAmbiguation();
  void removeAddressCalculation();
  void removeBranchEdges();
  void nodeStrengthReduction();

  void loopFlatten();

  void loopUnrolling();
	void removeSharedLoads();
	void removeRepeatedStores();
	void storeBuffer();
	void treeHeightReduction_integer();
	void treeHeightReduction_float();
#ifdef ALADDIN_H
  void loopPipelining();
#endif // End of ALADDIN_H

 protected:
  //Graph transformation helpers.
  void findMinRankNodes(
      unsigned &node1, unsigned &node2, std::map<unsigned, unsigned> &rank_map);
  void cleanLeafNodes();
  bool doesEdgeExistVertex(Vertex from, Vertex to) {return edge(from, to, graph_).second;}

//#ifdef ALADDIN_H
  // Configuration parsing and handling.
	void readArrayInfo(arrayInfoMapTy& arrayInfoMap);
  bool readPipeliningConfig();
  bool readUnrollingConfig(std::unordered_map<int, int > &unrolling_config);
  bool readFlattenConfig(std::unordered_set<int> &flatten_config);
  bool readPartitionConfig(std::unordered_map<std::string,
         partitionEntry> & partition_config);
  bool readCompletePartitionConfig(std::unordered_map<std::string, unsigned> &config);
//#endif // End of ALADDIN_H

  // State initialization.
	void initialize_graph();
  void initMethodID(std::vector<int> &methodid);
  void initDynamicMethodID(std::vector<std::string> &methodid);
  void initPrevBasicBlock(std::vector<std::string> &prevBasicBlock);
	void initCurBasicBlock(std::vector<std::string> &curBasicBlock);
	void initBaseAddress();
  void initInstID(std::vector<std::string> &instid);
  void initAddressAndSize(
      std::unordered_map<unsigned, pair<long long int, unsigned> > &address);
  void initAddress(std::unordered_map<unsigned, long long int> &address);
  void initLineNum(std::vector<int> &lineNum);
  void initGetElementPtr(
      std::unordered_map<unsigned, pair<std::string, long long int> > &get_element_ptr);

	// Get partition information
	bool completePartition();
	void scratchpadPartition();
	void setScratchpad(std::string baseName, unsigned num_of_bytes, unsigned word_size);
	bool partitionExist(std::string baseName);

  //Graph updates.
  void updateGraphWithIsolatedEdges(std::set<Edge> &to_remove_edges);
  void updateGraphWithNewEdges(std::vector<newEdge> &to_add_edges);
  void updateGraphWithIsolatedNodes(std::vector<unsigned> &to_remove_nodes);
  void updateRegStats();

	// Calculate instruction distribution of this kernel
	void calculateInstructionDistribution();

	// Calculate timestamp for DDDG
	//void calculateTimestampDDDG(Graph graph_arg);
	void calculateTimestampDDDG();
	unsigned getvertexTimestamp(Vertex vertex_arg);
	unsigned getvertexOpcode(Vertex vertex_arg) const;
	void vectorizationAnalysis(VertexAddressTuplePairVecTy &pairVec, std::string& staticID, unsigned& timestamp_arg, memSizeTy memSize, unsigned& partID);
	bool isUnitStride(const AddressTuple& strideTuple, memSizeTy size);
	bool isConstantNonUnitStride(const AddressTuple& strideTuple, memSizeTy size);

	void parallelismProfileDDDG();
	unsigned getvertexLevel(Vertex vertex_arg);
	bool ignoreDynamicInst(unsigned opcode_arg);

  // Scheduling
  void addMemReadyNode( unsigned node_id, float latency_so_far);
  void addNonMemReadyNode( unsigned node_id, float latency_so_far);
  int fireMemNodes();
  int fireNonMemNodes();

  // Stats output.
  void writeFinalLevel();
  void writeGlobalIsolated();
  void writeBaseAddress();
  void writeMicroop(std::vector<int> &microop);
	virtual void dumpStats();
#ifdef ALADDIN_H
	void writePerCycleActivity();
  void initPerCycleActivity(
         std::vector<std::string> &comp_partition_names,
         std::vector<std::string> &spad_partition_names,
         activity_map &ld_activity, activity_map &st_activity,
         activity_map &mul_activity, activity_map &add_activity,
         activity_map &bit_activity,
         max_activity_map &max_mul_per_function,
         max_activity_map &max_add_per_function,
         max_activity_map &max_bit_per_function,
         int num_cycles);
  void updatePerCycleActivity(
         activity_map &ld_activity, activity_map &st_activity,
         activity_map &mul_activity, activity_map &add_activity,
         activity_map &bit_activity,
         max_activity_map &max_mul_per_function,
         max_activity_map &max_add_per_function,
         max_activity_map &max_bit_per_function);
  void outputPerCycleActivity(
         std::vector<std::string> &comp_partition_names,
         std::vector<std::string> &spad_partition_names,
         activity_map &ld_activity, activity_map &st_activity,
         activity_map &mul_activity, activity_map &add_activity,
         activity_map &bit_activity,
         max_activity_map &max_mul_per_function,
         max_activity_map &max_add_per_function,
         max_activity_map &max_bit_per_function);

  // Memory structures.
  virtual double getTotalMemArea() = 0;
  virtual unsigned getTotalMemSize() = 0;
  virtual void getAverageMemPower(
      unsigned int cycles, float *avg_power,
      float *avg_dynamic, float *avg_leak) = 0;
  virtual void getMemoryBlocks(std::vector<std::string> &names) = 0;

	virtual void globalOptimizationPass() = 0;
#endif // End of ALADDIN_H

	/* Run simulation */
	/// For FPGA simulation
	void set_fpga_constraints(unsigned faddNum, unsigned fsubNum, unsigned fmulNum, unsigned fdivNum, unsigned bramNum, unsigned readPNum, unsigned writePNum);
	uint64_t run_fpga_simulation();
	void initExecutingQueue(NameVecTy& curBBname);
	bool step(NameVecTy& bb_names);
	void stepExecutingQueue(NameVecTy& bb_names);
	void updateChildren(unsigned node_id, NameVecTy& bb_names);
	void updateWaitForLoopQueue(NameVecTy& bb_names);
	unsigned getIthLoop(std::string bbName, std::string funcName);

	/// For parallel simulation
	uint64_t run_parallel_simulation();
	void initExecutingQueue();
  bool step();
  void stepExecutingQueue();
	void updateChildren(unsigned node_id);

	void copyToExecutingQueue();
	void updateDelayForNodeID();
	bool calculateConstraint(unsigned node_id, unsigned node_delay);
	void removeConstraint(unsigned node_id);
	void clearOccupiedBWPerPartition();
	bool memoryBWconstraint(std::string baseName, bool load_or_store);
	//bool canServicePartition(std::string baseName);
	unsigned findPartitionID(std::string baseName);
	//bool addressRequest(std::string baseName);
	/* Increment the loads counter for the specified partition. */
	void increment_loads(std::string partition);
	/* Increment the stores counter for the specified partition. */
	void increment_stores(std::string partition);

	// Miscellaneous
	void tokenizeString(std::string input, std::vector<int>& tokenized_list);

	/* For FPGA Estimation */
	uint64_t fpga_estimation();
	uint64_t fpga_estimation_one_more_subtrace_for_recII_calculation();
	max_asapTy asap_scheduling(Graph& graph_tmp, schedTimeTy& schedTime);
	void alap_scheduling(Graph& graph_tmp, schedTimeTy& schedTime, unsigned maxCstep, unsigned max_level);
	uint64_t rc_list_scheduling(Graph& graph_tmp, schedTimeTy& asap_time, schedTimeTy& alap_time, schedTimeTy& rsList_time, cPathNodeTy& cp_nodes, bool pipelining_enable);
	void pre_optimization();
	void calculateFPGAResRequired(cStep2nodeIDMapTy& cStepMap, UsedFPGA_ResClass& fpga_resources);
	void critical_path_extraction(schedTimeTy& asap_schedTime, schedTimeTy& alap_schedTime, cPathNodeTy& cpNodeVec);
	void initialize_array_partition_config(arrayName2arrayConfigTy& arrayName2Config);
	void initialize_starting_queue(Graph& graph_tmp, schedTimeTy& alapTime, schedTimeTy& asapTime);
	void initialize_ready_queue(unsigned alap_time_tick, unsigned cur_node_id);
	void updateChildrenNodes(Graph& graph_tmp, unsigned curr_node_Id, schedTimeTy& alap_time);
	void updateReadyQueue(unsigned child_nodeID, unsigned opcode, schedTimeTy& alapTime);
	void determineReadyAndSelectOperationList(Graph& graph_tmp, schedTimeTy& alapTime, schedTimeTy& rsListTime, unsigned& clock_tick, UsedFPGA_Res_With_Constraint& cFPGA_constraints, unsigned& scheduledCounter);
	void determineExecutingMap(Graph& graph_tmp, schedTimeTy& alapTime, schedTimeTy& rsListTime, unsigned& clock_tick, UsedFPGA_Res_With_Constraint& cFPGA_constraints, unsigned& scheduledCounter);
	void updateExecutingMapAndReadyQueue(Graph& graph_tmp, schedTimeTy& alapTime, schedTimeTy& rsListTime, unsigned& clock_tick, UsedFPGA_Res_With_Constraint& cFPGA_constraints, unsigned& scheduledCounter);
	unsigned getMemResII_based_on_rcList_scheduling(schedTimeTy& rsList_time, unsigned max_level, arrayName2arrayConfigTy& arrayName2arrayConfig, UsedFPGA_Res_With_Constraint& cFPGA_constraints);
	unsigned getOpResII_based_on_rcList_scheduling(UsedFPGA_Res_With_Constraint& cFPGA_constraints);
	LoopLatencyTy getLoopTotalLatency(uint64_t iterationLat, unsigned max_ii, std::string lp_name, unsigned lp_level, bool pipelining_or_not);
	unsigned getRecII(bool enable_lpPipelining, schedTimeTy& asap_time, cPathNodeTy& cp_nodes);

	void getTargetArrayName2maxReadWrite();
	void calculateArrayName2maxReadWrite(unsigned int iterations);
	void writeLogOfArrayName2maxReadWrite();

  std::unordered_map<unsigned, Vertex> nameToVertex;
  VertexNameMap vertexToName;
  EdgeWeightMap edgeToParid;

  //Graph node and edge attributes.
	unsigned numTotalNodes;
  unsigned numTotalEdges;

  // Completely partitioned arrays.
  Registers registers;

  std::vector<int> newLevel;
	std::vector<unsigned> maxParentOpLatency;
  std::vector<regEntry> regStats;
  std::vector<int> microop;
  std::unordered_map<unsigned, pair<std::string, long long int> > baseAddress;
  std::unordered_set<std::string> dynamicMemoryOps;
  std::vector<unsigned> numParents;
  std::vector<float> latestParents;
  std::vector<bool> finalIsolated;
  std::vector<int> edgeLatency;
  std::vector<int> loopBound;

  //Scheduling.
  unsigned totalConnectedNodes;
  unsigned executedNodes;
  std::vector<unsigned> executingQueue;
  std::vector<unsigned> readyToExecuteQueue;
	std::vector<unsigned> waitForLoopQueue;
	instID2TimestampMapTy staticInstID2TimestampMap;

	schedTimeTy asapSchedTime;
	schedTimeTy alapSchedTime;
	schedTimeTy rcListSchedTime;

private:
	//boost graph.
	Graph graph_;

	std::string benchName;
	std::string inputPath;
	ofstream *summaryFile;

	staticInstID2VertexListMapTy staticInstID2VertexListMap;
	Vertex2TimestampMapTy Vertex2TimestampMap;
	Vertex2LevelMapTy Vertex2LevelMap;
	std::vector<unsigned> parallelism_profile;
#ifdef CHECK_INST_IN_EACH_LEVEL
	std::map<unsigned, std::vector<std::string> > instInlevels; 
#endif // End of CHECK_INST_IN_EACH_LEVEL

	// Record timestamp information of instances of interesting static instructions
	staticInstID2TimestampInfoMapTy staticInstID2TimestampInfoMap;

	//instancesToAddressTupleMapTy instancesToAddressTupleMap;
	instancesToPartitionIDMapTy instancesToPartitionIDMap;

	uint64_t num_dynInst;
	uint64_t num_unitVectorInst;
	uint64_t num_constNonUnitVectorInst;
	// Used to calculate average vector size per vector instruction
	uint64_t total_sizeOf_vectorInst; 
	uint64_t num_singletonInst;

	memSizeTy num_byte_transferred;

	unsigned numOfPartitions;
	unsigned numOfPortsPerPartition;
	std::unordered_map<std::string, unsigned> baseToPartitionID;
	/* Number of loads per partition. */
	std::map<std::string, unsigned> partition_loads;
	/* Number of stores per partition. */
	std::map<std::string, unsigned> partition_stores;

	std::set<std::string> partitionArrayName;
	/* noPartitionArrayName will be inserted elements only if an array is not partitioned. */
	std::set<std::string> noPartitionArrayName;

	std::map<std::string, uint64_t> arrayName2numRead;
	std::map<std::string, uint64_t> arrayName2numWrite;
	/* The following two maps record (original array name --> maximum number of read/write 
	   operations for a specific array partition):  
		 Example: For memory read/write on array A and we have 4 partitions (A-0, A-1, A-2, A-3).
		          Number of memory read on each partition:  
																											 A-0: 2;  A-1:  4; 
																											 A-2: 8;  A-3:  0;
							Then we record (A --> 8) into the map. */
	arrayName2maxMemOpNumTy arrayName2maxMemOpNum;
	arrayName2maxMemOpNumTy arrayName2maxMemOpNum_subtrace;
	arrayName2MemBankStatisTy arrayName2aveLoadAccessPerBank_subtrace;
	arrayName2MemBankStatisTy arrayName2aveStoreAccessPerBank_subtrace;

	arrayInfoMapTy arrayN2sizeWordsizeBytePr;

	std::vector<OccupiedMemBWPerPartition> occupiedMemPerPartition;
	//std::vector<unsigned> occupiedBWPerPartition;
	std::vector<unsigned> sizePerPartition;

	std::map<node_idTy, unsigned> nodeID2delay;

	/* The following vector is used to trace whether a loop finishs its execution or not
	The size of loopExecutionTracer is number of loops in a function. The elements inside
	loopExecutionTracer are bool type and they are used to indicate whether a loop finishs
	its execution or not. We use this information to make different loops run in sequence.
	*/
	std::vector<Loop_Status> loopExecutionTracer;

	Constraint_FPGA_simulation fpga_constraints;

	engineSlotTy fadd_engine;
	engineSlotTy fsub_engine;
	engineSlotTy fmul_engine;
	engineSlotTy fdiv_engine;

	UsedFPGA_ResClass asap_FPGA_resources;
	UsedFPGA_ResClass alap_FPGA_resources;
	//cStep2nodeIDMapTy cStep2nodeIDMap;
	cPathNodeTy cPathNodes;

	startingQueueListTy startingNodesList;

	readyQueueListTy readyFaddList;
	readyQueueListTy readyFsubList;
	readyQueueListTy readyFmulList;
	readyQueueListTy readyFdivList;
	readyQueueListTy readyFcmpList;
	readyQueueListTy readyLoadList;
	readyQueueListTy readyStoreList;
	readyQueueListTy readyIntegerOpList;
	readyQueueListTy readyCallOpList;
	readyQueueListTy readyOthersList;

	selectedQueueListTy selectedFaddList;
	selectedQueueListTy selectedFsubList;
	selectedQueueListTy selectedFmulList;
	selectedQueueListTy selectedFdivList;
	selectedQueueListTy selectedFcmpList;
	selectedQueueListTy selectedLoadList;
	selectedQueueListTy selectedStoreList;
	selectedQueueListTy selectedIntegerOpList;
	selectedQueueListTy selectedCallOpList;

	// Executing queues are only used when nodes with latency larger than 1
	executingQueueMapTy executingFaddMap;
	executingQueueMapTy executingFsubMap;
	executingQueueMapTy executingFmulMap;
	executingQueueMapTy executingFdivMap;
	executingQueueMapTy executingIntegerOpMap;

#ifdef WRITE_EXECUTION_RECORD
	ofstream execution_record;
#endif // End of WRITE_EXECUTION_RECORD

	std::string configure_fileName;

	std::map<std::string, unsigned> arrayName2resII_r;
	std::map<std::string, unsigned> arrayName2resII_w;

	bool enable_pipeline;
	unsigned IL_asap_ii;
	bool enable_sharedLoadRemoval;

	/// ASAP result
	unsigned IL_asap;

	/// ALAP result
	unsigned alap_fadd_used;
	unsigned alap_fsub_used;
	unsigned alap_fmul_used;
	unsigned alap_fdiv_used;

	/// RC_List result
	uint64_t rc_list_il;

	/// Summary variables
	std::string target_loop_name;
	unsigned target_loop_level;
	unsigned target_lp_level_unroll_factor;
	
	uint64_t num_cycles;
	unsigned dsp_used;
	unsigned bram18k_used;
	unsigned ff_used;
	unsigned lut_used;
	unsigned fadd_used;
	unsigned fsub_used;
	unsigned fmul_used;
	unsigned fdiv_used;

	int shared_loads;
	int repeated_stores;

	unsigned max_II;
	unsigned ResII_mem;
	std::string limited_mem_name;
	unsigned ResII_op;
	std::string limited_op_name;
	unsigned RecII;

	/// Limited floating point operation unit type
	std::vector<std::string> limited_fop_unit_types;

	subTraceInstTy subTraceInst;

	float ave_parallelism;

	arrayName2memEfficiencyTy arrayName2memeff;
#endif
};

#if 0
void dump_summary(ofstream& summary_file, loopInfoTy loop_info, resourceTy fpga_rs, sharedMemTy sharedMem, arrayName2memEfficiencyTy& Memefficiency, std::vector<std::string>& limited_op_types, subTraceInstTy& sub_traceInst, float aveParallelism, arrayName2maxMemOpNumTy& arrayN2maxMemOpNum, arrayName2MemBankStatisTy arrayN2aveLdPerBank, arrayName2MemBankStatisTy arrayN2aveStPerBank);
#endif

#endif // End of __BASE_DATAPATH__
