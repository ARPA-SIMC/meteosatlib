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

#include <msat/ImportGRIB.h>
#include <msat/ImportSAFH5.h>
#include <msat/ImportNetCDF.h>
#include <msat/ImportNetCDF24.h>
#include <msat/ImportXRIT.h>

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

#define DEFAULT_SATINFO "/usr/share/dballe/satinfo.csv"

using namespace std;
using namespace msat;

static const char* op_dsn = "test";
static const char* op_user = "";
static const char* op_pass = "";
static const char* op_area = "";
static const char* op_step = "";
static const char* op_satinfo = DEFAULT_SATINFO;
static const char* op_ident = "";
static int op_overwrite = 0;
static int op_dump = 0;
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
#ifdef HAVE_NETCDF
	if (isNetCDF(filename))
		return createNetCDFImporter(filename);
	if (isNetCDF24(filename))
		return createNetCDF24Importer(filename);
#endif
#ifdef HAVE_HDF5
	if (isSAFH5(filename))
		return createSAFH5Importer(filename);
#endif
#ifdef HAVE_HRIT
	if (isXRIT(filename))
		return createXRITImporter(filename);
#endif
	return std::auto_ptr<ImageImporter>();
}

struct ChannelInfo
{
	dba_varcode var;
	int ltype, l1, l2;
	int pind, p1, p2;
	string rep;
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
			if (count < 9)
				continue;
			int channel = strtoul(cols[0], NULL, 0);
			(*this)[channel].var = DBA_STRING_TO_VAR(cols[1]+1);
			(*this)[channel].ltype = strtoul(cols[2], NULL, 0);
			(*this)[channel].l1 = strtoul(cols[3], NULL, 0);
			(*this)[channel].l2 = strtoul(cols[4], NULL, 0);
			(*this)[channel].pind = strtoul(cols[5], NULL, 0);
			(*this)[channel].p1 = strtoul(cols[6], NULL, 0);
			(*this)[channel].p2 = strtoul(cols[7], NULL, 0);
			// Truncate trailing newline of report memo
			string rep = cols[8];
			if (rep[rep.size() - 1] == '\n')
				rep.resize(rep.size() - 1);
			(*this)[channel].rep = rep;

			for (int i = 0; i < count; ++i)
				free(cols[i]);
		}

		fclose(in);
		return dba_error_ok();
	}
};

struct Processor
{
	std::vector<std::string> fileNames;
	ChannelTab satinfo;
	set<int> warned;
	dba_db db;
	size_t processed;

	Processor() : db(0), processed(0) {}
	virtual ~Processor()
	{
		if (db)
			dba_db_delete(db);
		dba_shutdown();
	}

	virtual dba_err parseCommandline(poptContext optCon)
	{
		DBA_RUN_OR_RETURN(satinfo.read(op_satinfo));

		// Get the file names of the images to read
		while (1)
		{
			const char* arg = poptPeekArg(optCon);
			if (arg == NULL) break;
			// Stop processing files when a query parameter is found
			if (strchr(arg, '=') != NULL) break;
			fileNames.push_back(arg);
			// Consume the argument if it was processed
			poptGetArg(optCon);
		}

		if (fileNames.empty())
			dba_cmdline_error(optCon, "No files given in the commandline");

		return dba_error_ok();
	}

	const ChannelInfo* getChannelInfo(int channel)
	{
		ChannelTab::const_iterator citer = satinfo.find(channel);
		if (citer == satinfo.end())
		{
			if (warned.find(channel) == warned.end())
			{
				fprintf(stderr, "No channel information for channel %d in %s: skipping image.\n",
						channel, op_satinfo);
				warned.insert(channel);
			}
			return 0;
		}
		return &(citer->second);
	}

	virtual std::auto_ptr<ImageImporter> getImporter(const std::string& name)
	{
		return ::getImporter(name);
	}

	virtual dba_err handleImage(Image& img) = 0;

