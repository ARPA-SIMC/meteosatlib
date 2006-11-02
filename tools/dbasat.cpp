//---------------------------------------------------------------------------
//
//  File        :   dbasat.cpp
//  Description :   Interpolate image values according to a given set of
//                  (lat,lon) coordinates, and fed the results to dballe
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  RCS ID      :   $Id: /local/meteosatlib/tools/msatconv.cpp 1879 2006-10-11T15:01:33.022993Z enrico  $
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//---------------------------------------------------------------------------

#include <config.h>

#include <conv/ImportGRIB.h>
#include <conv/ImportSAFH5.h>
#include <conv/ImportNetCDF.h>
#include <conv/ImportNetCDF24.h>
#include <conv/ImportXRIT.h>

#include <dballe/init.h>
#include <dballe/cmdline.h>
#include <dballe/db/dba_db.h>
#include <dballe/core/csv.h>

#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;
using namespace msat;

static const char* op_dsn = "test";
static const char* op_user = "";
static const char* op_pass = "";
static const char* op_area = "";
#if 0
static char* op_input_type = "auto";
static char* op_report = "";
static char* op_output_type = "bufr";
static char* op_output_template = "";
static int op_overwrite = 0;
static int op_fast = 0;
#endif
int op_verbose = 0;

struct poptOption dbTable[] = {
	{ "dsn", 0, POPT_ARG_STRING, &op_dsn, 0,
		"DSN to use for connecting to the DBALLE database" },
	{ "user", 0, POPT_ARG_STRING, &op_user, 0,
		"username to use for connecting to the DBALLE database" },
	{ "pass", 0, POPT_ARG_STRING, &op_pass, 0,
		"password to use for connecting to the DBALLE database" },
	POPT_TABLEEND
};

static dba_err create_dba_db(dba_db* db)
{
	if (op_user[0] == 0)
	{
		struct passwd *pwd = getpwuid(getuid());
		op_user = pwd == NULL ? "test" : pwd->pw_name;
	}
	return dba_db_create(op_dsn, op_user, op_pass, db);
}

/*
 * Create an importer for the given file, auto-detecting the file type.
 * 
 * If no supported file type could be detected, returns an empty auto_ptr.
 */
std::auto_ptr<ImageImporter> getImporter(const std::string& filename)
{
	if (isGrib(filename))
		return createGribImporter(filename);
	if (isNetCDF(filename))
		return createNetCDFImporter(filename);
	if (isNetCDF24(filename))
		return createNetCDF24Importer(filename);
	if (isSAFH5(filename))
		return createSAFH5Importer(filename);
	if (isXRIT(filename))
		return createXRITImporter(filename);
	return std::auto_ptr<ImageImporter>();
}


static dba_err read_files(poptContext optCon, ImageVector& imgs)
{
	// Default cropping parameters (all zeroes mean "no cropping")
	int ax = 0, ay = 0, aw = 0, ah = 0;
	if (op_area[0] != 0)
		if (sscanf(optarg, "%d,%d,%d,%d", &ax,&ay,&aw,&ah) != 4)
			dba_cmdline_error(optCon, "area parameter should be in the format x,y,width,height");

	// Get the file names and read the images
	while (1)
	{
		const char* arg = poptPeekArg(optCon);
		if (arg == NULL) break;
		// Stop processing files when a query parameter is found
		if (strchr(arg, '=') != NULL) break;

		try
		{
			std::auto_ptr<ImageImporter> importer = getImporter(arg);
			if (!importer.get())
			{
				cerr << "No importer found for " << arg << ": ignoring." << endl;
				continue;
			}
			importer->cropX = ax;
			importer->cropY = ay;
			importer->cropWidth = aw;
			importer->cropHeight = ah;
			importer->read(imgs);
		}
		catch (std::exception& e)
		{
			cerr << "Importing " << arg << " failed: ignoring." << endl;
			continue;
		}

		// Consume the argument if it was processed
		poptGetArg(optCon);
	}
	return dba_error_ok();
}

struct ChannelInfo
{
	dba_varcode var;
	int ltype, l1, l2;
	int pind, p1, p2;
};

