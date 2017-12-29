#include <stdio.h>
#include <ptrac_file.h>
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <PtracStep.h>

static const char *
ptrac_variable_type_names[] =
{
	"Unused",
	"History number",
	"Type of first event",
	"Cell number (NPS)",
	"Nearest surface (NPS)",
	"Tally specifier",
	"TFC specifier",
	"Next event type",
	"Number of nodes in track",
	"Source number",
	"ZZAAA for interaction",
	"Reaction type",
	"Surface number",
	"Angle with surface normal (deg)",
	"Termination type",
	"Branch number",
	"Particle type",
	"Cell number",
	"Material number",
	"Number of collisions",
	"x coordinate of event (cm)",
	"y coordinate of event (cm)",
	"z coordinate of event (cm)",
	"x coordinate of exit vector",
	"y coordinate of exit vector",
	"z coordinate of exit vector",
	"Energy of particle after event",
	"Weight of particle after event",
	"Time of event",
	NULL
};

struct PtracFile *
ptrac_open_file(const char *file_name, size_t n_input_lines)
{
	struct PtracFile *f = (struct PtracFile *)malloc(
	    sizeof(struct PtracFile));

	f->name = strdup(file_name);
	f->file = fopen(f->name, "r");
	if (f->file == NULL) {
		printf("Could not open file %s\n", f->name);
		abort();
	}

	f->lineno = 1;

	f->n_input_lines = n_input_lines;

	printf("Opening file '%s'\n", f->name);
	printf("Expecting %lu input lines\n", f->n_input_lines);

	return f;
}

void
ptrac_close_file(struct PtracFile *f)
{
	fclose(f->file);
}

#define MAX_LINE_BUF 255

static const enum PtracEventType ptrac_event_types[] =
{
	PET_NPS,
	PET_SRC,
	PET_BNK,
	PET_SUR,
	PET_COL,
	PET_TER,
	PET_END,
	PET_INVALID
};

struct PtracHeader *
ptrac_parse_header(struct PtracFile *f)
{
	size_t i, k, n;
	size_t n_col;
	int identifier = 0;
	char line[MAX_LINE_BUF];
	char *startptr, *endptr;

	struct PtracHeader *h = (struct PtracHeader *)malloc(
	    sizeof(struct PtracHeader));

	/* Read first line */
	fgets(line, MAX_LINE_BUF, f->file);
	sscanf(line, "%d", &identifier);
	if (identifier != -1) {
		printf("PTRAC file identifier mismatch on line %lu: %d != -1\n",
		   f->lineno, identifier);
		abort();
	}

	/* Read second line */
	f->lineno++;
	fgets(line, MAX_LINE_BUF, f->file);
	sscanf(line, "%s %s %s %s %s", h->code, h->version, h->load_date,
	    h->machine_date, h->machine_time);
	if (strncmp(h->code, "mcnp", 4) != 0) {
		printf("Expected code 'mcnp' on line %lu. Got %s.\n",
		    f->lineno, h->code);
	}
	printf("MCNP version = '%s'\n", h->version);
	printf("Load date = '%s'\n", h->load_date);
	printf("Machine date = '%s'\n", h->machine_date);
	printf("Machine time = '%s'\n", h->machine_time);

	/* Read third line (title) */
	f->lineno++;
	fgets(h->title, MAX_LINE_BUF, f->file);
	h->title[strlen(h->title)-1] = '\0';
	printf("Title = '%s'\n", h->title);

	/* Read K input lines */
	h->n_input_items = f->n_input_lines * 10;
	h->input_items = (double *)malloc(sizeof(double) * h->n_input_items);
	for (k = 0; k < f->n_input_lines; ++k) {
		f->lineno++;
		printf("Reading input line %lu (lineno %lu)\n", k, f->lineno);
		fgets(line, MAX_LINE_BUF, f->file);
		sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			&h->input_items[k * 10 + 0],
			&h->input_items[k * 10 + 1],
			&h->input_items[k * 10 + 2],
			&h->input_items[k * 10 + 3],
			&h->input_items[k * 10 + 4],
			&h->input_items[k * 10 + 5],
			&h->input_items[k * 10 + 6],
			&h->input_items[k * 10 + 7],
			&h->input_items[k * 10 + 8],
			&h->input_items[k * 10 + 9]);
	}
	printf("Input data (%lu):\n", h->n_input_items);
	for (i = 0; i < h->n_input_items; ++i) {
		printf("%f ", h->input_items[i]);
		if ((i + 1) % 8 == 0) {
			printf("\n");
		}
	}
	printf("\n");

	/* Read line format */
	f->lineno++;
	fgets(line, MAX_LINE_BUF, f->file);
	h->nps_format.type = PET_NPS;
	startptr = line;
	endptr = line;
	h->nps_format.n_variables_a = strtol(startptr, &endptr, 10);
	startptr = endptr;
	h->nps_format.n_variables_b = 0;
	h->nps_format.variable_types = (enum PtracVariableType *)
	    malloc(sizeof(enum PtracVariableType)
		* h->nps_format.n_variables_a);

	for (i = 1; i < PTRAC_N_LINE_TYPES; ++i) {

		h->format[i].type = ptrac_event_types[i];
		h->format[i].n_variables_a = strtol(startptr, &endptr, 10);
		startptr = endptr;
		h->format[i].n_variables_b = strtol(startptr, &endptr, 10);
		startptr = endptr;
		h->format[i].variable_types = (enum PtracVariableType *)
		    malloc(sizeof(enum PtracVariableType)
			* (h->format[i].n_variables_a
				+ h->format[i].n_variables_b));
	}

	h->transport_particle = strtol(startptr, &endptr, 10);
	startptr = endptr;
	printf("Transport particle = %d\n", h->transport_particle);
	h->output_multiplier = strtol(startptr, &endptr, 10);
	startptr = endptr;
	printf("Output multiplier = %d\n", h->output_multiplier);

	/* Read variable type lines */
	f->lineno++;
	fgets(line, MAX_LINE_BUF, f->file);
	endptr = line;
	startptr = line;
	for (i = 0; i < h->nps_format.n_variables_a; ++i) {
		int val;
		val = strtol(startptr, &endptr, 10);
		startptr = endptr;
		h->nps_format.variable_types[i]	= (enum PtracVariableType)val;
		printf("NPS[%lu] = '%s'\n", i, ptrac_variable_type_names[
		    h->nps_format.variable_types[i]]);
	}
	n_col = h->nps_format.n_variables_a;
	for (n = 1; n < PTRAC_N_LINE_TYPES; ++n) {
		size_t cols = h->format[n].n_variables_a
		    + h->format[n].n_variables_b;
		for (i = 0; i < cols; ++i) {
			int val;
			val = strtol(startptr, &endptr, 10);
			startptr = endptr;
			n_col++;
			if (n_col == 30) {
				n_col = 0;
				f->lineno++;
				fgets(line, MAX_LINE_BUF, f->file);
				endptr = line;
				startptr = line;
			}
			h->format[n].variable_types[i] =
			    (enum PtracVariableType)val;
			printf("TYPE%d[%lu] = '%s'\n", ptrac_event_types[n], i,
			    ptrac_variable_type_names[
			    h->format[n].variable_types[i]]);
		}
	}

	return h;
}

