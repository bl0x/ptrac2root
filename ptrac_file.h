#pragma once

struct PtracFile
{
	const char *name;
	FILE *file;
	size_t lineno;
	size_t n_input_lines;
};

#define PTRAC_N_LINE_TYPES 5
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

enum PtracEventType
{
	PET_NPS=0,
	PET_SRC=1000,
	PET_SUR=3000,
	PET_COL=4000,
	PET_TER=5000,
	PET_END=9000
};

struct PtracLineFormat
{
	enum PtracEventType type;
	size_t n_variables_a;
	size_t n_variables_b;
	enum PtracVariableType *variable_types;
};

struct PtracStep
{
	int next_event_type;
	int n_nodes; /* NODE */
	int source_number;  /* NSR */
	int zzaaa;         /* NXS */
	int recation_type;  /* NYTN */
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
	struct PtracStep *steps[PTRAC_MAX_N_STEPS];
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