	virtual dba_err handleFile(const std::string& name)
	{
		// Read the image
		ImageVector imgs;
		try
		{
			std::auto_ptr<ImageImporter> importer = getImporter(name);
			if (!importer.get())
			{
				cerr << "No importer found for " << name << ": ignoring." << endl;
				return dba_error_ok();
			}
			importer->read(imgs);
		}
		catch (std::exception& e)
		{
			cerr << "Importing " << name << " failed: ignoring." << endl;
			return dba_error_ok();
		}

		// Import the data
		for (ImageVector::const_iterator iterimg = imgs.begin(); iterimg != imgs.end(); ++iterimg)
		{
			DBA_RUN_OR_RETURN(handleImage(**iterimg));
		}
		return dba_error_ok();
	}

	virtual dba_err init(poptContext optCon)
	{
		DBA_RUN_OR_RETURN(parseCommandline(optCon));
		DBA_RUN_OR_RETURN(create_dba_db(&db));
		return dba_error_ok();
	}

	virtual dba_err process(poptContext optCon)
	{
		// Read the images
		for (vector<string>::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i)
			DBA_RUN_OR_RETURN(handleFile(*i));

		if (processed == 0)
			dba_cmdline_error(optCon, "No valid input files found");

		return dba_error_ok();
	}
};

struct Station
{
	double lat;
	double lon;
	int id;
	Station() {}
	Station(double lat, double lon, int ident) : lat(lat), lon(lon), id(id) {}
};

struct InterpolateProcessor : public Processor
{
	dba_record query, data;
	int ax, ay, aw, ah;
	double latmin, latmax, lonmin, lonmax;
	std::vector<Station> stations;
	bool dump;

	InterpolateProcessor() :
		query(0), data(0),
		ax(0), ay(0), aw(0), ah(0),
		latmin(1000), latmax(1000), lonmin(1000), lonmax(1000), dump(false) {}

	virtual ~InterpolateProcessor()
	{
		if (query)
			dba_record_delete(query);
		if (data)
			dba_record_delete(data);
	}

	virtual dba_err parseCommandline(poptContext optCon)
	{
		DBA_RUN_OR_RETURN(Processor::parseCommandline(optCon));

		// Default cropping parameters (all zeroes mean "no cropping")
		if (op_area[0] != 0)
			if (sscanf(op_area, "%d,%d,%d,%d", &ax,&ay,&aw,&ah) != 4)
				dba_cmdline_error(optCon, "area parameter should be in the format x,y,width,height");

		DBA_RUN_OR_RETURN(dba_cmdline_get_query(optCon, query));

		// Read the coordinates
		if (dba_record_key_peek(query, DBA_KEY_LATMIN) != NULL)
			DBA_RUN_OR_RETURN(dba_record_key_enqd(query, DBA_KEY_LATMIN, &latmin));
		if (dba_record_key_peek(query, DBA_KEY_LATMAX) != NULL)
			DBA_RUN_OR_RETURN(dba_record_key_enqd(query, DBA_KEY_LATMAX, &latmax));
		if (dba_record_key_peek(query, DBA_KEY_LONMIN) != NULL)
			DBA_RUN_OR_RETURN(dba_record_key_enqd(query, DBA_KEY_LONMIN, &lonmin));
		if (dba_record_key_peek(query, DBA_KEY_LONMAX) != NULL)
			DBA_RUN_OR_RETURN(dba_record_key_enqd(query, DBA_KEY_LONMAX, &lonmax));

		return dba_error_ok();
	}

	virtual std::auto_ptr<ImageImporter> getImporter(const std::string& name)
	{
		std::auto_ptr<ImageImporter> importer = Processor::getImporter(name);
		if (importer.get())
		{
			importer->cropX = ax;
			importer->cropY = ay;
			importer->cropWidth = aw;
			importer->cropHeight = ah;
			importer->cropLatMin = latmin;
			importer->cropLatMax = latmax;
			importer->cropLonMin = lonmin;
			importer->cropLonMax = lonmax;
		}
		return importer;
	}