int
is_valid_event_type(enum PtracEventType type)
{
	enum PtracEventType t = type;

	/* Handle various BNK types */
	if (((t / 1000) * 1000) == PET_BNK) {
		t = (enum PtracEventType) ((t / 1000) * 1000);
	}

	switch(t)
	{
	case PET_NPS:
	case PET_BNK:
	case PET_SRC:
	case PET_SUR:
	case PET_COL:
	case PET_TER:
	case PET_END:
		return 1;
	default:
		return 0;
	}
}

int
ptrac_parse_event(struct PtracHeader *h, struct PtracFile *f,
    struct PtracEvent *ev)
{
	size_t i;
	char line[MAX_LINE_BUF];
	char *startptr, *endptr;
	enum PtracEventType event_type;
	char *ret;

	/* Read NPS (event start) line */
	f->lineno++;
	ret = fgets(line, MAX_LINE_BUF, f->file);
	if (ret == NULL) {
		return 0;
	}
	endptr = line;
	startptr = line;

	for (i = 0; i < h->nps_format.n_variables_a; ++i) {
		int val;
		val = strtol(startptr, &endptr, 10);
		startptr = endptr;
		switch(h->nps_format.variable_types[i]) {
		case PVT_NPS:
			ev->nps = val;
			break;
		case PVT_TFH:
			ev->initial_type = (enum PtracEventType)val;
			break;
		case PVT_NCL_NPS:
			ev->initial_cell = val;
			break;
		default:
			printf("Unhandled NPS line field.\n");
			abort();
		}
		printf("%s = %d\n", ptrac_variable_type_names[
		    h->nps_format.variable_types[i]], val);
	}

	printf("event %lu\n", ev->nps);