struct ChannelTab : public std::map<int, ChannelInfo>
{
	dba_err read(const std::string& file)
	{
		FILE* in = fopen(file.c_str(), "rt");
		if (in == NULL)
			return dba_error_system("Opening file %s", file.c_str());

		char *cols[10];
		int count;
		while ((count = dba_csv_read_next(in, cols, 10)) != 0)
		{
			if (count < 8)
				continue;
			int channel = strtoul(cols[0], NULL, 0);
			(*this)[channel].var = DBA_STRING_TO_VAR(cols[1]+1);
			(*this)[channel].ltype = strtoul(cols[2], NULL, 0);
			(*this)[channel].l1 = strtoul(cols[3], NULL, 0);
			(*this)[channel].l2 = strtoul(cols[4], NULL, 0);
			(*this)[channel].pind = strtoul(cols[5], NULL, 0);
			(*this)[channel].p1 = strtoul(cols[6], NULL, 0);
			(*this)[channel].p2 = strtoul(cols[7], NULL, 0);

			for (int i = 0; i < count; ++i)
				free(cols[i]);
		}

		fclose(in);
		return dba_error_ok();
	}
};

dba_err do_dump(poptContext optCon)
{
	static const char* satinfo_fname = "satinfo.csv";
	const char* action;
	int count, i;
	dba_record query, result;
	dba_db_cursor cursor;
	dba_db db;

	/* Throw away the command name */
	action = poptGetArg(optCon);

	ChannelTab satinfo;
	satinfo.read(satinfo_fname);

	// Read the input data
	ImageVector imgs;
	DBA_RUN_OR_RETURN(read_files(optCon, imgs));

	if (imgs.empty())
		dba_cmdline_error(optCon, "No valid input files found");

	/* Create the query */
	DBA_RUN_OR_RETURN(dba_record_create(&query));
	DBA_RUN_OR_RETURN(dba_cmdline_get_query(optCon, query));

	// Connect the database and start the query
	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));
	DBA_RUN_OR_RETURN(dba_db_ana_query(db, query, &cursor, &count));
	DBA_RUN_OR_RETURN(dba_record_create(&result));

	// Read the stations from the database and print the corresponding image
	// values
	set<int> warned;
	for (i = 0; ; ++i)
	{
		int has_data;
		DBA_RUN_OR_RETURN(dba_db_cursor_next(cursor, &has_data));
		if (!has_data)
			break;
		dba_record_clear(result);
		DBA_RUN_OR_RETURN(dba_db_cursor_to_record(cursor, result));

		double lat, lon;
		DBA_RUN_OR_RETURN(dba_record_key_enqd(result, DBA_KEY_LAT, &lat));
		DBA_RUN_OR_RETURN(dba_record_key_enqd(result, DBA_KEY_LON, &lon));

		for (ImageVector::const_iterator i = imgs.begin();
					i != imgs.end(); ++i)
		{
			int x, y;
			int channel = (*i)->channel_id;
			ChannelTab::const_iterator c = satinfo.find(channel);
			if (c == satinfo.end())
			{
				if (warned.find(channel) == warned.end())
				{
					fprintf(stderr, "No channel information for channel %d in %s: skipping image.\n",
							channel, satinfo_fname);
					warned.insert(channel);
				}
				continue;
			}

			(*i)->coordsToPixels(lat, lon, x, y);
			//fprintf(stderr, "  (%f,%f) -> (%d,%d)\n", lat, lon, x, y);
			if (x >= 0 && x < (*i)->data->columns && y >= 0 && y < (*i)->data->lines)
			{
				dba_var var;
				DBA_RUN_OR_RETURN(dba_var_create_local(c->second.var, &var));
				DBA_RUN_OR_RETURN(dba_var_setd(var, (*i)->data->scaled(x, y)));
				fprintf(stdout, "%f,%f: %04d-%02d-%02d %02d:%02d %d,%d,%d %d,%d,%d", lat, lon,
						(*i)->year, (*i)->month, (*i)->day, (*i)->hour, (*i)->minute,
						c->second.ltype, c->second.l1, c->second.l2,
						c->second.pind, c->second.p1, c->second.p2);
				dba_var_print(var, stdout);
				dba_var_delete(var);
			}
		}
	}

	dba_db_delete(db);
	dba_shutdown();

	dba_record_delete(result);
	dba_record_delete(query);

	return dba_error_ok();
}









static struct tool_desc dbasat;

struct poptOption dbasat_dump_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "area", 'a', POPT_ARG_STRING, &op_area, 0, "read only the given subarea (in pixels) of the images (format: x,y,w,h)" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};

#if 0
struct poptOption dbasat_wipe_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};