	virtual dba_err init(poptContext optCon)
	{
		dba_record result;
		dba_db_cursor cursor;

		DBA_RUN_OR_RETURN(dba_record_create(&query));
		DBA_RUN_OR_RETURN(dba_record_create(&data));

		DBA_RUN_OR_RETURN(Processor::init(optCon));

		// Connect the database and start the query
		int count;
		DBA_RUN_OR_RETURN(dba_db_ana_query(db, query, &cursor, &count));
		DBA_RUN_OR_RETURN(dba_record_create(&result));

		// Retrieve the station list from the database
		while (true)
		{
			int has_data;
			DBA_RUN_OR_RETURN(dba_db_cursor_next(cursor, &has_data));
			if (!has_data)
				break;
			dba_record_clear(result);
			DBA_RUN_OR_RETURN(dba_db_cursor_to_record(cursor, result));

			Station s;
			DBA_RUN_OR_RETURN(dba_record_key_enqd(result, DBA_KEY_LAT, &(s.lat)));
			DBA_RUN_OR_RETURN(dba_record_key_enqd(result, DBA_KEY_LON, &(s.lon)));
			DBA_RUN_OR_RETURN(dba_record_key_enqi(result, DBA_KEY_ANA_ID, &(s.id)));
			stations.push_back(s);
		}

		dba_record_delete(result);
		return dba_error_ok();
	}

	virtual dba_err handleImage(Image& img)
	{
		const ChannelInfo* c = getChannelInfo(img.channel_id);
		if (!c) return dba_error_ok();

		// Prepare the common parts of the input record
		dba_record_clear(data);
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_YEAR, img.year));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_MONTH, img.month));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_DAY, img.day));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_HOUR, img.hour));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_MIN, img.minute));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_SEC, 0));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_LEVELTYPE, c->ltype));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_L1, c->l1));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_L2, c->l2));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_PINDICATOR, c->pind));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_P1, c->p1));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_P2, c->p2));
		DBA_RUN_OR_RETURN(dba_record_key_setc(data, DBA_KEY_REP_MEMO, c->rep.c_str()));

		for (vector<Station>::const_iterator i = stations.begin();
				i != stations.end(); ++i)
		{
			size_t x, y;
			img.coordsToPixels(i->lat, i->lon, x, y);
			//fprintf(stderr, "  (%f,%f) -> (%d,%d)\n", lat, lon, x, y);
			if (x >= img.data->columns || y >= img.data->lines) continue;

			double val = img.data->scaled(x, y);
			if (val == img.data->missingValue) continue;

			DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_ANA_ID, i->id));
			DBA_RUN_OR_RETURN(dba_record_var_setd(data, c->var, val));
			if (dump)
			{
				fprintf(stdout, "%f,%f: %04d-%02d-%02d %02d:%02d %d,%d,%d %d,%d,%d B%02d%03d %f\n", i->lat, i->lon,
						img.year, img.month, img.day, img.hour, img.minute,
						c->ltype, c->l1, c->l2, c->pind, c->p1, c->p2, DBA_VAR_X(c->var), DBA_VAR_Y(c->var), val);
			} else
				DBA_RUN_OR_RETURN(dba_db_insert(db, data, op_overwrite ? 1 : 0, 0, NULL, NULL));
		}
		++processed;
		return dba_error_ok();
	}
};

struct GridImporter : public Processor
{
	dba_record data;
	double latmin, latmax, lonmin, lonmax;
	double latstep, lonstep;
	bool dump;

	GridImporter() : data(0), dump(false) {}

	~GridImporter()
	{
		if (data)
			dba_record_delete(data);
	}

	virtual dba_err parseCommandline(poptContext optCon)
	{
		// Read the coordinates and steps
		if (op_area[0] == 0)
			dba_cmdline_error(optCon, "you need to specify --area=latmin,latmax,lonmin,lonmax");
		if (sscanf(op_area, "%lf,%lf,%lf,%lf", &latmin,&latmax,&lonmin,&lonmax) != 4)
			dba_cmdline_error(optCon, "area parameter should be in the format latmin,latmax,lonmin,lonmax");
		if (op_step[0] == 0)
			dba_cmdline_error(optCon, "you need to specify --step=latstep,lonstep");
		if (sscanf(op_step, "%lf,%lf", &latstep,&lonstep) != 2)
			dba_cmdline_error(optCon, "step parameter should be in the format latstep,lonstep");
		if (op_ident[0] == 0)
			dba_cmdline_error(optCon, "you need to specify --ident=pseudoana-id");

		return Processor::parseCommandline(optCon);
	}