	/* Read all event lines until PET_END */
	ev->n_steps = 0;
	event_type = (enum PtracEventType)((ev->initial_type / 1000) * 1000);
	for (;;) {
		int format_id;
		enum PtracBankSource bnk_source;

		printf("  step %lu\n", ev->n_steps);

		/* Read first step line */
		f->lineno++;
		fgets(line, MAX_LINE_BUF, f->file);
		endptr = line;
		startptr = line;

		printf("  event_type %d\n", event_type);

		if(!is_valid_event_type(event_type)) {
			printf("Invalid event type '%d' in line %lu\n",
			    (int)event_type, f->lineno);
			abort();
		}

		if (ev->n_steps + 1 == PTRAC_MAX_N_STEPS) {
			printf("Error: Too many steps (> %d) in event.\n",
			    PTRAC_MAX_N_STEPS);
			printf("       Increase PTRAC_MAX_N_STEPS.\n");
			abort();
		}

		/* Handle PET_BNK numbers */
		if ((event_type / 1000) * 1000 == PET_BNK) {
			bnk_source = (enum PtracBankSource)(event_type % 1000);
			event_type = (enum PtracEventType)
			    ((event_type / 1000) * 1000);
			ev->steps[ev->n_steps]->bank_source = bnk_source;
		}

		format_id = -1;
		for (i = 0; i < PTRAC_N_LINE_TYPES; ++i) {
			if (h->format[i].type == event_type) {
				format_id = i;
			}
		}
		if (format_id == -1) {
			printf("Unhandled next event type '%d' in line %lu\n",
			    (int)event_type, f->lineno);
			abort();
		} else {
			printf("  format id = %d\n", format_id);
		}

		ev->steps[ev->n_steps]->event_type = event_type;
		for (i = 0; i < h->format[format_id].n_variables_a; ++i) {
			int val;
			val = strtol(startptr, &endptr, 10);
			startptr = endptr;
			enum PtracVariableType var_type =
			    h->format[format_id].variable_types[i];

			printf("    field col %lu\n", i);
			switch(var_type) {
			case PVT_NET:
				ev->steps[ev->n_steps]->next_event_type =
				    (enum PtracEventType)(val);
				break;
			case PVT_NODE:
				ev->steps[ev->n_steps]->n_nodes = val;
				break;
			case PVT_NSR:
				ev->steps[ev->n_steps]->source_number = val;
				break;
			case PVT_NXS:
				ev->steps[ev->n_steps]->zzaaa = val;
				break;
			case PVT_NYTN:
				ev->steps[ev->n_steps]->reaction_type = val;
				break;
			case PVT_NSF:
				ev->steps[ev->n_steps]->surface_number = val;
				break;
			case PVT_ASN:
				ev->steps[ev->n_steps]->angle = val;
				break;
			case PVT_NTER:
				ev->steps[ev->n_steps]->termination_type = val;
				break;
			case PVT_BN:
				ev->steps[ev->n_steps]->branch_number = val;
				break;
			case PVT_IPT:
				ev->steps[ev->n_steps]->particle_type = val;
				break;
			case PVT_NCL:
				ev->steps[ev->n_steps]->cell_number = val;
				break;
			case PVT_MAT:
				ev->steps[ev->n_steps]->material_number = val;
				break;
			case PVT_NCP:
				ev->steps[ev->n_steps]->n_collisions = val;
				break;
			default:
				printf("Unhandled event line 1 field "
				    "(%d = %s). in line %lu\n", var_type,
				    ptrac_variable_type_names[var_type],
				    f->lineno);
				abort();
			}
			printf("      %s = %d\n", ptrac_variable_type_names[
			    var_type], val);
		}

		/* Read second step line */
		f->lineno++;
		fgets(line, MAX_LINE_BUF, f->file);
		endptr = line;
		startptr = line;

		for (i = 0; i < h->format[format_id].n_variables_b; ++i) {
			double val;
			val = strtod(startptr, &endptr);
			startptr = endptr;
			enum PtracVariableType var_type =
			    h->format[format_id].variable_types[
			        h->format[format_id].n_variables_a + i];
			printf("    field col %lu\n", i);
			switch(var_type) {
			case PVT_NODE:
				ev->steps[ev->n_steps]->n_nodes = val;
				break;
			case PVT_NXS:
				ev->steps[ev->n_steps]->zzaaa = val;
				break;
			case PVT_NYTN:
				ev->steps[ev->n_steps]->reaction_type = val;
				break;
			case PVT_NSF:
				ev->steps[ev->n_steps]->surface_number = val;
				break;
			case PVT_ASN:
				ev->steps[ev->n_steps]->angle = val;
				break;
			case PVT_IPT:
				ev->steps[ev->n_steps]->particle_type = val;
				break;
			case PVT_NTER:
				ev->steps[ev->n_steps]->termination_type = val;
				break;
			case PVT_BN:
				ev->steps[ev->n_steps]->branch_number = val;
				break;
			case PVT_NCL:
				ev->steps[ev->n_steps]->cell_number = val;
				break;
			case PVT_MAT:
				ev->steps[ev->n_steps]->material_number = val;
				break;
			case PVT_NCP:
				ev->steps[ev->n_steps]->n_collisions = val;
				break;
			case PVT_XXX:
				ev->steps[ev->n_steps]->x = val;
				break;
			case PVT_YYY:
				ev->steps[ev->n_steps]->y = val;
				break;
			case PVT_ZZZ:
				ev->steps[ev->n_steps]->z = val;
				break;
			case PVT_UUU:
				ev->steps[ev->n_steps]->u = val;
				break;
			case PVT_VVV:
				ev->steps[ev->n_steps]->v = val;
				break;
			case PVT_WWW:
				ev->steps[ev->n_steps]->w = val;
				break;
			case PVT_ERG:
				ev->steps[ev->n_steps]->energy = val;
				break;
			case PVT_WGT:
				ev->steps[ev->n_steps]->weight = val;
				break;
			case PVT_TME:
				ev->steps[ev->n_steps]->time = val;
				break;
			default:
				printf("Unhandled event line 2 field "
				    "(%d = %s) in line %lu.\n", var_type,
				    ptrac_variable_type_names[var_type],
				    f->lineno);
				abort();
			}
			printf("      %s = %lf\n", ptrac_variable_type_names[
			    var_type], val);
		}

		++ev->n_steps;
		event_type =(enum PtracEventType)
		    (ev->steps[ev->n_steps-1]->next_event_type);

		if (event_type == PET_END) {
			printf("  Found final step marker.\n");
			break;
		}
	}