struct poptOption dbasat_import_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ "type", 't', POPT_ARG_STRING, &op_input_type, 0,
		"format of the input data ('bufr', 'crex', 'aof')", "type" },
	{ "overwrite", 'f', POPT_ARG_NONE, &op_overwrite, 0,
		"overwrite existing data" },
	{ "report", 'r', POPT_ARG_STRING, &op_report, 0,
		"force data to be of this type of report, specified with rep_cod or rep_memo values", "rep" },
	{ "fast", 0, POPT_ARG_NONE, &op_fast, 0,
		"make import faster, but an interruption will mean that there can be "
		"data in the database for only part of an imported message" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &grepTable, 0,
		"Options used to filter messages" },
	POPT_TABLEEND
};

struct poptOption dbasat_export_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ "report", 'r', POPT_ARG_STRING, &op_report, 0,
		"force exported data to be of this type of report, specified with rep_cod or rep_memo values", "rep" },
	{ "dest", 'd', POPT_ARG_STRING, &op_output_type, 0,
		"format of the data in output ('bufr', 'crex', 'aof')", "type" },
	{ "template", 't', POPT_ARG_STRING, &op_output_template, 0,
		"template of the data in output (autoselect if not specified)", "type.sub" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};

struct poptOption dbasat_repinfo_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};

struct poptOption dbasat_cleanup_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};

struct poptOption dbasat_stations_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0,
		"Options used to connect to the database" },
	POPT_TABLEEND
};
#endif


static void init()
{
	dbasat.desc = "DB-ALLe utility for satellite data";
	dbasat.longdesc =
		"It allows to correlate satellite data with DB-ALLe stations and import the results "
		"in the database.  It can import satellite data in Grib"
#ifdef HAVE_HRIT
		", HRIT"
#endif
#ifdef HAVE_HDF5
		", SAFH5"
#endif
#ifdef HAVE_NETCDF
		", NetCDF, NetCDF24"
#endif
		" format.";
	dbasat.ops = (struct op_dispatch_table*)calloc(2, sizeof(struct op_dispatch_table));

	dbasat.ops[0].func = do_dump;
	dbasat.ops[0].aliases[0] = "dump";
	dbasat.ops[0].usage = "dump [options] [file1 [file2...]] [queryparm1=val1 [queryparm2=val2 [...]]]";
	dbasat.ops[0].desc = "Dump satellite data from the stations found in the database matching the given query";
	dbasat.ops[0].longdesc = "Query parameters are the same of the Fortran API. "
		"Please see the section \"Input and output parameters -- For data "
		"related action routines\" of the Fortran API documentation for a "
		"complete list.";
	dbasat.ops[0].optable = dbasat_dump_options;

#if 0
	dbasat.ops[1].func = do_wipe;
	dbasat.ops[1].aliases[0] = "wipe";
	dbasat.ops[1].usage = "wipe [options] [optional rep_memo description file]";
	dbasat.ops[1].desc = "Reinitialise the database, removing all data";
	dbasat.ops[1].longdesc =
			"Reinitialisation is done using the given report code description file. "
			"If no file is provided, a default version is used";
	dbasat.ops[1].optable = dbasat_wipe_options;

	dbasat.ops[2].func = do_import;
	dbasat.ops[2].aliases[0] = "import";
	dbasat.ops[2].usage = "import [options] filename [filename [ ... ] ]";
	dbasat.ops[2].desc = "Import data into the database";
	dbasat.ops[2].longdesc = NULL;
	dbasat.ops[2].optable = dbasat_import_options;

	dbasat.ops[3].func = do_export;
	dbasat.ops[3].aliases[0] = "export";
	dbasat.ops[3].usage = "export [options] rep_memo [queryparm1=val1 [queryparm2=val2 [...]]]";
	dbasat.ops[3].desc = "Export data from the database";
	dbasat.ops[3].longdesc = "Query parameters are the same of the Fortran API. "
		"Please see the section \"Input and output parameters -- For data "
		"related action routines\" of the Fortran API documentation for a "
		"complete list.";
	dbasat.ops[3].optable = dbasat_export_options;

	dbasat.ops[4].func = do_repinfo;
	dbasat.ops[4].aliases[0] = "repinfo";
	dbasat.ops[4].usage = "repinfo [options] [filename]";
	dbasat.ops[4].desc = "Update the report information table";
	dbasat.ops[4].longdesc =
			"Update the report information table with the data from the given "
			"report code description file.  "
			"If no file is provided, a default version is used";
	dbasat.ops[4].optable = dbasat_repinfo_options;

	dbasat.ops[5].func = do_cleanup;
	dbasat.ops[5].aliases[0] = "cleanup";
	dbasat.ops[5].usage = "cleanup [options]";
	dbasat.ops[5].desc = "Perform database cleanup operations";
	dbasat.ops[5].longdesc =
			"The only operation currently performed by this command is "
			"deleting stations that have no values.  If more will be added in "
			"the future, they will be documented here.";
	dbasat.ops[5].optable = dbasat_cleanup_options;

	dbasat.ops[6].func = do_stations;
	dbasat.ops[6].aliases[0] = "stations";
	dbasat.ops[6].usage = "stations [options] [queryparm1=val1 [queryparm2=val2 [...]]]";
	dbasat.ops[6].desc = "List the stations present in the database";
	dbasat.ops[6].longdesc = "Query parameters are the same of the Fortran API. "
		"Please see the section \"Input and output parameters -- For data "
		"related action routines\" of the Fortran API documentation for a "
		"complete list.";
	dbasat.ops[6].optable = dbasat_stations_options;
#endif

	dbasat.ops[1].func = NULL;
	dbasat.ops[1].usage = NULL;
	dbasat.ops[1].desc = NULL;
	dbasat.ops[1].longdesc = NULL;
	dbasat.ops[1].optable = NULL;
};