	virtual std::auto_ptr<ImageImporter> getImporter(const std::string& name)
	{
		std::auto_ptr<ImageImporter> importer = Processor::getImporter(name);
		if (importer.get())
		{
			importer->cropLatMin = latmin;
			importer->cropLatMax = latmax;
			importer->cropLonMin = lonmin;
			importer->cropLonMax = lonmax;
		}
		return importer;
	}

	virtual dba_err handleImage(Image& img)
	{
		// Get the mapping data for the channel
		const ChannelInfo* c = getChannelInfo(img.channel_id);
		if (!c) return dba_error_ok();

		// Prepare the new input record
		dba_record_clear(data);
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_YEAR, img.year));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_MONTH, img.month));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_DAY, img.day));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_HOUR, img.hour));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_MIN, img.minute));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_SEC, 0));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_LEVELTYPE, c->ltype));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_L1, c->l1));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_L2, c->l2));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_PINDICATOR, c->pind));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_P1, c->p1));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_P2, c->p2));
		DBA_RUN_OR_RETURN(dba_record_key_setc(data, DBA_KEY_REP_MEMO, c->rep.c_str()));
		DBA_RUN_OR_RETURN(dba_record_key_seti(data, DBA_KEY_MOBILE, 1));
		DBA_RUN_OR_RETURN(dba_record_key_setc(data, DBA_KEY_IDENT, op_ident));

		// Sample the points at the given intervals and insert them
		for (double lat = latmin; lat <= latmax; lat += latstep)
			for (double lon = lonmin; lon <= lonmax; lon += lonstep)
			{
				size_t x, y;
				img.coordsToPixels(lat, lon, x, y);
				//fprintf(stderr, "  (%f,%f) -> (%d,%d)\n", lat, lon, x, y);

				if (x >= img.data->columns || y >= img.data->lines) continue;

				double val = img.data->scaled(x, y);
				if (val == img.data->missingValue) continue;

				DBA_RUN_OR_RETURN(dba_record_key_setd(data, DBA_KEY_LAT, lat));
				DBA_RUN_OR_RETURN(dba_record_key_setd(data, DBA_KEY_LON, lon));
				DBA_RUN_OR_RETURN(dba_record_var_setd(data, c->var, val));
				if (dump)
					fprintf(stdout, "%f,%f: %04d-%02d-%02d %02d:%02d %d,%d,%d %d,%d,%d B%02d%03d %f\n", lat, lon,
							img.year, img.month, img.day, img.hour, img.minute,
							c->ltype, c->l1, c->l2, c->pind, c->p1, c->p2, DBA_VAR_X(c->var), DBA_VAR_Y(c->var), val);
				else
					DBA_RUN_OR_RETURN(dba_db_insert(db, data, op_overwrite ? 1 : 0, 1, NULL, NULL));
			}
		++processed;
		return dba_error_ok();
	}

	virtual dba_err init(poptContext optCon)
	{
		DBA_RUN_OR_RETURN(Processor::init(optCon));
		DBA_RUN_OR_RETURN(dba_record_create(&data));
		return dba_error_ok();
	}
};

dba_err do_import(poptContext optCon)
{
	const char* action;
	/* Throw away the command name */
	action = poptGetArg(optCon);

	//Importer consumer(db, op_overwrite != 0);
	//DBA_RUN_OR_RETURN(interpolate(optCon, db, consumer));

	DBA_RUN_OR_RETURN(dba_init());
	InterpolateProcessor processor;
	processor.dump = op_dump != 0;
	DBA_RUN_OR_RETURN(processor.init(optCon));
	DBA_RUN_OR_RETURN(processor.process(optCon));
	dba_shutdown();

	return dba_error_ok();
}