	return 1;
}

void
init_root_tree(struct PtracEvent *ev, TTree *t, TClonesArray *array)
{
	/* Setup branches */
	t->Branch("NPS", &ev->nps, "NPS/l");
	t->Branch("InitialType", (int *)&ev->initial_type, "InitialType/I");
	t->Branch("InitialCell", &ev->initial_cell, "InitialCell/l");
	t->Branch("PtracSteps", array);
}

void
ptrac_to_root_tree(struct PtracEvent *ev, TTree *t, TClonesArray *array)
{
	size_t n;
	for (n = 0; n < ev->n_steps; ++n) {
		auto step = (PtracStep *)array->ConstructedAt(n);
		step->fEventType = ev->steps[n]->event_type;
		step->fNextEventType = ev->steps[n]->next_event_type;
		step->fNumberOfNodes = ev->steps[n]->n_nodes;
		step->fSourceNumber = ev->steps[n]->source_number;
		step->fBankSource = ev->steps[n]->bank_source;
		step->fZZAAA = ev->steps[n]->zzaaa;
		step->fReactionType = ev->steps[n]->reaction_type;
		step->fSurfaceNumber = ev->steps[n]->surface_number;
		step->fAngle = ev->steps[n]->angle;
		step->fTerminationType = ev->steps[n]->termination_type;
		step->fBranchNumber = ev->steps[n]->branch_number;
		step->fParticleType = ev->steps[n]->particle_type;
		step->fCellNumber = ev->steps[n]->cell_number;
		step->fMaterialNumber = ev->steps[n]->material_number;
		step->fNumberOfCollisions = ev->steps[n]->n_collisions;
		step->fX = ev->steps[n]->x;
		step->fY = ev->steps[n]->y;
		step->fZ = ev->steps[n]->z;
		step->fU = ev->steps[n]->u;
		step->fV = ev->steps[n]->v;
		step->fW = ev->steps[n]->w;
		step->fEnergy = ev->steps[n]->energy;
		step->fWeight = ev->steps[n]->weight;
		step->fTime = ev->steps[n]->time;
	}
	t->Fill();
	array->Clear();
}

int
main(int argc, char *argv[])
{
	size_t i;
	if (argc < 3) {
		printf("Usage: %s <ptrac input file> <n input lines>\n",
		    argv[0]);
		abort();
	}

	auto input = ptrac_open_file(argv[1], atoi(argv[2]));
	auto output = new TFile("out.root", "RECREATE");
	auto tree = new TTree("events", "events");

	struct PtracHeader *h = ptrac_parse_header(input);
	struct PtracEvent *ev;

	ev = (struct PtracEvent *)malloc(sizeof(struct PtracEvent));
	for (i = 0; i < PTRAC_MAX_N_STEPS; ++i) {
		ev->steps[i] = (PtracTrack *)malloc(sizeof(struct PtracTrack));
	}
	TClonesArray array("PtracStep", PTRAC_MAX_N_STEPS);

	init_root_tree(ev, tree, &array);

	for (;;) {
		int ok = 0;
		ok = ptrac_parse_event(h, input, ev);
		if (ok == 0) {
			break;
		}
		ptrac_to_root_tree(ev, tree, &array);
	}

	ptrac_close_file(input);
	tree->Write();
	output->Close();

	return 0;
}