int main (int argc, const char* argv[])
{
	init();
	return dba_cmdline_dispatch_main(&dbasat, argc, argv);
}














#if 0

#include <dballe/msg/dba_msg.h>

#include <dballe/dba_file.h>
/*
#include <dballe/bufrex/bufr_file.h>
#include <dballe/bufrex/crex_file.h>
#include <dballe/aof/aof_file.h>

#include <dballe/bufrex/bufrex_conv.h>
#include <dballe/aof/aof_conv.h>
*/

#include <dballe/db/import.h>
#include <dballe/db/export.h>

#include <dballe/bufrex/bufrex.h>


#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct import_data
{
	dba_db db;
	int overwrite;
	int forced_repcod;
};

static dba_err import_message(dba_rawmsg rmsg, bufrex_raw braw, dba_msg msg, void* data)
{
	struct import_data* d = (struct import_data*)data;
	if (msg == NULL)
	{
		fprintf(stderr, "Message #%d cannot be parsed: ignored\n",
				rmsg->index);
	}
	else if (d->forced_repcod == -1 && msg->type == MSG_GENERIC)
	{
		/* Put generic messages in the generic rep_cod by default */
		DBA_RUN_OR_RETURN(dba_import_msg(d->db, msg, 255, d->overwrite, op_fast));
	}
	else
	{
		DBA_RUN_OR_RETURN(dba_import_msg(d->db, msg, d->forced_repcod, d->overwrite, op_fast));
	}
	return dba_error_ok();
}

dba_err do_stations(poptContext optCon)
{
	const char* action;
	int count, i;
	dba_record query, result;
	dba_db_cursor cursor;
	dba_db db;

	/* Throw away the command name */
	action = poptGetArg(optCon);

	/* Create the query */
	DBA_RUN_OR_RETURN(dba_record_create(&query));
	DBA_RUN_OR_RETURN(dba_cmdline_get_query(optCon, query));

	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));
	DBA_RUN_OR_RETURN(dba_db_ana_query(db, query, &cursor, &count));
	DBA_RUN_OR_RETURN(dba_record_create(&result));

	for (i = 0; ; ++i)
	{
		int has_data;
		DBA_RUN_OR_RETURN(dba_db_cursor_next(cursor, &has_data));
		if (!has_data)
			break;
		dba_record_clear(result);
		DBA_RUN_OR_RETURN(dba_db_cursor_to_record(cursor, result));
		printf("#%d: -----------------------\n", i);
		dba_record_print(result, stdout);
	}

	dba_db_delete(db);
	dba_shutdown();

	dba_record_delete(result);
	dba_record_delete(query);

	return dba_error_ok();
}

dba_err do_wipe(poptContext optCon)
{
	const char* action;
	const char* table;
	dba_db db;

	/* Throw away the command name */
	action = poptGetArg(optCon);

	/* Get the optional name of the repinfo file */
	table = poptGetArg(optCon);

	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));
	DBA_RUN_OR_RETURN(dba_db_reset(db, table));
	dba_db_delete(db);
	dba_shutdown();

	return dba_error_ok();
}

