/* 
 * Copyright (C) 2011 James Coxon, Daniel Richman, Robert Harrison,
 *                    Philip Heron, Adam Greig, Simrun Basuita
 * License: GNU GPL 3
 *
 * location.cxx: Stationary and moving location management
 */

#include "dl_fldigi/location.h"

#include <sstream>

#include "configuration.h"
#include "fl_digi.h"

#include "dl_fldigi/dl_fldigi.h"
#include "dl_fldigi/gps.h"

using namespace std;

namespace dl_fldigi {
namespace location {

enum location_mode new_location_mode, current_location_mode;
double listener_latitude, listener_longitude,
       balloon_latitude, balloon_longitude;
bool listener_valid, balloon_valid;

void start()
{
    if (progdefaults.gps_start_enabled)
        current_location_mode = LOC_GPS;
    else
        current_location_mode = LOC_STATIONARY;

    gps::configure_gps();
}

void update_distance_bearing()
{
    Fl_AutoLock lock;

    if (!hab_ui_exists)
        return;

    if (!listener_valid || !balloon_valid)
    {
        habDistance->value("");
        habBearing->value("");
        return;
    }

    /* Convert everything to radians */
    double c = M_PI/180;

    double lat1, lon1, lat2, lon2;
    lat1 = listener_latitude * c;
    lon1 = listener_longitude * c;
    lat2 = balloon_latitude * c;
    lon2 = balloon_longitude * c;

    double d_lat, d_lon;
    d_lat = lat2 - lat1;
    d_lon = lon2 - lon1;

    /* haversine formula */
    double p = sin(d_lat / 2);
    p *= p;
    double q = sin(d_lon / 2);
    q *= q;
    double a = p + cos(lat1) * cos(lat2) * q;

    double t = atan2(sqrt(a), sqrt(1 - a)) * 2;
    /* 6371 = approx radius of earth in km */
    double distance = t * 6371;

    double y = sin(d_lon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(d_lon);
    double bearing = atan2(y, x);

    /* back to degrees */
    bearing *= (180/M_PI);

    ostringstream str_distance;
    str_distance.precision(4);
    str_distance << distance << "km";

    if (bearing < 0)
        bearing += 360;

    ostringstream str_bearing;
    str_bearing.setf(ios::fixed, ios::floatfield);
    str_bearing.precision(1);
    str_bearing.fill('0');
    str_bearing.width(3 + 1 + 1);
    str_bearing << bearing;

    habDistance->value(str_distance.str().c_str());
    habBearing->value(str_bearing.str().c_str());
}

void update_stationary()
{
    if (current_location_mode != LOC_STATIONARY)
    {
        throw runtime_error("attempted to update stationary location "
                            "while in GPS mode");
    }

    istringstream lat_strm(progdefaults.myLat), lon_strm(progdefaults.myLon);

    if (!progdefaults.myLat.size() || !progdefaults.myLon.size())
    {
        status_important("unable to set listener location: "
                         "latitude or longitude missing");
        goto fail;
    }

    lat_strm >> listener_latitude;
    lon_strm >> listener_longitude;

    if (lat_strm.fail())
    {
        status_important("unable to parse stationary latitude");
        goto fail;
    }

    if (lon_strm.fail())
    {
        status_important("unable to parse stationary longitude");
        goto fail;
    }

    listener_valid = true;
    update_distance_bearing();
    return;

fail:
    listener_valid = false;
    update_distance_bearing();
}

} /* namespace location */
} /* namespace dl_fldigi */
