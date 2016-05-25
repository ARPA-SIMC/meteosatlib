#include "utils.h"
#include <gdal_priv.h>
#include <ogr_spatialref.h>

using namespace std;

namespace msat {
namespace tests {

namespace gdal {

/*
 * Error handling
 */
class gdalexception : public std::exception
{
    std::string msg;

public:
    gdalexception() throw ();
    gdalexception(CPLErr eErrClass, int err_no, const char *msg) throw ();
    virtual ~gdalexception() throw () {}

    virtual const char* what() const throw() { return msg.c_str(); }
};

gdalexception::gdalexception() throw () : msg(CPLGetLastErrorMsg())
{
}

gdalexception::gdalexception(CPLErr eErrClass, int err_no, const char *msg) throw ()
{
    stringstream fullmsg;
    fullmsg << "GDAL ";
    switch (eErrClass)
    {
        case CE_None: fullmsg << "(no error)"; break;
        case CE_Debug: fullmsg << "debug"; break;
        case CE_Warning: fullmsg << "warning"; break;
        case CE_Failure: fullmsg << "failure"; break;
        case CE_Fatal: fullmsg << "fatal"; break;
        default: fullmsg << "(unknown)"; break;
    }
    fullmsg << " error " << err_no << ": " << msg;
    this->msg = fullmsg.str();
}


static void throwExceptionsOnGDALErrors(CPLErr eErrClass, int err_no, const char *msg)
{
    throw gdalexception(eErrClass, err_no, msg);
}

static bool initialized = false;

void init()
{
    if (gdal::initialized) return;
    GDALAllRegister();
    CPLSetErrorHandler(gdal::throwExceptionsOnGDALErrors);
    gdal::initialized = true;
}

bool has_driver(const std::string& name)
{
    GDALDriverManager* dm = GetGDALDriverManager();
    return dm->GetDriverByName(name.c_str()) != 0;
}

std::unique_ptr<GDALDataset> open_ro(const std::string& name, const char* const* open_options)
{
    if (open_options)
    {
        return std::unique_ptr<GDALDataset>((GDALDataset*)GDALOpenEx(name.c_str(), GA_ReadOnly, nullptr, open_options, nullptr));
    } else {
        return std::unique_ptr<GDALDataset>((GDALDataset*)GDALOpen(name.c_str(), GA_ReadOnly));
    }
}

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, bool leaveFile,
        const char* driverName, const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
    GDALDriver* driver = (GDALDriver*)GDALGetDriverByName(driverName);
    TempTestFile tf(leaveFile);

    char** options = 0;
    if (!opt1.empty()) options = CSLAddString(options, opt1.c_str());
    if (!opt2.empty()) options = CSLAddString(options, opt2.c_str());
    if (!opt3.empty()) options = CSLAddString(options, opt3.c_str());

    unique_ptr<GDALDataset> dataset((GDALDataset*)GDALCreateCopy(driver, tf.name().c_str(), ds,
                TRUE, options, GDALDummyProgress, NULL));

    CSLDestroy(options);

    return dataset;
}

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, const msat::proj::ImageBox& cropArea, bool leaveFile,
        const char* driverName, const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
    TempTestFile file(leaveFile);

    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driverName);
    wassert(actual(driver != 0).istrue());

    msat::dataset::recode(ds, cropArea, file.name(), driver, opt1, opt2, opt3);

    return open_ro(file.name().c_str());
}

}


TempTestFile::TempTestFile(bool leave) : leave(leave)
{
    char* pn = tempnam(".", "test");
    pathname = pn;
    free(pn);
}


GeoReferencer::GeoReferencer(GDALDataset* ds)
    : ds(ds), proj(0), latlon(0), toLatLon(0), fromLatLon(0)
{
    if (ds->GetGeoTransform(geoTransform) != CE_None)
        throw std::runtime_error("no geotransform found in input dataset");

    dataset::invertGeoTransform(geoTransform, invGeoTransform);

    const char* projname = ds->GetProjectionRef();
    if (!projname || !projname[0])
        throw std::runtime_error("no projection name found in input dataset");

    projection = projname;

    proj = new OGRSpatialReference(projection.c_str());
    latlon = proj->CloneGeogCS();
    toLatLon = OGRCreateCoordinateTransformation(proj, latlon);
    fromLatLon = OGRCreateCoordinateTransformation(latlon, proj);

    delete proj; proj = 0;
    delete latlon; latlon = 0;
}

GeoReferencer::~GeoReferencer()
{
    if (proj) delete proj;
    if (latlon) delete latlon;
    if (toLatLon) delete toLatLon;
    if (fromLatLon) delete fromLatLon;
}

void GeoReferencer::pixelToProjected(int x, int y, double& px, double& py) const
{
    px = geoTransform[0] + geoTransform[1] * x
        + geoTransform[2] * y;
    py = geoTransform[3] + geoTransform[4] * x
        + geoTransform[5] * y;
}

void GeoReferencer::projectedToPixel(double px, double py, int& x, int& y) const
{
    x = lrint(invGeoTransform[0] + px * invGeoTransform[1] + py * invGeoTransform[2]);
    y = lrint(invGeoTransform[3] + px * invGeoTransform[4] + py * invGeoTransform[5]);
}

void GeoReferencer::projectedToLatlon(double px, double py, double& lat, double& lon)
{
    if (!toLatLon->Transform(1, &px, &py))
        throw std::runtime_error("points failed to transform to lat,lon");
    lat = py;
    lon = px;
}

void GeoReferencer::latlonToProjected(double lat, double lon, double& px, double& py)
{
    if (!fromLatLon->Transform(1, &lon, &lat))
        throw std::runtime_error("points failed to transform from lat,lon");
    py = lat;
    px = lon;
}

GDALFixture::~GDALFixture()
{
    delete _dataset;
}

void GDALFixture::test_setup()
{
    gdal::init();

    if (!gdal::has_driver(driver))
        throw TestFailed("driver " + driver + " is not available");
}

GDALDataset* GDALFixture::dataset()
{
    if (!_dataset)
        _dataset = gdal::open_ro(fname).release();
    return _dataset;
}

}
}
