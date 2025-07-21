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
static const int DAY = 24 * 60 * 60; // one day (sec)
static const int WARRANTY_TTL = 3 * DAY; // 3 days (sec)

int main(int /*argc*/, char** /*argv*/)
{
    std::function<void(const tntdb::Row&)> cb = [] (const tntdb::Row& row) {
        std::string name, keytag, date;
        row["name"].get(name); // asset iname
        row["date"].get(date); // date of warranty
        row["keytag"].get(keytag);

        // REQUIRE keytag = end_warranty_date
        if (keytag != "end_warranty_date") {
            log_error("%s: Unexpected keytag (%s)", AGENT_NAME, keytag.c_str());
            return;
        }

        int day_diff = 0;
        {
            time_t warranty = 0; // end_warranty_date
            {
                struct tm tm_ewd;
                memset(&tm_ewd, 0, sizeof(tm_ewd));
                char* ret = strptime(date.c_str(), "%Y-%m-%d", &tm_ewd);
                if (!ret) {
                    log_error("%s: Cannot convert %s to date", AGENT_NAME, date.c_str());
                    return;
                }
                warranty = mktime(&tm_ewd);
            }

            time_t now = time(NULL);
            {
                struct tm* tm_now_p = gmtime(&now);
                /** if (!tm_now_p) {
                    log_error("%s: Cannot convert current time (error: %s)", AGENT_NAME, strerror(errno));
                    return;
                } */
                tm_now_p->tm_hour = 0;
                tm_now_p->tm_min  = 0;
                tm_now_p->tm_sec  = 0;
                now = mktime(tm_now_p); // truncated day
            }

            // number of days below/after warranty date
            // >=0: the warranty expires in less than X days
            //  <0: the warranty expired X days ago
            day_diff = int(std::ceil((warranty - now) / DAY));
        }

        // write the "end_warranty_date" metric
        int r = fty::shm::write_metric(name, keytag, std::to_string(day_diff), "day", WARRANTY_TTL);
        if (r == 0) {
            log_info("%s: %s@%s = %d days", AGENT_NAME, keytag.c_str(), name.c_str(), day_diff);
        }
        else {
            log_error("%s: write_metric '%s@%s' failed (r: %d)", AGENT_NAME, keytag.c_str(), name.c_str(), r);
        }
    };

    ManageFtyLog::setInstanceFtylog(AGENT_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);

    log_info("%s started", AGENT_NAME);

    // unchecked errors with connection, the tool will fail otherwise
    tntdb::Connection conn = tntdb::connectCached(DBConn::url);
    int r = DBAssets::select_asset_element_all_with_warranty_end(conn, cb);
    if (r != 0) {
        log_error("%s: Error in element selection (r: %d)", AGENT_NAME, r);
        return EXIT_FAILURE;
    }

    log_info("%s ended", AGENT_NAME);
    return EXIT_SUCCESS;
}
