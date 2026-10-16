//
// Database - SQLite wrapper with migration support.
//
// Stores game settings and player progress in a local SQLite database
// (vera.db). Replaces the legacy flat-file system (settings.dat +
// Player.state) with a versioned, structured storage that supports
// schema migrations.
//
// Usage:
//   Database::init("vera.db");    // opens DB, runs migrations
//   Database::setSetting("MusicVolume", "75");
//   string vol = Database::getSetting("MusicVolume", "50");
//   Database::close();
//
// Migrations:
//   On startup, the Database checks the schema_version table and applies
//   pending migrations in order. Each migration is a SQL string that runs
//   inside a transaction. See DEVELOPMENT.md for details.
//

#pragma once

#include <string>
#include <map>

// Forward-declare sqlite3 to avoid leaking the SQLite header into every
// file that includes this header. Only Database.cpp needs the full type.
struct sqlite3;

class Database {
public:
  // Open the database file and run any pending migrations.
  // Returns true on success. On failure, the error is logged to stderr
  // and the database remains closed (the game continues with defaults).
  static bool init(const std::string &path);

  // Close the database. Safe to call even if init() failed.
  static void close();

  // True if the database is open and ready.
  static bool isReady();

  // ----- Settings (key-value) -----
  // Get a setting by key. Returns defaultValue if the key doesn't exist.
  static std::string getSetting(const std::string &key,
                                 const std::string &defaultValue = "");
  static int getSettingInt(const std::string &key, int defaultValue = 0);
  static double getSettingDouble(const std::string &key,
                                  double defaultValue = 0.0);

  // Set a setting. Creates or updates the row.
  static void setSetting(const std::string &key, const std::string &value);
  static void setSettingInt(const std::string &key, int value);
  static void setSettingDouble(const std::string &key, double value);

  // ----- Player state -----
  struct PlayerState {
    double cash;
    double fuel;
    double kerosine;
    std::string validVehicles;  // e.g. ">Bogian<>Spirit<"
    std::string checksum;
  };

  static bool loadPlayerState(PlayerState &out);
  static void savePlayerState(const PlayerState &state);

  // ----- Scene progress -----
  struct SceneProgress {
    bool randomDelivery;     // true = random, false = fixed order
    int deliveryEntry;       // current delivery index
    double vehicleX, vehicleY, vehicleZ;  // last vehicle position
    bool hasVehicleLocation; // true if position was saved
  };

  static bool loadSceneProgress(const std::string &sceneName,
                                 SceneProgress &out);
  static void saveSceneProgress(const std::string &sceneName,
                                 const SceneProgress &progress);

  // ----- Migration from legacy files -----
  // Reads settings.dat and Player.state, imports their contents into
  // the database, then renames them to .bak. Called once during init()
  // if the database is freshly created (version 0 -> 1).
  static void migrateFromLegacyFiles();

  // Get the current schema version.
  static int getVersion();

private:
  Database() = default;
  ~Database() = default;

  static sqlite3 *s_db;
  static bool s_ready;

  // Run all pending migrations. Returns true on success.
  static bool runMigrations();

  // Execute a SQL statement (no results). Returns true on success.
  static bool exec(const std::string &sql);

  // Get the schema version from the database (0 if table doesn't exist).
  static int queryVersion();
};
