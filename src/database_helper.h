#pragma once
#include <stdio.h>
#include <sqlite3.h>
#include "csv_handler.h"
#include "rapidcsv.h"
using json = nlohmann::json;

class DatabaseHelper {
public:
        explicit DatabaseHelper(const char *database_name) noexcept : db_name(database_name) {
            sqlite3_open(database_name, &db);
            sqlite3_close(db);
        }

        ~DatabaseHelper() = default;

        void createAssignmentsTable();

        void createStudentsTable();

        void addStudents(const char *csv_filepath);

        void addAssignments(std::unordered_map<std::string, StudentData> &studentsMap);

        void updateAssignments(json newAssignments);

        json getAssignments();

        json getStudentsUnderProject(const char *projectID);

private:
    sqlite3 *db;
    const char *db_name;
};