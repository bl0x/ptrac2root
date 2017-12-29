#pragma once

struct PtracFile
{
	const char *name;
	FILE *file;
	size_t lineno;
	size_t n_input_lines;
};

#define PTRAC_N_LINE_TYPES 6
#define PTRAC_MAX_N_STEPS 100

enum PtracVariableType
{
	PVT_NPS=1,
	PVT_TFH, /* Type of first history event */
	PVT_NCL_NPS,
	PVT_NSF_NPS,
	PVT_JPTAL,
	PVT_TAL,
	PVT_NET, /* Next event type */
	PVT_NODE,
	PVT_NSR,
	PVT_NXS,
	PVT_NYTN,
	PVT_NSF,
	PVT_ASN, /* Angle with surface normal */
	PVT_NTER,
	PVT_BN, /* Branch number */
	PVT_IPT,
	PVT_NCL,
	PVT_MAT,
	PVT_NCP,
	PVT_XXX,
	PVT_YYY,
	PVT_ZZZ,
	PVT_UUU,
	PVT_VVV,
	PVT_WWW,
	PVT_ERG,
	PVT_WGT,
	PVT_TME
};

/*
 * from table I-6 in LA-13709-M
 * and Appendix F, table 13-6 LA-CP-13-00634, Rev. 0
 */
enum PtracBankSource
{
	PBS_NONE,
	PBS_DXTRAN_TRACK,
	PBS_ENERGY_SPLIT,
	PBS_WEIGHT_WINDOW_SURFACE_SPLIT,
	PBS_WEIGHT_WINDOW_COLLISION_SPLIT,
	PBS_FORCED_COLLISION_UNCOLLIDED_PART,
	PBS_IMPORTANCE_SPLIT,
	PBS_NEUTRON_FROM_NEUTRON,
	PBS_PHOTON_FROM_NEUTRON,
	PBS_PHOTON_FROM_DOUBLE_FLUORESCENCE,
	PBS_PHOTON_FROM_ANNIHILATION,
	PBS_ELECTRON_FROM_PHOTOELECTRIC,
	PBS_ELECTRON_FROM_COMPTON,
	PBS_ELECTRON_FROM_PAIR_PRODUCTION,
	PBS_AUGER_ELECTRON_FROM_PHOTON,
	PBS_POSITRON_FROM_PAIR_PRODUCTION,
	PBS_BREMSSTRAHLUNG_FROM_ELECTRON,
	PBS_KNOCK_ON_ELECTRON,
	PBS_XRAYS_FROM_ELECTRON,
	PBS_PHOTON_FROM_NEUTRON_MG,
	PBS_NEUTRON_N_F_MG,
	PBS_NEUTRON_N_XN_K_MG,
	PBS_PHOTON_FROM_PHOTON,
	PBS_ADJOINT_WEIGHT_SPLIT,
	PBS_WEIGHT_WINDOW_PSEUDO_COLLISION_SPLIT,
	PBS_SECONDARY_FROM_PHOTONUCLEAR,
	PBS_DXTRAN_ANNIHILATION_PHOTON,
	PBS_LIGHT_IONS_FROM_NEUTRONS=30,
	PBS_LIGHT_IONS_FROM_PROTONS=31,
	PBS_LIBRARY_NEUTRONS_FROM_MODEL_NEUTRONS=32,
	PBS_SECONDARY_FROM_INELASTIC_NUCLEAR=33,
	PBS_SECONDARY_FROM_ELASTIC_NUCLEAR=34,
	PBS_INVALID
};

enum PtracEventType
{
	PET_NPS=0,
	PET_SRC=1000,
	PET_BNK=2000,
	PET_SUR=3000,
	PET_COL=4000,
	PET_TER=5000,
	PET_END=9000,
	PET_INVALID
};

struct PtracLineFormat
{
	enum PtracEventType type;
	size_t n_variables_a;
	size_t n_variables_b;
	enum PtracVariableType *variable_types;
};

struct PtracTrack
{
	enum PtracEventType event_type;
	enum PtracEventType next_event_type;
	int n_nodes; /* NODE */
	int source_number;  /* NSR */
	int bank_source;
	int zzaaa;         /* NXS */
	int reaction_type;  /* NYTN */
	int surface_number; /* NSF */
	int angle;         /* in degrees */
	int termination_type; /* NTER */
	int branch_number;
	int particle_type;  /* IPT */
	int cell_number;    /* NCL */
	int material_number; /* MAT */
	int n_collisions; /* NCP */

	double x;
	double y;
	double z;
	double u;
	double v;
	double w;
	double energy;
	double weight;
	double time;
};

struct PtracEvent
{
	size_t nps;
	enum PtracEventType initial_type;
	size_t initial_cell;
	size_t n_steps;
	struct PtracTrack *steps[PTRAC_MAX_N_STEPS];
};

struct PtracHeader
{
	char code[10];
	char version[10];
	char load_date[10];
	char machine_date[10];
	char machine_time[10];
	char title[82];
	size_t n_input_items;
	double *input_items;
	struct PtracLineFormat nps_format;
	struct PtracLineFormat format[PTRAC_N_LINE_TYPES];
	int transport_particle;
	int output_multiplier;
};

