#include <stdio.h>
#include <ptrac_file.h>
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>

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
	for (i = 0; i < PTRAC_N_LINE_TYPES; ++i) {
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
	n_col = 0;
	for (n = 0; n < PTRAC_N_LINE_TYPES; ++n) {
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
			printf("TYPE%lu[%lu] = '%s'\n", n, i,
			    ptrac_variable_type_names[
			    h->format[n].variable_types[i]]);
		}
	}

	return h;
}

int
ptrac_parse_event(struct PtracHeader *h, struct PtracFile *f,
    struct PtracEvent *ev)
{
	size_t i;
	char line[MAX_LINE_BUF];
	char *startptr, *endptr;

	/* Read NPS (event start) line */
	f->lineno++;
	fgets(line, MAX_LINE_BUF, f->file);
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

	return 0;
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
ptrac_to_root_tree(struct PtracEvent *ev, TTree *t)
{
	(void)ev;
	(void)t;
}

int
main(int argc, char *argv[])
{
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
	TClonesArray array("PtracStep", PTRAC_MAX_N_STEPS);

	init_root_tree(ev, tree, &array);

	for (;;) {
		int ok = 0;
		ok = ptrac_parse_event(h, input, ev);
		if (ok == 0) {
			break;
		}
		ptrac_to_root_tree(ev, tree);
	}

	ptrac_close_file(input);
	output->Close();

	return 0;
}