dba_err do_importgrid(poptContext optCon)
{
	const char* action;
	/* Throw away the command name */
	action = poptGetArg(optCon);

	DBA_RUN_OR_RETURN(dba_init());
	GridImporter processor;
	processor.dump = op_dump != 0;
	DBA_RUN_OR_RETURN(processor.init(optCon));
	DBA_RUN_OR_RETURN(processor.process(optCon));
	dba_shutdown();

	return dba_error_ok();
}



static struct tool_desc dbasat;

struct poptOption dbasat_import_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "area", 'a', POPT_ARG_STRING, &op_area, 0, "read only the given subarea (in pixels) of the images (format: x,y,w,h)" },
	{ "satinfo", 0, POPT_ARG_STRING, &op_satinfo, 0, "pathname of the .csv file mapping satellite channels to DB-ALLe variable information (default: " DEFAULT_SATINFO ")" },
	{ "dump", 0, POPT_ARG_NONE, &op_dump, 0, "do not insert data in the database, but dump the results on standard output" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ "overwrite", 'f', POPT_ARG_NONE, &op_overwrite, 0, "overwrite existing data" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0, "Options used to connect to the database" },
	POPT_TABLEEND
};

struct poptOption dbasat_importgrid_options[] = {
	{ "help", '?', 0, 0, 1, "print an help message" },
	{ "area", 'a', POPT_ARG_STRING, &op_area, 0, "import the given subarea (in coordinates) of the images (format: latmin,latmax,lonmin,lonmax)" },
	{ "step", 's', POPT_ARG_STRING, &op_step, 0, "distance (in degrees) between the sampled points (format: latstep,lonstep)" },
	{ "satinfo", 0, POPT_ARG_STRING, &op_satinfo, 0, "pathname of the .csv file mapping satellite channels to DB-ALLe variable information (default: " DEFAULT_SATINFO ")" },
	{ "ident", 'i', POPT_ARG_STRING, &op_ident, 0, "pseudoana identifier to use on import" },
	{ "dump", 0, POPT_ARG_NONE, &op_dump, 0, "do not insert data in the database, but dump the results on standard output" },
	{ "verbose", 0, POPT_ARG_NONE, &op_verbose, 0, "verbose output" },
	{ "overwrite", 'f', POPT_ARG_NONE, &op_overwrite, 0, "overwrite existing data" },
	{ NULL, 0, POPT_ARG_INCLUDE_TABLE, &dbTable, 0, "Options used to connect to the database" },
	POPT_TABLEEND
};


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
	dbasat.ops = (struct op_dispatch_table*)calloc(3, sizeof(struct op_dispatch_table));

	dbasat.ops[0].func = do_import;
	dbasat.ops[0].aliases[0] = "import";
	dbasat.ops[0].usage = "import [options] [file1 [file2...]] [queryparm1=val1 [queryparm2=val2 [...]]]";
	dbasat.ops[0].desc = "Import satellite data corresponding to the stations found in the database matching the given query";
	dbasat.ops[0].longdesc = "Query parameters are the same of the Fortran API. "
		"Please see the section \"Input and output parameters -- For data "
		"related action routines\" of the Fortran API documentation for a "
		"complete list.";
	dbasat.ops[0].optable = dbasat_import_options;

	dbasat.ops[1].func = do_importgrid;
	dbasat.ops[1].aliases[0] = "importgrid";
	dbasat.ops[1].usage = "importgrid --area=latmin,latmax,lonmin,lonmax --step=latstep,lonstep --ident=ident [options] [file1 [file2...]]";
	dbasat.ops[1].desc = "Import satellite data taken at regular intervals";
	dbasat.ops[1].longdesc = NULL;
	dbasat.ops[1].optable = dbasat_importgrid_options;

	dbasat.ops[2].func = NULL;
	dbasat.ops[2].usage = NULL;
	dbasat.ops[2].desc = NULL;
	dbasat.ops[2].longdesc = NULL;
	dbasat.ops[2].optable = NULL;
};

int main (int argc, const char* argv[])
{
	init();
	return dba_cmdline_dispatch_main(&dbasat, argc, argv);
}

// vim:set ts=2 sw=2:
