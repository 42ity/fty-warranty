/*  =========================================================================
    warranty_metric - Agent producing warranty expiration metrics

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/// Agent producing warranty expiration metrics

#include <fty_common_db_dbpath.h>
#include <fty_common_db_asset.h>
#include <fty_shm.h>
#include <fty_log.h>

#include <tntdb.h>
#include <time.h>
#include <string>
#include <functional>

static const char* AGENT_NAME = "fty-warranty";

// compute the day diff from now to warranty date
static int compute_date_distance(const std::string& date, int& day_diff)
{
    time_t warranty = 0; // warranty date
    {
        struct tm tm_ewd;
        memset(&tm_ewd, 0, sizeof(tm_ewd));
        char* ret = strptime(date.c_str(), "%Y-%m-%d", &tm_ewd);
        if (!ret) {
            log_error("%s: Cannot convert %s to date", AGENT_NAME, date.c_str());
            return -1;
        }
        warranty = mktime(&tm_ewd); // epoch, sec
    }

    time_t now = time(NULL);
    {
        struct tm* tm_now_p = gmtime(&now);
        if (!tm_now_p) {
            log_error("%s: Cannot convert current time (error: %s)", AGENT_NAME, strerror(errno));
            return -1;
        }
        tm_now_p->tm_hour = tm_now_p->tm_min = tm_now_p->tm_sec = 0;
        now = mktime(tm_now_p); // day truncated, epoch, sec
    }

    // number of days below/after the warranty date
    // >=0: the warranty expires in less than X days
    //  <0: the warranty expired X days ago
    day_diff = int(std::ceil((warranty - now) / (24 * 60 * 60)));
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    std::function<void(const tntdb::Row&)> cb = [] (const tntdb::Row& row) {
        // handle *only* active assets
        std::string status;
        row["status"].get(status);
        if (status != "active") {
            return;
        }

        // REQUIRE keytag = end_warranty_date
        std::string keytag;
        row["keytag"].get(keytag);
        if (keytag != "end_warranty_date") {
            log_error("%s: Unexpected keytag (%s)", AGENT_NAME, keytag.c_str());
            return;
        }

        // compute the day diff from now
        std::string date;
        row["date"].get(date); // warranty date
        int day_diff{0}; // days
        compute_date_distance(date, day_diff);

        std::string name;
        row["name"].get(name); // asset iname

        // write the "end_warranty_date" metric
        const int ttl = 12 * 60 * 60; // half a day (sec)
        int r = fty::shm::write_metric(name, keytag, std::to_string(day_diff), "day", ttl);
        if (r == 0) {
            log_info("%s: %s@%s = %d days (ttl: %d)",
                AGENT_NAME, keytag.c_str(), name.c_str(), day_diff, ttl);
        }
        else {
            log_error("%s: write_metric '%s@%s' failed (r: %d)",
                AGENT_NAME, keytag.c_str(), name.c_str(), r);
        }
    };

    ManageFtyLog::setInstanceFtylog(AGENT_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);

    log_info("%s started", AGENT_NAME);

    // first, remove end_warranty_date metrics from shm
    {
        const int ttl{1}; // metric should be removed on the next reading
        fty::shm::shmMetrics results;
        fty::shm::read_metrics(".*", ".*end_warranty_date", results);
        for (const auto& it : results) {
            fty_proto_t* p = it;
            if (p) {
                log_info("%s: set %s@%s ttl to %d", AGENT_NAME, fty_proto_type(p), fty_proto_name(p), ttl);
                fty_proto_set_ttl(p, ttl);
                fty::shm::write_metric(p);
            }
        }
    }

    // finally, handle assets owning end_warranty_date attribute
    {
        tntdb::Connection conn = tntdb::connectCached(DBConn::url);
        int r = DBAssets::select_asset_element_all_with_warranty_end(conn, cb);
        if (r != 0) {
            log_error("%s: Error in element selection (r: %d)", AGENT_NAME, r);
            return EXIT_FAILURE;
        }
    }

    log_info("%s ended", AGENT_NAME);
    return EXIT_SUCCESS;
}