dba_err do_cleanup(poptContext optCon)
{
	const char* action;
	const char* table;
	dba_db db;

	/* Throw away the command name */
	action = poptGetArg(optCon);

	/* Get the optional name of the repinfo file */
	table = poptGetArg(optCon);

	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));
	DBA_RUN_OR_RETURN(dba_db_remove_orphans(db));
	dba_db_delete(db);
	dba_shutdown();

	return dba_error_ok();
}

dba_err do_repinfo(poptContext optCon)
{
	int added, deleted, updated;
	const char* action;
	const char* table;
	dba_db db;

	/* Throw away the command name */
	action = poptGetArg(optCon);

	/* Get the optional name of the */
	table = poptGetArg(optCon);

	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));
	DBA_RUN_OR_RETURN(dba_db_update_repinfo(db, table, &added, &deleted, &updated));
	printf("Update completed: %d added, %d deleted, %d updated.\n", added, deleted, updated);
	dba_db_delete(db);
	dba_shutdown();

	return dba_error_ok();
}

dba_err parse_op_report(dba_db db, int* res)
{
	if (op_report[0] != 0)
	{
		const char* s;
		int is_cod = 1;
		for (s = op_report; *s && is_cod; s++)
			if (!isdigit(*s))
				is_cod = 0;
		
		if (is_cod)
		{
			int valid;
			*res = strtoul(op_report, NULL, 0);
			DBA_RUN_OR_RETURN(dba_db_check_rep_cod(db, *res, &valid));
			if (valid)
				return dba_error_ok();
			else
				return dba_error_consistency("report code %d not found in the database", *res);
		}
		else
		{
			return dba_db_rep_cod_from_memo(db, op_report, res);
		}
	} else {
		*res = -1;
		return dba_error_ok();
	}
}

dba_err do_import(poptContext optCon)
{
	dba_encoding type;
	struct import_data data;

	/* Throw away the command name */
	poptGetArg(optCon);

	type = dba_cmdline_stringToMsgType(op_input_type, optCon);

	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&data.db));
	data.overwrite = op_overwrite;
	DBA_RUN_OR_RETURN(parse_op_report(data.db, &(data.forced_repcod)));

	DBA_RUN_OR_RETURN(process_all(optCon, type, &grepdata, import_message, (void*)&data));

	dba_db_delete(data.db);
	dba_shutdown();

	return dba_error_ok();
}

struct export_data
{
	dba_file file;
	int cat;
	int subcat;
	int forced_rep_cod;
};

static dba_err msg_writer(dba_msg msg, void* data)
{
	struct export_data* d = (struct export_data*)data;
	/* Override the message type if the user asks for it */
	if (d->forced_rep_cod != -1)
		msg->type = dba_msg_type_from_repcod(d->forced_rep_cod);
	DBA_RUN_OR_RETURN(dba_file_write(d->file, msg, d->cat, d->subcat));
	dba_msg_delete(msg);
	return dba_error_ok();
}

dba_err do_export(poptContext optCon)
{
	struct export_data d = { NULL, 0, 0, -1 };
	dba_encoding type;
	dba_record query;
	dba_db db;

	/* Throw away the command name */
	poptGetArg(optCon);

	if (op_output_template[0] != 0)
		if (sscanf(op_output_template, "%d.%d", &d.cat, &d.subcat) != 2)
			dba_cmdline_error(optCon, "output template must be specified as 'type.subtype' (type number, then dot, then subtype number)");

	/* Connect to the database */
	DBA_RUN_OR_RETURN(dba_init());
	DBA_RUN_OR_RETURN(create_dba_db(&db));

	/* Create the query */
	DBA_RUN_OR_RETURN(dba_record_create(&query));

	/* Add the query data from commandline */
	DBA_RUN_OR_RETURN(dba_cmdline_get_query(optCon, query));

	DBA_RUN_OR_RETURN(parse_op_report(db, &(d.forced_rep_cod)));

	type = dba_cmdline_stringToMsgType(op_output_type, optCon);
	DBA_RUN_OR_RETURN(dba_file_create(&d.file, type, "(stdout)", "w"));

	DBA_RUN_OR_RETURN(dba_db_export(db, query, msg_writer, &d));

	dba_file_delete(d.file);
	dba_db_delete(db);
	dba_shutdown();

	dba_record_delete(query);

	return dba_error_ok();
}

#endif

// vim:set ts=2 sw=2:
