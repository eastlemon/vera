//
// Database - SQLite wrapper with migration support (implementation).
//

#include "Database.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <fstream>

#include <sqlite3.h>

sqlite3 *Database::s_db = nullptr;
bool Database::s_ready = false;

// ---------------------------------------------------------------------------
// Migration SQL scripts
// ---------------------------------------------------------------------------

// Migration v1: create the initial schema.
// Tables:
//   schema_version  - tracks which migrations have been applied
//   settings        - key-value store for game settings
//   player_state    - single-row table for player progress
//   scene_progress  - per-scene delivery state + vehicle position
static const char *kMigrationV1 = R"(
CREATE TABLE IF NOT EXISTS schema_version (
    version     INTEGER PRIMARY KEY,
    applied_at  TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS settings (
    key    TEXT PRIMARY KEY,
    value  TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS player_state (
    id              INTEGER PRIMARY KEY CHECK (id = 1),
    cash            REAL NOT NULL DEFAULT 50.0,
    fuel            REAL NOT NULL DEFAULT 25.0,
    kerosine        REAL NOT NULL DEFAULT 25.0,
    valid_vehicles  TEXT NOT NULL DEFAULT '>Bogian<',
    checksum        TEXT NOT NULL DEFAULT ''
);

-- Ensure the single player_state row exists.
INSERT OR IGNORE INTO player_state (id) VALUES (1);

CREATE TABLE IF NOT EXISTS scene_progress (
    scene_name          TEXT PRIMARY KEY,
    random_delivery     INTEGER NOT NULL DEFAULT 1,
    delivery_entry      INTEGER NOT NULL DEFAULT 0,
    vehicle_x           REAL NOT NULL DEFAULT 0.0,
    vehicle_y           REAL NOT NULL DEFAULT 0.0,
    vehicle_z           REAL NOT NULL DEFAULT 0.0,
    has_vehicle_location INTEGER NOT NULL DEFAULT 0
);
)";

// Migration v2 (placeholder for future schema changes):
// Example: ALTER TABLE player_state ADD COLUMN total_deliveries INTEGER DEFAULT 0;

// List of all migrations, in order. Index 0 = version 1, etc.
struct Migration {
  int version;
  const char *sql;
};

static const Migration kMigrations[] = {
  {1, kMigrationV1},
  {0, nullptr}  // sentinel
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool Database::init(const std::string &path) {
  if (s_ready) return true;

  int rc = sqlite3_open(path.c_str(), &s_db);
  if (rc != SQLITE_OK) {
    std::fprintf(stderr, "Database: cannot open %s: %s\n", path.c_str(),
                 sqlite3_errmsg(s_db));
    sqlite3_close(s_db);
    s_db = nullptr;
    return false;
  }

  // Enable foreign keys + WAL mode for better concurrency.
  exec("PRAGMA foreign_keys = ON;");
  exec("PRAGMA journal_mode = WAL;");

  // Run migrations.
  if (!runMigrations()) {
    std::fprintf(stderr, "Database: migration failed\n");
    sqlite3_close(s_db);
    s_db = nullptr;
    return false;
  }

  // If this is a fresh database (was version 0), import legacy files.
  // We check this AFTER migrations run, so version is now >= 1.
  // The migrateFromLegacyFiles() call checks for .dat/.state files
  // and imports them if they exist.
  static bool s_legacyMigrated = false;
  if (!s_legacyMigrated) {
    migrateFromLegacyFiles();
    s_legacyMigrated = true;
  }

  s_ready = true;
  return true;
}

void Database::close() {
  if (s_db) {
    sqlite3_close(s_db);
    s_db = nullptr;
  }
  s_ready = false;
}

bool Database::isReady() { return s_ready; }

// ---------------------------------------------------------------------------
// Migrations
// ---------------------------------------------------------------------------

bool Database::runMigrations() {
  int currentVersion = queryVersion();

  for (int i = 0; kMigrations[i].sql != nullptr; ++i) {
    int targetVersion = kMigrations[i].version;
    if (targetVersion <= currentVersion) continue;

    // Begin transaction.
    if (!exec("BEGIN TRANSACTION;")) return false;

    // Execute migration SQL.
    if (!exec(kMigrations[i].sql)) {
      std::fprintf(stderr, "Database: migration v%d failed: %s\n",
                   targetVersion, sqlite3_errmsg(s_db));
      exec("ROLLBACK;");
      return false;
    }

    // Record the version.
    std::stringstream sql;
    sql << "INSERT INTO schema_version (version) VALUES ("
        << targetVersion << ");";
    if (!exec(sql.str())) {
      exec("ROLLBACK;");
      return false;
    }

    // Commit.
    if (!exec("COMMIT;")) return false;

    std::printf("Database: applied migration v%d\n", targetVersion);
    currentVersion = targetVersion;
  }

  return true;
}

int Database::queryVersion() {
  // Check if schema_version table exists.
  sqlite3_stmt *stmt = nullptr;
  int version = 0;

  int rc = sqlite3_prepare_v2(s_db,
      "SELECT name FROM sqlite_master WHERE type='table' AND name='schema_version';",
      -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      // Table exists - query the max version.
      sqlite3_finalize(stmt);
      rc = sqlite3_prepare_v2(s_db,
          "SELECT MAX(version) FROM schema_version;", -1, &stmt, nullptr);
      if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        version = sqlite3_column_int(stmt, 0);
      }
    }
  }
  if (stmt) sqlite3_finalize(stmt);
  return version;
}

int Database::getVersion() { return queryVersion(); }

// ---------------------------------------------------------------------------
// SQL execution helper
// ---------------------------------------------------------------------------

bool Database::exec(const std::string &sql) {
  if (!s_db) return false;
  char *err = nullptr;
  int rc = sqlite3_exec(s_db, sql.c_str(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    std::fprintf(stderr, "Database: SQL error: %s\n", err ? err : "(null)");
    if (err) sqlite3_free(err);
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// Settings (key-value)
// ---------------------------------------------------------------------------

std::string Database::getSetting(const std::string &key,
                                  const std::string &defaultValue) {
  if (!s_ready) return defaultValue;
  sqlite3_stmt *stmt = nullptr;
  std::string result = defaultValue;

  int rc = sqlite3_prepare_v2(s_db,
      "SELECT value FROM settings WHERE key = ?;", -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char *val = sqlite3_column_text(stmt, 0);
      if (val) result = reinterpret_cast<const char *>(val);
    }
  }
  if (stmt) sqlite3_finalize(stmt);
  return result;
}

int Database::getSettingInt(const std::string &key, int defaultValue) {
  std::string val = getSetting(key, "");
  if (val.empty()) return defaultValue;
  return std::atoi(val.c_str());
}

double Database::getSettingDouble(const std::string &key, double defaultValue) {
  std::string val = getSetting(key, "");
  if (val.empty()) return defaultValue;
  return std::atof(val.c_str());
}

void Database::setSetting(const std::string &key, const std::string &value) {
  if (!s_ready) return;
  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(s_db,
      "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);",
      -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
  }
  if (stmt) sqlite3_finalize(stmt);
}

void Database::setSettingInt(const std::string &key, int value) {
  setSetting(key, std::to_string(value));
}

void Database::setSettingDouble(const std::string &key, double value) {
  setSetting(key, std::to_string(value));
}

// ---------------------------------------------------------------------------
// Player state
// ---------------------------------------------------------------------------

bool Database::loadPlayerState(PlayerState &out) {
  if (!s_ready) return false;
  sqlite3_stmt *stmt = nullptr;
  bool found = false;

  int rc = sqlite3_prepare_v2(s_db,
      "SELECT cash, fuel, kerosine, valid_vehicles, checksum "
      "FROM player_state WHERE id = 1;", -1, &stmt, nullptr);
  if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
    out.cash = sqlite3_column_double(stmt, 0);
    out.fuel = sqlite3_column_double(stmt, 1);
    out.kerosine = sqlite3_column_double(stmt, 2);
    const unsigned char *v = sqlite3_column_text(stmt, 3);
    out.validVehicles = v ? reinterpret_cast<const char *>(v) : ">Bogian<";
    const unsigned char *c = sqlite3_column_text(stmt, 4);
    out.checksum = c ? reinterpret_cast<const char *>(c) : "";
    found = true;
  }
  if (stmt) sqlite3_finalize(stmt);
  return found;
}

void Database::savePlayerState(const PlayerState &state) {
  if (!s_ready) return;
  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(s_db,
      "UPDATE player_state SET cash = ?, fuel = ?, kerosine = ?, "
      "valid_vehicles = ?, checksum = ? WHERE id = 1;",
      -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, state.cash);
    sqlite3_bind_double(stmt, 2, state.fuel);
    sqlite3_bind_double(stmt, 3, state.kerosine);
    sqlite3_bind_text(stmt, 4, state.validVehicles.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, state.checksum.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
  }
  if (stmt) sqlite3_finalize(stmt);
}

// ---------------------------------------------------------------------------
// Scene progress
// ---------------------------------------------------------------------------

bool Database::loadSceneProgress(const std::string &sceneName,
                                  SceneProgress &out) {
  if (!s_ready) return false;
  sqlite3_stmt *stmt = nullptr;
  bool found = false;

  int rc = sqlite3_prepare_v2(s_db,
      "SELECT random_delivery, delivery_entry, vehicle_x, vehicle_y, "
      "vehicle_z, has_vehicle_location FROM scene_progress "
      "WHERE scene_name = ?;", -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, sceneName.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      out.randomDelivery = sqlite3_column_int(stmt, 0) != 0;
      out.deliveryEntry = sqlite3_column_int(stmt, 1);
      out.vehicleX = sqlite3_column_double(stmt, 2);
      out.vehicleY = sqlite3_column_double(stmt, 3);
      out.vehicleZ = sqlite3_column_double(stmt, 4);
      out.hasVehicleLocation = sqlite3_column_int(stmt, 5) != 0;
      found = true;
    }
  }
  if (stmt) sqlite3_finalize(stmt);
  return found;
}

void Database::saveSceneProgress(const std::string &sceneName,
                                  const SceneProgress &progress) {
  if (!s_ready) return;
  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(s_db,
      "INSERT OR REPLACE INTO scene_progress "
      "(scene_name, random_delivery, delivery_entry, vehicle_x, vehicle_y, "
      "vehicle_z, has_vehicle_location) VALUES (?, ?, ?, ?, ?, ?, ?);",
      -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, sceneName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, progress.randomDelivery ? 1 : 0);
    sqlite3_bind_int(stmt, 3, progress.deliveryEntry);
    sqlite3_bind_double(stmt, 4, progress.vehicleX);
    sqlite3_bind_double(stmt, 5, progress.vehicleY);
    sqlite3_bind_double(stmt, 6, progress.vehicleZ);
    sqlite3_bind_int(stmt, 7, progress.hasVehicleLocation ? 1 : 0);
    sqlite3_step(stmt);
  }
  if (stmt) sqlite3_finalize(stmt);
}

// ---------------------------------------------------------------------------
// Legacy file migration
// ---------------------------------------------------------------------------

void Database::migrateFromLegacyFiles() {
  // Check if settings.dat exists.
  std::ifstream settingsFile("settings.dat");
  if (settingsFile.good()) {
    std::printf("Database: migrating settings.dat...\n");
    char line[1024];
    while (settingsFile.getline(line, sizeof(line))) {
      std::string s(line);
      // Parse "key = value" format.
      size_t eq = s.find('=');
      if (eq == std::string::npos) continue;
      std::string key = s.substr(0, eq);
      std::string value = s.substr(eq + 1);
      // Trim whitespace.
      while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
        key.pop_back();
      while (!key.empty() && (key.front() == ' ' || key.front() == '\t'))
        key.erase(0, 1);
      while (!value.empty() && (value.back() == ' ' || value.back() == '\t' ||
                                value.back() == '\r' || value.back() == '\n'))
        value.pop_back();
      while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
        value.erase(0, 1);
      if (!key.empty()) {
        setSetting(key, value);
      }
    }
    settingsFile.close();
    // Rename to .bak.
    std::rename("settings.dat", "settings.dat.bak");
    std::printf("Database: settings.dat migrated, renamed to .bak\n");
  }

  // Check if Player.state exists.
  std::ifstream stateFile("Player.state");
  if (stateFile.good()) {
    std::printf("Database: migrating Player.state...\n");
    // The legacy format is an INI file with [State] section.
    // Keys: Cash, Fuel, Kerosine, Vehicles, Checksum, and per-scene entries.
    PlayerState ps;
    loadPlayerState(ps);  // Load current defaults

    char line[1024];
    while (stateFile.getline(line, sizeof(line))) {
      std::string s(line);
      // Skip section headers and empty lines.
      if (s.empty() || s[0] == '[' || s[0] == '#' || s[0] == ';') continue;
      size_t eq = s.find('=');
      if (eq == std::string::npos) continue;
      std::string key = s.substr(0, eq);
      std::string value = s.substr(eq + 1);
      // Trim.
      while (!key.empty() && key.back() == ' ') key.pop_back();
      while (!key.empty() && key.front() == ' ') key.erase(0, 1);
      while (!value.empty() && (value.back() == ' ' || value.back() == '\r' ||
                                value.back() == '\n'))
        value.pop_back();
      while (!value.empty() && value.front() == ' ') value.erase(0, 1);

      if (key == "Cash") ps.cash = std::atof(value.c_str());
      else if (key == "Fuel") ps.fuel = std::atof(value.c_str());
      else if (key == "Kerosine") ps.kerosine = std::atof(value.c_str());
      else if (key == "Vehicles") ps.validVehicles = value;
      else if (key == "Checksum") ps.checksum = value;
      else {
        // Per-scene entry. Format: "R 003: x y z" or "D 003: x y z"
        // The scene name is the key.
        SceneProgress sp;
        sp.randomDelivery = (value.size() > 0 && value[0] == 'R');
        if (value.size() >= 2) {
          sp.deliveryEntry = std::atoi(value.c_str() + 2);
        }
        if (value.size() >= 7) {
          sp.vehicleX = 0; sp.vehicleY = 0; sp.vehicleZ = 0;
          std::sscanf(value.c_str() + 7, "%lf %lf %lf",
                      &sp.vehicleX, &sp.vehicleY, &sp.vehicleZ);
          sp.hasVehicleLocation = true;
        }
        saveSceneProgress(key, sp);
      }
    }
    savePlayerState(ps);
    stateFile.close();
    std::rename("Player.state", "Player.state.bak");
    std::printf("Database: Player.state migrated, renamed to .bak\n");
  }
}
